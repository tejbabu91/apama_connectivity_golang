#include <sag_connectivity_plugins.hpp>
#include <iostream>
#include <string>

using namespace com::softwareag::connectivity;

class GoMapperCodec: public AbstractSimpleCodec{
    public:
        GoMapperCodec(const CodecConstructorParameters &config)
		: AbstractSimpleCodec(config) {}

        virtual bool transformMessageTowardsTransport(Message &m)
        {
            map_t metadata = std::move(m.getMetadataMap());
            data_t payload = std::move(m.getPayload());
            map_t data;
            data.insert(std::make_pair("data", std::move(payload)));
            data.insert(std::make_pair("metadata", std::move(metadata)));
            m.setPayload(std::move(data));
            logger.info("Message TowardsTransport:%s", to_string(m).c_str());
            return true;
        }

        virtual bool transformMessageTowardsHost(Message &m)
        {
            map_t mp = std::move(get<map_t>(m.getPayload()));
            data_t payload = std::move(mp.find("data")->second);
            data_t metadata = std::move(mp.find("metadata")->second);
            m.setMetadata(std::move(get<map_t>(metadata)));
            m.setPayload(std::move(payload));
            logger.info("Message TowardsHost: %s", to_string(m).c_str());
            return true;
        }

};

SAG_DECLARE_CONNECTIVITY_CODEC_CLASS(GoMapperCodec)