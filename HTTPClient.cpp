/*
 * Title:        HTTPClient.cpp
 * Description:  HTTPClient Transport
 * $Copyright (c) 2016, 2017 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * $Revision: 321386 $ $Date: 2017-12-14 10:26:04 +0000 (Thu, 14 Dec 2017) $
 */

#include <sag_connectivity_plugins.hpp>
// #include <sag_connectivity_threading.h>
#include "HTTPClient.h"
#include <vector>

#include "gowrapper.h"

using namespace com::softwareag::connectivity;

namespace com {
namespace softwareag {
namespace samples {


	HTTPClient::HTTPClient(const TransportConstructorParameters &params)
		: AbstractSimpleTransport(params),
		// host(),
		// port(),
		// running(false), 
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
		// host = configEx.getStringDisallowEmpty("host");
		// port = configEx.getStringDisallowEmpty("port", "80");
		configEx.checkNoItemsRemaining();

		go_init(this);
	}

	void HTTPClient::start()
	{
		logger.error("start called");
		Start();

		char buf[11] = "HelloWorld";
		DeliverMessageTowardsTransport(buf, sizeof(buf)-1);

		transportStatus->setStatus(StatusReporter::STATUS_ONLINE());
	}

	/** Stop the plugin and wait for the request-handling thread */
	void HTTPClient::shutdown()
	{
		Stop();
	}

	/** Parse the request and queue it for later servicing */
	void HTTPClient::deliverMessageTowardsTransport(Message &m)
	{
	}

	void HTTPClient::towardsHost(char* buf, int bufLen) {
		// Message m;
		// hostSide->sendBatchTowardsHost(&m, &m+1);
		logger.error("##### towards host called, %p, %d", buf, bufLen);
		char lbuf[100];
		for(int i=0;i<bufLen;i++) {
			lbuf[i] = buf[i];
		}
		lbuf[bufLen] = '\0';
		logger.error("Received: %s", lbuf);
	}

/** Export this transport */
SAG_DECLARE_CONNECTIVITY_TRANSPORT_CLASS(HTTPClient)

}}} // com.softwareag.samples


