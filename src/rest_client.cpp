#include "config.h"

#include "rest_client.h"
#include "exception.h"

#ifndef HAVE_LIBCURL
#include "buffered_socket.h"
#endif

namespace arg3
{
    namespace net
    {

#ifdef HAVE_LIBCURL

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
#endif

        rest_client::rest_client(const string &host) : scheme_(http::SCHEME), host_(host)
        {
            add_header("User-Agent", "libarg3net");
        }

        rest_client::rest_client()
        {
            add_header("User-Agent", "libarg3net");
        }

        rest_client::~rest_client()
        {
        }

        rest_client::rest_client(const rest_client &other) : scheme_(other.scheme_), host_(other.host_),
            payload_(other.payload_), responseCode_(other.responseCode_),
            response_(other.response_), headers_(other.headers_)
        {
        }

        rest_client::rest_client(rest_client &&other) : scheme_(std::move(other.scheme_)), host_(std::move(other.host_)),
            payload_(std::move(other.payload_)), responseCode_(other.responseCode_),
            response_(std::move(other.response_)), headers_(std::move(other.headers_))
        {
        }

        rest_client &rest_client::operator=(const rest_client &other)
        {
            scheme_ = other.scheme_;
            host_ = other.host_;
            payload_ = other.payload_;
            responseCode_ = other.responseCode_;
            response_ = other.response_;
            headers_ = other.headers_;
            return *this;
        }

        rest_client &rest_client::operator=(rest_client && other)
        {
            scheme_ = std::move(other.scheme_);
            host_ = std::move(other.host_);
            payload_ = std::move(other.payload_);
            responseCode_ = std::move(other.responseCode_);
            response_ = std::move(other.response_);
            headers_ = std::move(other.headers_);
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
            char buf[http::MAX_URL_LEN + 1];

#ifdef HAVE_LIBCURL
            struct curl_slist *headers = NULL;

            CURL *curl_ = curl_easy_init();

            if (curl_ == NULL)
            {
                throw rest_exception("unable to initialize request");
            }

            snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s", scheme_.c_str(), host_.c_str(), path.c_str());

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
                curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, http::method_names[http::DELETE]);
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
#else
            buffered_socket sock;

            // create the socket based on hostname and port
            auto pos = host().find(':');

            if (pos != string::npos)
            {
                string hostname = host().substr(0, pos);
                int port = stoi(host().substr(pos + 1));
                sock.connect(hostname, port);
            }
            else
            {
                sock.connect(host(), is_secure() ? http::DEFAULT_SECURE_PORT : http::DEFAULT_PORT);
            }

            if (is_secure())
                sock.set_secure(true);

            // send the method and path
            snprintf(buf, http::MAX_URL_LEN, http::REQUEST_HEADER, http::method_names[method], path.c_str());

            sock.writeln(buf);

            // specify the host
            snprintf(buf, http::MAX_URL_LEN, "Host: %s", host().c_str());

            sock.writeln(buf);

            // add the headers
            for (auto & h : headers_)
            {
                snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());

                sock.writeln(buf);
            }

            // if we have a payload, add the size
            if (!payload_.empty())
            {
                snprintf(buf, http::MAX_URL_LEN, "Content-Size: %ld", payload_.size());

                sock.writeln(buf);
            }

            // finish header
            sock.writeln();

            // add the payload
            if (payload_.empty())
            {
                sock.writeln(payload_);
            }

            sock.write_from_buffer();

            sock.read_to_buffer();

            string line = sock.readln();

            if (sscanf(line.c_str(), http::RESPONSE_HEADER, &responseCode_, buf) == 2)
            {
                // parse out the rest of the header
                while (!line.empty())
                {
                    line = sock.readln();
                    response_ += line;
                }
                response_.clear();
            }
            else
            {
                // could be plain text
                response_ = line;
            }

            auto input = sock.input();

            if (!input.empty())
                response_.append(input.begin(), input.end());

            sock.close();
#endif
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

