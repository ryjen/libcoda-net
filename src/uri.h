#ifndef CODA_NET_URI_H_
#define CODA_NET_URI_H_

#ifdef URIPARSER_FOUND
#include <uriparser/Uri.h>
#endif

#include <string>

namespace coda
{
    namespace net
    {
        class uri
        {
           public:
            uri();
            uri(const std::string &uri, const std::string &defaultScheme = "http");
            uri(const uri &other) = default;
            uri(uri &&other) = default;
            ~uri();
            uri &operator=(const uri &other) = default;
            uri &operator=(uri &&other) = default;

            bool is_valid() const;

            std::string scheme() const;
            std::string username() const;
            std::string password() const;
            std::string host() const;
            std::string host_with_port() const;
            std::string port() const;
            std::string path() const;
            std::string query() const;
            std::string fragment() const;
            std::string full_path() const;

            std::string to_string() const;
            operator std::string() const;

            static std::string encode(const std::string &value);
            static std::string decode(const std::string &value);

           private:
            bool parse(const std::string &uri_s, const std::string &defaultScheme);
            std::string uri_;
            bool isValid_;
            std::string scheme_, user_, password_, host_, port_, path_, query_, fragment_;
        };
    }
}

#endif
