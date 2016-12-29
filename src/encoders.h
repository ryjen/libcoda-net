#ifndef RJ_NET_ENCODERS_H
#define RJ_NET_ENCODERS_H

#include <map>
#include <string>
#include <vector>

namespace rj
{
    namespace net
    {
        template <typename T>
        class encoder
        {
           public:
            typedef T type;

            encoder() = default;
            encoder(const encoder &) = default;
            encoder(encoder &&) = default;
            virtual ~encoder() = default;
            encoder &operator=(const encoder &) = default;
            encoder &operator=(encoder &&) = default;

            virtual type encode(const std::map<std::string, std::string> &values) const = 0;

            virtual type encode(const std::string &value) const = 0;
        };

        class urlencoder : public encoder<std::string>
        {
           public:
            encoder::type encode(const std::map<std::string, std::string> &values) const;
            encoder::type encode(const std::string &value) const;
        };

        class jsonencoder : public encoder<std::string>
        {
           public:
            encoder::type encode(const std::map<std::string, std::string> &values) const;
            encoder::type encode(const std::string &value) const;
        };
    }
}

#endif
