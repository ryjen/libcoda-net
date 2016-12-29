#include "encoders.h"
#include <cereal/archives/json.hpp>
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include "uri.h"

namespace rj
{
    namespace net
    {
        urlencoder::type urlencoder::encode(const std::map<std::string, std::string> &values) const
        {
            std::ostringstream buf;
            for (auto it = values.begin(); it != values.end(); ++it) {
                buf << uri::encode(it->first);
                buf << uri::encode("=");
                buf << uri::encode(it->second);
                if (std::next(it) != values.end()) {
                    buf << uri::encode("&");
                }
            }
            return buf.str();
        }
        urlencoder::type urlencoder::encode(const std::string &value) const
        {
            return uri::encode(value);
        }

        jsonencoder::type jsonencoder::encode(const std::map<std::string, std::string> &values) const
        {
            std::ostringstream buf;
            cereal::JSONOutputArchive archive(buf);
            for (auto &entry : values) {
                archive << cereal::make_nvp(entry.first, entry.second);
            }
            return buf.str();
        }
        jsonencoder::type jsonencoder::encode(const std::string &value) const
        {
            return value;
        }
    }
}