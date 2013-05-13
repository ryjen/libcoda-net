#ifndef ARG3_NET_REST_CLIENT_H
#define ARG3_NET_REST_CLIENT_H

#include <curl/curl.h>
#include "protocol.h"
#include <string>
#include <map>

using namespace std;

namespace arg3
{
    namespace net
    {
        class RESTClient
        {
        public:
            RESTClient(const string &host, const string &version);
            RESTClient();
            virtual ~RESTClient();
            RESTClient(const RESTClient &other);
            RESTClient(RESTClient &&other);
            RESTClient &operator=(const RESTClient &other);
            RESTClient &operator=(RESTClient &&other);

            void addHeader(const string &key, const string &value);
            void removeHeader(const string &key);
            string getHeader(const string &key);

            string getHost() const;
            string getVersion() const;
            string getPayload() const;
            int getResponseCode() const;
            string getResponse() const;

            bool isSecure() const;

            void setHost(const string &);
            void setVersion(const string &);

            RESTClient& setPayload(const string &value);

            RESTClient& request(http::Method method, const string &path);

            RESTClient& get(const string &path);

            RESTClient& post(const string &path);

            RESTClient& put(const string &path);

            RESTClient& de1ete(const string &path);

            void setSecure(bool value);

        private:
            string scheme_;
            string host_;
            string version_;
            string payload_;
            int responseCode_;
            string response_;
            map<string,string> headers_;
        };
    }
}

#endif
