#include "encoders.h"
#include "uri.h"
#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <codecvt>
#include <iostream>
#include <locale>
#include <string>

namespace coda {
  namespace net {
    namespace encoding {
      void url::encode(const input::map &from, encoded_type &to) const {
        std::ostringstream buf;
        for (auto it = from.begin(); it != from.end(); ++it) {
          buf << it->first;
          buf << "=";
          buf << it->second;
          if (std::next(it) != from.end()) {
            buf << "&";
          }
        }
        to = uri::encode(buf.str());
      }

      void url::encode(const input::str &from, encoded_type &to) const {
        to = uri::encode(from);
      }

      void url::decode(const encoded_type &from, input::str &to) const {
        to = uri::decode(from);
      }

      void url::decode(const encoded_type &from, input::map &to) const {
        std::string::size_type pos = from.find('=');
        std::string::size_type last_pos = 0;

        while (pos != std::string::npos) {
          std::string key;
          std::string value;

          // parse the key
          key = from.substr(last_pos, pos);
          last_pos = from.find('&', pos);
          // parse the value
          value = from.substr(pos + 1, last_pos);

          to[key] = value;

          // find next
          pos = from.find('=', last_pos);
        }
      }

      void json::encode(const input::map &from, encoded_type &to) const {
        std::ostringstream buf;
        cereal::JSONOutputArchive archive(buf);

        for (auto &pair : from) {
          archive << cereal::make_nvp(pair.first, pair.second);
        }

        to = buf.str();
      }
      void json::encode(const input::str &from, encoded_type &to) const {
        // TODO: don't assume its a json string
        to = from;
      }

      void json::decode(const encoded_type &from, input::map &to) const {
        std::istringstream buf(from);
        cereal::JSONInputArchive archive(buf);

        auto namePtr = archive.getNodeName();
        while (namePtr) {
          std::string key = namePtr;
          std::string value;
          archive(value);
          to.emplace(std::move(key), std::move(value));
          namePtr = archive.getNodeName();
        }
      }

      void json::decode(const encoded_type &from, input::str &to) const {
        // TODO: don't assume its just a string
        to = from;
      }

      void xml::encode(const input::map &from, encoded_type &to) const {
        std::ostringstream buf;
        cereal::XMLOutputArchive archive(buf);

        for (auto &pair : from) {
          archive << cereal::make_nvp(pair.first, pair.second);
        }

        to = buf.str();
      }

      void xml::encode(const input::str &from, encoded_type &to) const {
        // TODO: don't assume its an xml string
        to = from;
      }

      void xml::decode(const encoded_type &from, input::map &to) const {
        std::istringstream buf(from);
        cereal::XMLInputArchive archive(buf);

        auto namePtr = archive.getNodeName();
        while (namePtr) {
          std::string key = namePtr;
          std::string value;
          archive(value);
          to.emplace(std::move(key), std::move(value));
          namePtr = archive.getNodeName();
        }
      }

      void xml::decode(const encoded_type &from, input::str &to) const {
        // TODO: don't assume its just a string
        to = from;
      }
    } // namespace encoding
  }   // namespace net
} // namespace coda