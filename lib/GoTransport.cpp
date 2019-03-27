/*
 * $Copyright (c) 2019 Software AG, Darmstadt, Germany and/or Software AG USA Inc., Reston, VA, USA, and/or its subsidiaries and/or its affiliates and/or their licensors.$
 * Use, reproduction, transfer, publication or disclosure is prohibited except as specifically provided for in your License Agreement with Software AG
 * $Revision: 321386 $ $Date: 2017-12-14 10:26:04 +0000 (Thu, 14 Dec 2017) $
 */

#include <sag_connectivity_plugins.hpp>
// #include <sag_connectivity_threading.h>
#include "GoTransport.h"
#include <vector>
#include<sstream>
#include <cmath>
#include "_cgo_export.h"

using namespace com::softwareag::connectivity;

namespace apamax {
namespace golang {
	/** data_t -> JSON */
	struct JsonVisitor : public const_visitor <JsonVisitor, void> {
		explicit JsonVisitor(std::ostringstream &w) : writer(w) {}
		void visitString(const char *s) const { writer << "\"" << s << "\""; }
		void visitInteger(int64_t i) const { writer << i; }
		void visitDouble(double d) const {
			if(d == INFINITY) { writer << "\"Infinity\""; }
			else if(d == -INFINITY) { writer << "\"-Infinity\""; }
			else if(std::isnan(d)) { writer << "\"NaN\""; }
			else { writer << d; }
		}
		void visitBoolean(bool b) const { 
			if (b) {
				writer << "true";
			} else {
				writer << "false";
			}
		 }
		void visitDecimal(const decimal_t &v) const {
			// decimal64 d;
			// char buf[decimal64::BUFFER_SIZE];
			// d.setUnderlyingInteger(v.d);
			// d.toString(buf);
			// writer << ap_strtod(buf, nullptr);
			writer << "\"<decimal>\"";
		}
		void visitList(const list_t &li) const {
			writer << "[";
			for (auto it = li.begin(); it != li.end(); ++it) {
				const data_t &dat = *it;
				apply_visitor(JsonVisitor(writer), dat);
				if (it != --li.end()) {
					writer << ",";
				}
			}
			writer << "]";
		}
		void visitMap(const map_t &m) const {
			writer << "{";
			for (auto it = m.begin(); it != m.end(); ++it) {
				const data_t &k = it.key();
				const data_t &v = it.value();
				if(k.type_tag() == SAG_DATA_STRING) {
					writer << "\"" << get<const char*>(k) << "\"";
				} else {
					writer << "\"" << convert_to<std::string>(k).c_str() << "\"";
				}
				writer << ":";
				apply_visitor(JsonVisitor(writer), v);
				if (it != --m.end()) {
					writer << ",";
				}
			}
			writer << "}";
		}
		void visitEmpty() const {
			writer << "\"Null\"";
		}
		void error(const std::string &type) const {
			throw std::runtime_error("Unsupported type in Transportwards message: " + type);
		}

	private:
		std::ostringstream &writer;
	};



	GoTransport::GoTransport(const TransportConstructorParameters &params)
		: AbstractSimpleTransport(params)
	{
		

		map_t &config = const_cast<map_t&>(params.getConfig());
		logger.info("C++ config: %s", to_string(config).c_str());
		std::ostringstream os;
		apply_visitor(JsonVisitor(os), data_t(std::move(config)));
		logger.info("Config JSON: %s", os.str().c_str());
		std::string content = os.str();
		//char * str = const_cast<char*>(payload.c_str());
		go_transport_create(this, static_cast<void*>(const_cast<char*>(content.c_str())), content.size());
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

	void convertConfigToJson() {
		//ostringstream os;


	}

/** Export this transport */
SAG_DECLARE_CONNECTIVITY_TRANSPORT_CLASS(GoTransport)

}} // apamax.golang


