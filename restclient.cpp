#include "restclient.h"
#include "exception.h"
#include "../log/log.h"

namespace arg3
{
    namespace net
    {
        size_t curl_append_response_callback(void *ptr, size_t size, size_t nmemb, string *s)
        {
            if(s == NULL) return 0;

            const size_t new_len = size * nmemb;

            char buf[new_len + 1];

            memcpy(buf, ptr, size * nmemb);

            buf[new_len] = '\0';

            s->append(buf);

            return new_len;
        }

        RESTClient::RESTClient(const string &host, const string &version) : curl_(curl_easy_init()), scheme_(http::PROTOCOL), host_(host), version_(version)
        {
            if (curl_ == NULL)
            {
                throw RESTException("unable to initialize request");
            }

            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_append_response_callback);
        }

        RESTClient::RESTClient() : curl_(curl_easy_init())
        {
            if (curl_ == NULL)
            {
                throw RESTException("unable to initialize request");
            }

            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_append_response_callback);
        }

        void RESTClient::addHeader(const string &key, const string &value)
        {
            headers_[key] = value;
        }

        void RESTClient::removeHeader(const string &key)
        {
            headers_.erase(key);
        }

        string RESTClient::getHeader(const string &key)
        {
            return headers_[key];
        }

        string RESTClient::getVersion() const {
            return version_;
        }

        string RESTClient::getHost() const {
            return host_;
        }

        int RESTClient::getResponseCode() const {
            return responseCode_;
        }

        string RESTClient::getPayload() const {
            return payload_;
        }

        string RESTClient::getResponse() const {
            return response_;
        }

        bool RESTClient::isSecure() const {
            return scheme_ == http::SECURE_PROTOCOL;
        }

        void RESTClient::setHost(const string &host) {
            host_ = host;
        }

        void RESTClient::setVersion(const string &version) {
            version_ = version;
        }

        void RESTClient::setSecure(bool value) {
            scheme_ = value ? http::SECURE_PROTOCOL : http::PROTOCOL;
        }

        RESTClient &RESTClient::setPayload(const string &payload) {
            payload_ = payload;
            return *this;
        }

        RESTClient& RESTClient::request(http::Method method, const string& path)
        {
            struct curl_slist *headers = NULL;

            char buf[http::MAX_URL_LEN+1];

            snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s/%s", scheme_.c_str(), host_.c_str(), version_.c_str(), path.c_str());

            curl_easy_setopt(curl_, CURLOPT_URL, buf);

            switch(method)
            {
            case http::Get:
                curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
                break;
            case http::Post:
                curl_easy_setopt(curl_, CURLOPT_POST, 1L);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_.size());
                break;
            case http::Put:
                curl_easy_setopt(curl_, CURLOPT_PUT, 1L);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_.size());
            case http::Delete:
                curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, http::DELETE);
                break;
            }

            for(auto &h : headers_)
            {
                snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());

                headers = curl_slist_append(headers, buf);
            }

            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_);

            CURLcode res = curl_easy_perform(curl_);

            if (res != CURLE_OK)
            {
                throw RESTException(curl_easy_strerror(res));
            }

            curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &responseCode_);

            /* always cleanup */
            curl_easy_cleanup(curl_);

            return *this;
        }

        RESTClient& RESTClient::get(const string &path)
        {
            return request(http::Get, path);
        }

        RESTClient& RESTClient::post(const string &path)
        {
            return request(http::Post, path);
        }

        RESTClient& RESTClient::put(const string &path)
        {
            return request(http::Put, path);
        }

        RESTClient& RESTClient::de1ete(const string &path)
        {
            curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, http::DELETE);

            return request(http::Delete, path);
        }
    }
}