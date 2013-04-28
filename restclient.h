
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

            void addHeader(const string &key, const string &value);
            void removeHeader(const string &key);
            string getHeader(const string &key);

            string getHost() const;
            string getVersion() const;
            string getPayload() const;
            int getResponseCode() const;
            string getResponse() const;

            void setHost(const string &);
            void setVersion(const string &);

            RESTClient& setPayload(const string &);

            virtual RESTClient& request(http::Method method, const string &path);

            virtual RESTClient& get(const string &path);

            virtual RESTClient& post(const string &path);

            virtual RESTClient& put(const string &path);

            virtual RESTClient& de1ete(const string &path);

            void setSecure(bool value);

        private:
            CURL *curl_;
            string protocol_;
            string host_;
            string version_;
            string payload_;
            int responseCode_;
            string response_;
            map<string,string> headers_;
        };
    }
}