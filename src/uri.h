#ifndef CODA_NET_URI_H
#define CODA_NET_URI_H

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
            uri() noexcept;
            uri(const std::string &uri, const std::string &defaultScheme = "http");
            uri(const uri &other) = default;
            uri(uri &&other) = default;
            ~uri() noexcept;
            uri &operator=(const uri &other) = default;
            uri &operator=(uri &&other) = default;

            bool is_valid() const noexcept;

            std::string scheme() const noexcept;
            std::string username() const noexcept;
            std::string password() const noexcept;
            std::string host() const noexcept;
            std::string host_with_port() const;
            std::string port() const noexcept;
            std::string path() const noexcept;
            std::string query() const noexcept;
            std::string fragment() const noexcept;
            std::string full_path() const;

            std::string to_string() const noexcept;
            operator std::string() const noexcept;

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
