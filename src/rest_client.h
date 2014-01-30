#ifndef ARG3_NET_REST_CLIENT_H
#define ARG3_NET_REST_CLIENT_H

#include "config.h"

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
        class rest_client
        {
        public:
            rest_client(const string &host);
            rest_client();
            virtual ~rest_client();
            rest_client(const rest_client &other);
            rest_client(rest_client &&other);
            rest_client &operator=(const rest_client &other);
            rest_client &operator=(rest_client && other);

            /*!
             * adds an HTTP header to the request
             */
            void add_header(const string &key, const string &value);

            /*!
             * Removes an HTTP header from the request
             */
            void remove_header(const string &key);

            /*!
             * returns the value for an HTTP header for this request
             */
            string header(const string &key);

            /*!
             * returns the host used to connect
             */
            string host() const;

            /*!
             * the post data usually
             */
            string payload() const;

            /*!
             * the HTTP response code
             */
            int response_code() const;

            /*!
             * the response body
             */
            string response() const;

            /*!
             * @returns true if the request is using HTTPS
             */
            bool is_secure() const;

            /*!
             * sets the host for this request
             */
            void set_host(const string &);

            /*!
             * sets the payload for this request
             */
            rest_client &set_payload(const string &value);

            /*!
             * performs a request
             */
            rest_client &request(http::method method, const string &path);

            /*!
             * performs a GET request
             */
            rest_client &get(const string &path);

            /*!
             * performs a POST request
             */
            rest_client &post(const string &path);

            /*!
             * performs a PUT request
             */
            rest_client &put(const string &path);

            /*!
             * performs a DELETE request
             */
            rest_client &de1ete(const string &path);

            /*!
             * sets whether this request uses HTTPS
             */
            void set_secure(bool value);

        private:
            string scheme_;
            string host_;
            string payload_;
            int responseCode_;
            string response_;
            map<string, string> headers_;
        };
    }
}

#endif
