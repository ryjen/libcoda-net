
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

            void addHeader(const string &key, const string &value);
            void removeHeader(const string &key);

            string getHost() const;
            string getVersion() const;
            int getResult() const;

            string get(const string &path);

            string post(const string &path, const string &payload="");

            string put(const string &path, const string &payload="");

            string de1ete(const string &path);

            void setSecure(bool value);

        protected:
            string perform_request(const string &path);

        private:
            string protocol_;
            string host_;
            string version_;
            CURL *curl_;
            int result_;
            map<string,string> headers_;
        };
    }
}