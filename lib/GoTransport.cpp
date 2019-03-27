/*
 * $Copyright (c) 2019 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * $Revision: 321386 $ $Date: 2017-12-14 10:26:04 +0000 (Thu, 14 Dec 2017) $
 */

#include <sag_connectivity_plugins.hpp>
// #include <sag_connectivity_threading.h>
#include "GoTransport.h"
#include <vector>

#include "_cgo_export.h"

using namespace com::softwareag::connectivity;

namespace apamax {
namespace golang {
	GoTransport::GoTransport(const TransportConstructorParameters &params)
		: AbstractSimpleTransport(params)
	{
		go_transport_create(this);
	}

	void GoTransport::start()
	{
		logger.info("C++ start called");
		go_transport_start();

		// char buf[11] = "HelloWorld";
		// CallIntoTransport(buf, sizeof(buf)-1);
	}

	/** Stop the plugin and wait for the request-handling thread */
	void GoTransport::shutdown()
	{
		go_transport_shutdown();
	}

	/** Parse the request and queue it for later servicing */
	void GoTransport::deliverMessageTowardsTransport(Message &m)
	{
		logger.info("C++ deliverMessageTowardsTransport: %s", to_string(m).c_str());
		auto payload = get<std::string>(m.getPayload());
		char * str = const_cast<char*>(payload.c_str());
		go_transport_deliverMessageTowardsTransport(static_cast<void*>(str), payload.size());
	}

	void GoTransport::towardsHost(char* buf, int bufLen) {
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
SAG_DECLARE_CONNECTIVITY_TRANSPORT_CLASS(GoTransport)

}} // apamax.golang


