#ifndef ARG3_NET_HTTP_CLIENT_H
#define ARG3_NET_HTTP_CLIENT_H

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

#include <map>
#include <string>
#include "protocol.h"

namespace arg3
{
    namespace net
    {
        class http_transfer
        {
           public:
            http_transfer();
            http_transfer(const http_transfer &);
            http_transfer(http_transfer &&);
            virtual ~http_transfer();

            http_transfer &operator=(const http_transfer &);
            http_transfer &operator=(http_transfer &&other);

            /*!
             * the post data usually
             */
            std::string payload() const;

            std::string version() const;

            void set_version(const std::string &value);

            /*!
             * returns the value for an HTTP header for this request
             */
            std::string header(const std::string &key);

            const std::map<std::string, std::string> headers() const;

           protected:
            std::string payload_;
            std::map<std::string, std::string> headers_;
            std::string version_;
        };

        class http_response : public http_transfer
        {
           public:
            http_response();
            http_response(const std::string &);
            http_response(const http_response &);
            http_response(http_response &&);
            virtual ~http_response();

            http_response &operator=(const http_response &);
            http_response &operator=(http_response &&other);

            std::string full_response() const;

            operator std::string() const;

            /*!
             * the HTTP response code
             */
            int code() const;

           private:
            void clear();

            void parse();
            void parse(const std::string &);

            std::string response_;

            int responseCode_;

            friend class http_client;
        };

        class http_client : public http_transfer
        {
           public:
            http_client(const std::string &host);
            http_client();
            virtual ~http_client();
            http_client(const http_client &other);
            http_client(http_client &&other);
            http_client &operator=(const http_client &other);
            http_client &operator=(http_client &&other);

            /*!
             * adds an HTTP header to the request
             */
            http_client &add_header(const std::string &key, const std::string &value);

            /*!
             * Removes an HTTP header from the request
             */
            void remove_header(const std::string &key);

            /*!
             * returns the host used to connect
             */
            std::string host() const;

            /*!
             * the response body
             */
            http_response response() const;

            /*!
             * @returns true if the request is using HTTPS
             */
            bool is_secure() const;

            /*!
             * sets the host for this request
             */
            http_client &set_host(const std::string &);

            /*!
             * sets the payload for this request
             */
            http_client &set_payload(const std::string &value);

            /*!
             * performs a request
             */
            http_client &request(http::method method, const std::string &path);

            /*!
             * performs a GET request
             */
            http_client &get(const std::string &path);

            /*!
             * performs a POST request
             */
            http_client &post(const std::string &path);

            /*!
             * performs a PUT request
             */
            http_client &put(const std::string &path);

            /*!
             * performs a DELETE request
             */
            http_client &de1ete(const std::string &path);

            /*!
             * sets whether this request uses HTTPS
             */
            http_client &set_secure(bool value);

            http_client &set_timeout(int value);

           private:
#ifdef HAVE_LIBCURL
            void request_curl(http::method method, const std::string &path);
#else
            void request_socket(http::method method, const std::string &path);
#endif
            std::string scheme_;
            std::string host_;
            int timeout_;
            http_response response_;
        };
    }
}

#endif
