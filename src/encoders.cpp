#include "encoders.h"
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>
#include "uri.h"

using boost::property_tree::ptree;
using boost::property_tree::read_json;
using boost::property_tree::write_json;

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
            ptree pt;
            for (auto &entry : values) {
                pt.put(entry.first, entry.second);
            }
            std::ostringstream buf;
            write_json(buf, pt, false);
            return buf.str();
        }
        jsonencoder::type jsonencoder::encode(const std::string &value) const
        {
            return value;
        }
    }
}