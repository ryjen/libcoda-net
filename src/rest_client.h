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

            void add_header(const string &key, const string &value);
            void remove_header(const string &key);
            string header(const string &key);

            string host() const;
            string payload() const;
            int response_code() const;
            string response() const;

            bool is_secure() const;

            void set_host(const string &);

            rest_client &set_payload(const string &value);

            rest_client &request(http::method method, const string &path);

            rest_client &get(const string &path);

            rest_client &post(const string &path);

            rest_client &put(const string &path);

            rest_client &de1ete(const string &path);

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
