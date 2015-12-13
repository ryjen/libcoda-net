#ifndef ARG3_NET_http_client_H
#define ARG3_NET_http_client_H

#ifdef HAVE_LIBCURL
#include <curl/curl.h>
#endif

#include "protocol.h"
#include <string>
#include <map>

using namespace std;

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
            string payload() const;

            string version() const;

            void set_version(const string &value);

            /*!
             * returns the value for an HTTP header for this request
             */
            string header(const string &key);

            const map<string, string> headers() const;

           protected:
            string payload_;
            map<string, string> headers_;
            string version_;
        };

        class http_response : public http_transfer
        {
           public:
            http_response();
            http_response(const string &);
            http_response(const http_response &);
            http_response(http_response &&);
            virtual ~http_response();

            http_response &operator=(const http_response &);
            http_response &operator=(http_response &&other);

            string full_response() const;

            operator string() const;

            /*!
             * the HTTP response code
             */
            int code() const;

           private:
            void clear();

            void parse();
            void parse(const string &);

            string response_;

            int responseCode_;

            friend class http_client;
        };

        class http_client : public http_transfer
        {
           public:
            http_client(const string &host);
            http_client();
            virtual ~http_client();
            http_client(const http_client &other);
            http_client(http_client &&other);
            http_client &operator=(const http_client &other);
            http_client &operator=(http_client &&other);

            /*!
             * adds an HTTP header to the request
             */
            http_client &add_header(const string &key, const string &value);

            /*!
             * Removes an HTTP header from the request
             */
            void remove_header(const string &key);

            /*!
             * returns the host used to connect
             */
            string host() const;

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
            http_client &set_host(const string &);

            /*!
             * sets the payload for this request
             */
            http_client &set_payload(const string &value);

            /*!
             * performs a request
             */
            http_client &request(http::method method, const string &path);

            /*!
             * performs a GET request
             */
            http_client &get(const string &path);

            /*!
             * performs a POST request
             */
            http_client &post(const string &path);

            /*!
             * performs a PUT request
             */
            http_client &put(const string &path);

            /*!
             * performs a DELETE request
             */
            http_client &de1ete(const string &path);

            /*!
             * sets whether this request uses HTTPS
             */
            http_client &set_secure(bool value);

           private:
#ifdef HAVE_LIBCURL
            void request_curl(http::method method, const string &path);
#else
            void request_socket(http::method method, const string &path);
#endif
            string scheme_;
            string host_;
            http_response response_;
        };
    }
}

#endif
