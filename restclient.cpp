#ifndef ARG3_NO_CURL

#include "restclient.h"
#include "exception.h"

namespace arg3
{
    namespace net
    {
        size_t curl_append_response_callback(void *ptr, size_t size, size_t nmemb, string *s)
        {
            if (s == NULL) return 0;

            const size_t new_len = size * nmemb;

            char buf[new_len + 1];

            memcpy(buf, ptr, size * nmemb);

            buf[new_len] = '\0';

            s->append(buf);

            return new_len;
        }

        rest_client::rest_client(const string &host, const string &version) : scheme_(http::SCHEME), host_(host), version_(version)
        {
        }

        rest_client::rest_client()
        {
        }

        rest_client::~rest_client()
        {
        }

        rest_client::rest_client(const rest_client &other) : scheme_(other.scheme_), host_(other.host_),
            version_(other.version_), payload_(other.payload_), responseCode_(other.responseCode_),
            response_(other.response_), headers_(other.headers_)
        {
        }

        rest_client::rest_client(rest_client &&other) : scheme_(std::move(other.scheme_)), host_(std::move(other.host_)),
            version_(std::move(other.version_)), payload_(std::move(other.payload_)), responseCode_(other.responseCode_),
            response_(std::move(other.response_)), headers_(std::move(other.headers_))
        {
        }

        rest_client &rest_client::operator=(const rest_client &other)
        {
            if (this != &other)
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

        rest_client &rest_client::operator=(rest_client && other)
        {
            if (this != &other)
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

        void rest_client::add_header(const string &key, const string &value)
        {
            headers_[key] = value;
        }

        void rest_client::remove_header(const string &key)
        {
            headers_.erase(key);
        }

        string rest_client::header(const string &key)
        {
            return headers_[key];
        }

        string rest_client::version() const
        {
            return version_;
        }

        string rest_client::host() const
        {
            return host_;
        }

        int rest_client::response_code() const
        {
            return responseCode_;
        }

        string rest_client::payload() const
        {
            return payload_;
        }

        string rest_client::response() const
        {
            return response_;
        }

        bool rest_client::is_secure() const
        {
            return scheme_ == http::SECURE_SCHEME;
        }

        void rest_client::set_host(const string &host)
        {
            host_ = host;
        }

        void rest_client::set_version(const string &version)
        {
            version_ = version;
        }

        void rest_client::set_secure(bool value)
        {
            scheme_ = value ? http::SECURE_SCHEME : http::SCHEME;
        }

        rest_client &rest_client::set_payload(const string &payload)
        {
            payload_ = payload;
            return *this;
        }

        rest_client &rest_client::request(http::method method, const string &path)
        {
            struct curl_slist *headers = NULL;

            char buf[http::MAX_URL_LEN + 1];

            CURL *curl_ = curl_easy_init();

            if (curl_ == NULL)
            {
                throw rest_exception("unable to initialize request");
            }

            snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s/%s", scheme_.c_str(), host_.c_str(), version_.c_str(), path.c_str());

            curl_easy_setopt(curl_, CURLOPT_URL, buf);

            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_append_response_callback);

            switch (method)
            {
            case http::GET:
                curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
                break;
            case http::POST:
                curl_easy_setopt(curl_, CURLOPT_POST, 1L);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_.size());
                break;
            case http::PUT:
                curl_easy_setopt(curl_, CURLOPT_PUT, 1L);
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());
                curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_.size());
            case http::DELETE:
                curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, http::DELETE);
                break;
            }

            for (auto & h : headers_)
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

                throw rest_exception(curl_easy_strerror(res));
            }

            curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &responseCode_);

            curl_easy_cleanup(curl_);

            return *this;
        }

        rest_client &rest_client::get(const string &path)
        {
            return request(http::GET, path);
        }

        rest_client &rest_client::post(const string &path)
        {
            return request(http::POST, path);
        }

        rest_client &rest_client::put(const string &path)
        {
            return request(http::PUT, path);
        }

        rest_client &rest_client::de1ete(const string &path)
        {
            return request(http::DELETE, path);
        }
    }
}

#endif
