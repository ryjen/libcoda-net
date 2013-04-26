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

        RESTClient::RESTClient(const string &host, const string &version) : protocol_(http::PROTOCOL), host_(host), version_(version), curl_(curl_easy_init())
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

        string RESTClient::getVersion() const {
            return version_;
        }

        string RESTClient::getHost() const {
            return host_;
        }

        int RESTClient::getResult() const {
            return result_;
        }

        void RESTClient::setSecure(bool value) {
            protocol_ = value ? http::SECURE_PROTOCOL : http::PROTOCOL;
        }

        string RESTClient::perform_request(const string& path)
        {
            struct curl_slist *headers = NULL;

            string output;

            char buf[http::MAX_URL_LEN+1];

            snprintf(buf, sizeof(buf), "%s://%s/%s/%s", protocol_.c_str(), host_.c_str(), version_.c_str(), path.c_str());

            curl_easy_setopt(curl_, CURLOPT_URL, buf);

            for(auto &h : headers_)
            {
                sprintf(buf, "%s: %s", h.first.c_str(), h.second.c_str());

                headers = curl_slist_append(headers, buf);
            }

            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &output);

            CURLcode res = curl_easy_perform(curl_);

            if (res != CURLE_OK)
                throw RESTException(curl_easy_strerror(res));

            curl_easy_getinfo (curl_, CURLINFO_RESPONSE_CODE, &result_);

            /* always cleanup */
            curl_easy_cleanup(curl_);

            return output;
        }

        string RESTClient::get(const string &path)
        {
            curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);

            return perform_request(path);
        }

        string RESTClient::post(const string &path, const string &payload)
        {
            curl_easy_setopt(curl_, CURLOPT_POST, 1L);
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload.size());

            return perform_request(path);
        }

        string RESTClient::put(const string &path, const string &payload)
        {
            curl_easy_setopt(curl_, CURLOPT_PUT, 1L);
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload.c_str());
            curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload.size());

            return perform_request(path);
        }

        string RESTClient::de1ete(const string &path)
        {
            curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, http::DELETE);

            return perform_request(path);
        }
    }
}