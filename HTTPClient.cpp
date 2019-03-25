/*
 * Title:        HTTPClient.cpp
 * Description:  HTTPClient Transport
 * $Copyright (c) 2016, 2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * $Revision: 321386 $ $Date: 2017-12-14 10:26:04 +0000 (Thu, 14 Dec 2017) $
 */

#include <sag_connectivity_plugins.hpp>
/*
 * We use the threading primitives from sag_connectivity_threading.h
 * in order to support older compilers. If your compiler supports
 * C++11 std::thread and std::mutex you should use those instead.
 */
#include <sag_connectivity_threading.h>
#include <vector>

/* Handle multiple platforms. In particular:
 *
 * - different includes between windows and unix
 * - different type for socket and close (#defines to make them consistent)
 *
 */
#ifdef _WIN32
#pragma comment(lib,"Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#define closesocket close
#define SOCKET int
#endif
#include <errno.h>
#include <string.h>

using namespace com::softwareag::connectivity;

namespace com {
namespace softwareag {
namespace samples {

/**
 *  HTTPClient which can send GET requests to websites.
 *
 *  The chain is configured with the host to connect to and (optionally)
 *  the port to connect on.
 *
 *  Requests are handled by a separate thread which will 
 *  send back responses asynchronously.
 *
 *  Requests can include an ID which will be used to associate
 *  requests with responses.
 *
 *  Requests are of the form:
 *
 *  metadata.requestId - the ID to match with the response
 *  metadata.http.path - the path to retrieve from the configured host.
 *  metadata.http.method - must be the string GET
 *  metadata.http.headers.* - any other headers to be set in the request
 *
 *  Responses are either errors of the form:
 *
 *  metadata.http.statusCode - a value other than 200
 *  metadata.requestId - The ID of the request
 *  metadata.http.statusReason - the error message
 *  payload - data which was returned
 *
 *  Or they are responses of the form:
 *
 *  metadata.http.statusCode - 200
 *  metadata.http.requestId - The ID of the request
 *  metadata.http.headers.* - any other headers which were returned
 *  payload - the data returned
 *
 * ** NOTE: this is only a sample and not suitable for use in production.
 * ** We make no guarantee of security or compliance with HTTP standards.
 * ** For production use, use the HTTPClient provided in the connectivity-http-client library
 */
class HTTPClient: public AbstractSimpleTransport
{

public:
	/** Create the transport and check we have the correct (no) config */
	HTTPClient(const TransportConstructorParameters &params)
		: AbstractSimpleTransport(params),
		host(),
		port(),
		running(false), 
		// status monitoring and KPIs. See naming conventions described in documentation.  
		transportStatus(getStatusReporter().createStatusItem(chainId + "." + pluginName + ".status", StatusReporter::STATUS_STARTING())),
		errorsTowardsHost(getStatusReporter().createStatusItem(chainId + "." + pluginName + ".errorsTowardsHost", 0)),
		responsesTowardsHost(getStatusReporter().createStatusItem(chainId + "." + pluginName + ".responsesTowardsHost", 0)),
		kpis(getStatusReporter().createStatusItem(chainId + "." + pluginName + ".KPIs",
			errorsTowardsHost->key() + "," + responsesTowardsHost->key()
			))
	  {
		// get the host and port values
		MapExtractor configEx(config, "config");
		host = configEx.getStringDisallowEmpty("host");
		port = configEx.getStringDisallowEmpty("port", "80");
		configEx.checkNoItemsRemaining();

		// initialize the concurrency primitives
		SAG_MUTEX_INIT(queueLock);
		SAG_SEMA_INIT(queueSemaphore);
	}

	~HTTPClient() {
		SAG_SEMA_DESTROY(queueSemaphore);
		SAG_MUTEX_DESTROY(queueLock);
	}
	
	/** Start the request-handling thread */
	virtual void start()
	{
		SAG_LOCK_GUARD queueLockGuard(queueLock);
		running = true;
		
		// create the request-handling thread
		SAG_THREAD_CREATE(thread, run, this);

		transportStatus->setStatus(StatusReporter::STATUS_ONLINE());
	}

	/** Stop the plugin and wait for the request-handling thread */
	virtual void shutdown()
	{
		SAG_LOCK_GUARD queueLockGuard(queueLock);
		if(!running) {
			return;
		}
		
		// signal the thread to stop
		running = false;
		queueLockGuard.unlock();
		SAG_SEMA_POST(queueSemaphore);
		// then wait for it to terminate
		SAG_THREAD_JOIN(thread);
	}

	/** Parse the request and queue it for later servicing */
	virtual void deliverMessageTowardsTransport(Message &m) 
	{
		std::string id = "0";
		std::string url;
		std::vector<std::pair<std::string, std::string> > headers;
		try {
			// got to have something here (will throw if http isn't present or isn't a map)
			auto &d = m.getMetadataMap()["http"];
			auto &httpmap = get<map_t>(d);

			// id is optional
			auto id_it = m.getMetadataMap().find("requestId");
			if (id_it != httpmap.end()) {
				id = convert_to<std::string>(id_it->second);
			}
			// url is not.
			auto path_it = httpmap.find("path");
			if (path_it == httpmap.end()) return sendError("400", "Path must be specified", "", id);
			url = "http://"+host+":"+port+get<const char*>(path_it->second);
			// must be a GET request
			auto method_it = httpmap.find("method");
			if (method_it == httpmap.end()) return sendError("400", "Method must be specified", url, id);
			if (std::string("GET") != get<const char*>(method_it->second)) {
				return sendError("405", "Method not supported", url, id);
			}
			// loop over the additional headers
			auto hs = httpmap.find("headers");
			if (hs != httpmap.end()) {
				for (auto it = get<map_t>(hs->second).begin(); it != get<map_t>(hs->second).end(); ++it) {
					headers.push_back(std::make_pair(get<const char*>(it->first), get<const char*>(it->second)));
				}
			}
		} catch (const std::exception &e) {
			// other parse errors are caught here
			return sendError("400", e.what(), url, id);
		}
		// queue a request and signal the thread to wake up
		SAG_LOCK_GUARD queueLockGuard(queueLock);
		logger.debug("Queued request: url=%s\n", url.c_str());
		requestQueue.push_back(Request(id, url, std::move(headers)));
		queueLockGuard.unlock();
		SAG_SEMA_POST(queueSemaphore);
	}

private:

	/** Structure for storing requests in the queue */
	struct Request
	{
		Request() {}
		Request(const std::string &id, const std::string &url, std::vector<std::pair<std::string, std::string> > &&headers)
			: id(id), url(url), headers(headers) {}
		std::string id;
		std::string url;
		std::vector<std::pair<std::string, std::string> > headers;
	};

	/** Send back an error message */
	void sendError(const char *code, const char *message, const std::string &url, const std::string &id)
	{
		errorsTowardsHost->increment();

		Message m;
		map_t http;
		http["statusCode"] = code;
		http["statusReason"] = message;
		m.putMetadataValue("http", std::move(http));
		m.putMetadataValue("requestId", id);
		
		// need to ensure that only one thread is calling into hostSide->sendBatchTowardsHost at any one time
		SAG_LOCK_GUARD queueLockGuard(queueLock);
		hostSide->sendBatchTowardsHost(&m, &m+1);
	}

	/** Send back a full response */
	void sendResponse(const char *body, const std::string &url, const std::string &id, const std::vector<std::pair<std::string, std::string> > &headers)
	{
		Message m;
		map_t http;
		map_t hs;
		for (std::vector<std::pair<std::string, std::string> >::const_iterator it = headers.begin(); it != headers.end(); ++it) {
			hs[it->first] = it->second;
		}
		http["headers"] = std::move(hs);
		http["statusCode"] = "200";
		m.putMetadataValue("http", std::move(http));
		m.putMetadataValue("requestId", id);
		size_t len = strlen(body);
		buffer_t buff{len};
		std::copy(body, body+len, buff.begin());
		m.setPayload(std::move(buff));
		// need to ensure that only one thread is calling into hostSide->sendBatchTowardsHost at any one time
		SAG_LOCK_GUARD queueLockGuard(queueLock);
		hostSide->sendBatchTowardsHost(&m, &m+1);

		responsesTowardsHost->increment();
	}

	/** Parse the request and actually perform it. Returns the full document including headers */
	static std::string getRequest(HTTPClient *client, const Request &r)
	{
		// check it's the correct format
		size_t hostend = r.url.find("/", 7);
		if (hostend == std::string::npos) throw std::runtime_error("Malformed URI: "+r.url);

		// get the host and path
		std::string host = r.url.substr(7, hostend-7);
		std::string path = r.url.substr(hostend);
		
		int port = 80;
		// get the port
		hostend = host.find(":");
		if (hostend != std::string::npos) {
			port = atoi(host.substr(hostend+1).c_str());
			host = host.substr(0, hostend);
		}

		client->logger.debug("Request: host=%s port=%d path=%s", host.c_str(), port, path.c_str());

		// lookup the host and connect
		SOCKET sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		struct hostent *hostr = gethostbyname(host.c_str());
		if (!hostr) throw std::runtime_error("Could not look up host: "+host);
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = *((unsigned long*)hostr->h_addr);
		if (connect(sock,(struct sockaddr *) &addr,sizeof(struct sockaddr_in)) != 0) {
			throw std::runtime_error("Failed to connect to: "+r.url);
		}

		// construct the request
		std::ostringstream os;
		os << "GET " << path << " HTTP/1.1\r\n";
		os << "Host: " << host << "\r\n";
		os << "Accept-Charset: utf-8\r\n";
		os << "User-Agent: Apama-HTTPClientSample/1.1\r\n";
		os << "Connection: close\r\n";
		for (std::vector<std::pair<std::string, std::string> >::const_iterator it = r.headers.begin(); it != r.headers.end(); ++it) {
			os << it->first << ": " << it->second << "\r\n";
		}
		os << "\r\n";
		client->logger.trace("Data: %s", os.str().c_str());

		// send the request
		send(sock, os.str().c_str(), (int) strlen(os.str().c_str()), 0);

		// read the response
		std::ostringstream rs;
		char buffer[1001];
		int64_t len;
		while ((len = recv(sock, buffer, 1000, 0)) > 0) {
			buffer[len] = '\0';
			rs << buffer;
		}
		closesocket(sock);

		// check for exceptions
		if (len < 0) {
			throw std::runtime_error(strerror(errno));
		}

		// return the response document
		client->logger.trace("Reply: %s", rs.str().c_str());
		return rs.str();
	}

	/**
	 * Attempts to parse out the first chunk from Transport-Encoding: chunked messages.
	 * Note: this makes no attempt to read subsequent chunks which a fully-HTTP-complient
	 * implementation would have to do.
	 */
	static std::string parseChunkedEncoding(const std::string &body)
	{
		size_t chunkOffs = body.find("\r\n");
		if (chunkOffs == std::string::npos) return body;
		int64_t number = strtol(body.substr(0, chunkOffs).c_str(), NULL, 16);
		if (number < 0) return body;
		return body.substr(chunkOffs+2, number);
	}

	/**
	 * Run method that handles requests from the queue, does the HTTP request 
	 * and then sends any response
	 */
	static SAG_THREAD_RET_T run(void *args)
	{
		HTTPClient *client = (HTTPClient*) args;

		while (true) {

			// check the queue
			Request r;
			{
				SAG_LOCK_GUARD queueLockGuard(client->queueLock);
				// return if we have stopped
				if (!client->running) {
					SAG_THREAD_RETURN;
				}
				// get the first item
				std::vector<Request>::iterator it = client->requestQueue.begin();

				// wait if the queue is empty
				if (it == client->requestQueue.end()) {
					queueLockGuard.unlock();
					SAG_SEMA_WAIT(client->queueSemaphore);
					continue;
				}

				// dequeue the next request
				r = *it;
				client->requestQueue.erase(it);
			}

			// service the request
			client->logger.debug("Handling request: url=%s\n", r.url.c_str());
			try {
				// do the HTTP request
				std::string response = getRequest(client, r);

				// parse the response headers
				size_t offs = 0;
				std::string line;
				std::string responseCode;
				std::vector<std::pair<std::string, std::string> > headers;
				bool isChunked = false;
				do {
					size_t next = response.find("\r\n", offs);
					line = response.substr(offs, next-offs);
					size_t split;
					if (0 == line.find("HTTP/1.1")) {
						responseCode = line.substr(9,3);
					} else if (std::string::npos != (split = line.find(":"))) {
						std::string key = line.substr(0, split);
						std::transform(key.begin(), key.end(), key.begin(), ::tolower);
						headers.push_back(std::make_pair(key, line.substr(split+2)));
						if (line.substr(0, split) == "Transfer-Encoding" && line.substr(split+2) == "chunked") isChunked = true;
					}
					offs = next+2;
				} while (!line.empty());

				std::string body = response.substr(offs);
				if (isChunked) {
					body = parseChunkedEncoding(body);
				}

				// send the appropriate response
				if ("200" == responseCode) {
					client->sendResponse(body.c_str(), r.url, r.id, headers);
				} else {
					client->sendError(responseCode.c_str(), body.c_str(), r.url, r.id);
				}
			} catch (const std::exception &e) {
				// handle any error in sending or parsing the request
				client->sendError("500", e.what(), r.url, r.id);
			}
		}
	}

	// transport members
	
	/** Protected by queueLock. queueSemaphore is used to signal when new items have been added. */
	std::vector<Request> requestQueue;
	
	/** Configuration of the host to connect to. */
	std::string host;
	/** Configuration of the port to connect on. */
	std::string port;
		
	/** Protected by queueLock. */
	bool running;

	// status reporting
	StatusReporter::item_ptr transportStatus;
	StatusReporter::item_ptr errorsTowardsHost;
	StatusReporter::item_ptr responsesTowardsHost;
	StatusReporter::item_ptr kpis;

	/*
	 * WARNING: We use the threading primitives from sag_connectivity_threading.h 
	 * only in order to support older compilers. If your compiler supports
	 * C++11 std::thread and std::mutex/condition_variable you should use those instead.
	 */
	SAG_MUTEX_T queueLock;
	SAG_SEMA_T queueSemaphore;
	SAG_THREAD_T thread;

};

/** Export this transport */
SAG_DECLARE_CONNECTIVITY_TRANSPORT_CLASS(HTTPClient)


}}} // com.softwareag.samples


