#ifndef INCLUDE__HTTPClient__
#define INCLUDE__HTTPClient__
/*
 * Title:        HTTPClient.cpp
 * Description:  HTTPClient Transport
 * $Copyright (c) 2016, 2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * $Revision: 321386 $ $Date: 2017-12-14 10:26:04 +0000 (Thu, 14 Dec 2017) $
 */

#include <sag_connectivity_plugins.hpp>

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
class HTTPClient: public com::softwareag::connectivity::AbstractSimpleTransport
{

public:
	/** Create the transport and check we have the correct (no) config */
	HTTPClient(const TransportConstructorParameters &params);
	
	/** Start the request-handling thread */
	virtual void start();

	/** Stop the plugin and wait for the request-handling thread */
	virtual void shutdown();

	/** Parse the request and queue it for later servicing */
	virtual void deliverMessageTowardsTransport(com::softwareag::connectivity::Message &m) override;

    void towardsHost(char* buf, int bufLen);

private:
	// status reporting
	com::softwareag::connectivity::StatusReporter::item_ptr transportStatus;
	com::softwareag::connectivity::StatusReporter::item_ptr errorsTowardsHost;
	com::softwareag::connectivity::StatusReporter::item_ptr responsesTowardsHost;
	com::softwareag::connectivity::StatusReporter::item_ptr kpis;

};

}}} // com.softwareag.samples
#endif