#include "restclient.h"
#include "exception.h"

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

        RESTClient::RESTClient(const string &host, const string &version) : scheme_(http::SCHEME), host_(host), version_(version)
        {
        }

        RESTClient::RESTClient()
        {
        }

        RESTClient::~RESTClient()
        {
        }

        RESTClient::RESTClient(const RESTClient &other) : scheme_(other.scheme_), host_(other.host_),
            version_(other.version_), payload_(other.payload_), responseCode_(other.responseCode_),
            response_(other.response_), headers_(other.headers_)
        {
        }

        RESTClient::RESTClient(RESTClient &&other) : scheme_(std::move(other.scheme_)), host_(std::move(other.host_)),
            version_(std::move(other.version_)), payload_(std::move(other.payload_)), responseCode_(other.responseCode_),
            response_(std::move(other.response_)), headers_(std::move(other.headers_))
        {
        }

        RESTClient &RESTClient::operator=(const RESTClient &other)
        {
            if(this != &other)
            {
                scheme_ = other.scheme_;
                host_ = other.host_;
                version_ = other.version_;
                payload_ = other.payload_;
                responseCode_ = other.responseCode_;
                response_ = other.response_;
                headers_ = other.headers_;
            }
            return *this;
        }

        RESTClient &RESTClient::operator=(RESTClient &&other)
        {
            if(this != &other)
            {
                scheme_ = std::move(other.scheme_);
                host_ = std::move(other.host_);
                version_ = std::move(other.version_);
                payload_ = std::move(other.payload_);
                responseCode_ = std::move(other.responseCode_);
                response_ = std::move(other.response_);
                headers_ = std::move(other.headers_);
            }
            return *this;
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
            return scheme_ == http::SECURE_SCHEME;
        }

        void RESTClient::setHost(const string &host) {
            host_ = host;
        }

        void RESTClient::setVersion(const string &version) {
            version_ = version;
        }

        void RESTClient::setSecure(bool value) {
            scheme_ = value ? http::SECURE_SCHEME : http::SCHEME;
        }

        RESTClient &RESTClient::setPayload(const string &payload) {
            payload_ = payload;
            return *this;
        }

        RESTClient& RESTClient::request(http::Method method, const string& path)
        {
            struct curl_slist *headers = NULL;

            char buf[http::MAX_URL_LEN+1];

            CURL *curl_ = curl_easy_init();

            if (curl_ == NULL)
            {
                throw RESTException("unable to initialize request");
            }

            snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s/%s", scheme_.c_str(), host_.c_str(), version_.c_str(), path.c_str());

            curl_easy_setopt(curl_, CURLOPT_URL, buf);

            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_append_response_callback);

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
                curl_easy_cleanup(curl_);

                throw RESTException(curl_easy_strerror(res));
            }

            curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &responseCode_);

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
            return request(http::Delete, path);
        }
    }
}