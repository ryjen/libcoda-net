#include "config.h"

#include <algorithm>
#include <functional>

#include "http_client.h"
#include "exception.h"
#include <cinttypes>
#ifndef HAVE_LIBCURL
#include "buffered_socket.h"
#include <cstring>
#else
#include <cstring>
#endif

namespace arg3
{
    namespace net
    {
#define THIS_USER_AGENT "libarg3net"

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

        size_t skip_newline(const string &s, size_t pos)
        {
            for (int i = 0; i < 2; i++, pos++) {
                if (s[pos] != '\r' && s[pos] != '\n') break;
            }

            return pos;
        }

        http_transfer::http_transfer() : version_(http::VERSION_1_1)
        {
        }

        http_transfer::http_transfer(const http_transfer &other) : payload_(other.payload_), headers_(other.headers_), version_(other.version_)
        {
        }

        http_transfer::http_transfer(http_transfer &&other)
            : payload_(std::move(other.payload_)), headers_(std::move(other.headers_)), version_(std::move(other.version_))
        {
        }

        http_transfer::~http_transfer()
        {
        }

        http_transfer &http_transfer::operator=(const http_transfer &other)
        {
            payload_ = other.payload_;
            headers_ = other.headers_;
            version_ = other.version_;
            return *this;
        }

        http_transfer &http_transfer::operator=(http_transfer &&other)
        {
            payload_ = std::move(other.payload_);
            headers_ = std::move(other.headers_);
            version_ = std::move(other.version_);
            return *this;
        }

        string http_transfer::header(const string &key)
        {
            return headers_[key];
        }

        const map<string, string> http_transfer::headers() const
        {
            return headers_;
        }

        string http_transfer::payload() const
        {
            return payload_;
        }

        string http_transfer::version() const
        {
            return version_;
        }

        void http_transfer::set_version(const string &value)
        {
            version_ = value;
        }

        http_response::http_response() : responseCode_(0)
        {
        }

        http_response::http_response(const string &fullResponse) : response_(fullResponse), responseCode_(0)
        {
            parse();
        }

        http_response::http_response(const http_response &other)
            : http_transfer(other), response_(other.response_), responseCode_(other.responseCode_)
        {
        }

        http_response::http_response(http_response &&other)
            : http_transfer(std::move(other)), response_(std::move(other.response_)), responseCode_(std::move(other.responseCode_))
        {
        }

        http_response::~http_response()
        {
        }

        http_response &http_response::operator=(const http_response &other)
        {
            http_transfer::operator=(other);

            response_ = other.response_;

            responseCode_ = other.responseCode_;

            return *this;
        }

        http_response &http_response::operator=(http_response &&other)
        {
            http_transfer::operator=(std::move(other));

            response_ = std::move(other.response_);

            responseCode_ = std::move(other.responseCode_);

            return *this;
        }

        string http_response::full_response() const
        {
            return response_;
        }

        http_response::operator string() const
        {
            return payload_;
        }

        int http_response::code() const
        {
            return responseCode_;
        }

        void http_response::parse(const string &value)
        {
            response_ = value;

            parse();
        }

        void http_response::clear()
        {
            response_.clear();
            responseCode_ = 0;
            headers_.clear();
            payload_.clear();
        }

        void http_response::parse()
        {
            auto pos = response_.find("\r\n");

            if (pos == string::npos) {
                payload_ = response_;
                return;
            }

            string line = response_.substr(0, pos);

            char buf[http::MAX_URL_LEN + 1] = {0};

            char version[http::MAX_URL_LEN + 1] = {0};

            if (sscanf(line.c_str(), http::RESPONSE_PREAMBLE, version, &responseCode_, buf) != 3) {
                payload_ = response_;
                return;
            }

            version_ = version;

            pos = skip_newline(response_, pos);

            while (!line.empty()) {
                auto next = response_.find("\r\n", pos);

                if (next == string::npos) break;

                line = response_.substr(pos, next - pos);

                auto sep = line.find(':');

                if (sep != string::npos) {
                    auto key = line.substr(0, sep);

                    auto value = line.substr(sep + 2);

                    headers_[key] = value;
                }

                pos = skip_newline(response_, next);
            }

            if (pos != string::npos) payload_ = response_.substr(pos);
        }


        http_client::http_client(const string &host) : scheme_(http::SCHEME), host_(host)
        {
            add_header(http::HEADER_USER_AGENT, THIS_USER_AGENT);
        }

        http_client::http_client()
        {
            add_header(http::HEADER_USER_AGENT, THIS_USER_AGENT);
        }

        http_client::~http_client()
        {
        }

        http_client::http_client(const http_client &other)
            : http_transfer(other), scheme_(other.scheme_), host_(other.host_), response_(other.response_)
        {
        }

        http_client::http_client(http_client &&other)
            : http_transfer(std::move(other)), scheme_(std::move(other.scheme_)), host_(std::move(other.host_)), response_(std::move(other.response_))
        {
        }

        http_client &http_client::operator=(const http_client &other)
        {
            http_transfer::operator=(other);
            scheme_ = other.scheme_;
            host_ = other.host_;
            headers_ = other.headers_;
            return *this;
        }

        http_client &http_client::operator=(http_client &&other)
        {
            http_transfer::operator=(std::move(other));
            scheme_ = std::move(other.scheme_);
            host_ = std::move(other.host_);
            response_ = std::move(other.response_);
            return *this;
        }

        http_client &http_client::add_header(const string &key, const string &value)
        {
            headers_[key] = value;
            return *this;
        }

        void http_client::remove_header(const string &key)
        {
            headers_.erase(key);
        }

        string http_client::host() const
        {
            return host_;
        }

        http_response http_client::response() const
        {
            return response_;
        }

        bool http_client::is_secure() const
        {
            return scheme_ == http::SECURE_SCHEME;
        }

        http_client &http_client::set_host(const string &host)
        {
            host_ = host;
            return *this;
        }

        http_client &http_client::set_secure(bool value)
        {
            scheme_ = value ? http::SECURE_SCHEME : http::SCHEME;
            return *this;
        }

        http_client &http_client::set_payload(const string &payload)
        {
            payload_ = payload;
            return *this;
        }

#ifdef HAVE_LIBCURL
        void http_client::request_curl(http::method method, const string &path)
        {
            char buf[http::MAX_URL_LEN + 1] = {0};

            struct curl_slist *headers = NULL;

            CURL *curl_ = curl_easy_init();

            if (curl_ == NULL) {
                throw rest_exception("unable to initialize request");
            }

            if (path.empty())
                snprintf(buf, http::MAX_URL_LEN, "%s://%s", scheme_.c_str(), host_.c_str());
            else if (path[0] == '/')
                snprintf(buf, http::MAX_URL_LEN, "%s://%s%s", scheme_.c_str(), host_.c_str(), path.c_str());
            else
                snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s", scheme_.c_str(), host_.c_str(), path.c_str());

            curl_easy_setopt(curl_, CURLOPT_URL, buf);

            curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, curl_append_response_callback);

            curl_easy_setopt(curl_, CURLOPT_HEADER, 1L);

#ifdef DEBUG
            curl_easy_setopt(curl_, CURLOPT_VERBOSE, 1L);
#endif

            switch (method) {
                case http::GET:
                    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
                    break;
                case http::POST:
                    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
                    if (!payload_.empty()) {
                        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());
                        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_.size());
                    }
                    break;
                case http::PUT:
                    curl_easy_setopt(curl_, CURLOPT_PUT, 1L);
                    if (!payload_.empty()) {
                        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, payload_.c_str());
                        curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, payload_.size());
                    }
                case http::DELETE:
                    curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, http::method_names[http::DELETE]);
                    break;
            }

            for (auto &h : headers_) {
                snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());

                headers = curl_slist_append(headers, buf);
            }

            curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

            curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response_.response_);

            CURLcode res = curl_easy_perform(curl_);

            curl_slist_free_all(headers);

            if (res != CURLE_OK) {
                curl_easy_cleanup(curl_);

                throw rest_exception(curl_easy_strerror(res));
            }

            curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_.responseCode_);

            curl_easy_cleanup(curl_);

            response_.parse();
        }
#else
        void http_client::request_socket(http::method method, const string &path)
        {
            char buf[http::MAX_URL_LEN + 1] = {0};

            buffered_socket sock;

            // create the socket based on hostname and port
            auto pos = host().find(':');

            if (is_secure()) sock.set_secure(true);

            if (pos != string::npos) {
                string hostname = host().substr(0, pos);
                int port = stoi(host().substr(pos + 1));

                if (!sock.connect(hostname, port)) throw socket_exception("unable to connect to " + host());
            } else {
                if (!sock.connect(host(), is_secure() ? http::DEFAULT_SECURE_PORT : http::DEFAULT_PORT))
                    throw socket_exception("unable to connect to " + host());
            }

            // send the method and path

            if (path.empty())
                snprintf(buf, http::MAX_URL_LEN, http::REQUEST_PREAMBLE, http::method_names[method], "/", version_.c_str());
            else if (path[0] == '/')
                snprintf(buf, http::MAX_URL_LEN, http::REQUEST_PREAMBLE, http::method_names[method], path.c_str(), version_.c_str());
            else
                snprintf(buf, http::MAX_URL_LEN, http::REQUEST_PREAMBLE, http::method_names[method], ("/" + path).c_str(), version_.c_str());

            sock.writeln(buf);

            bool chunked =
                headers_.count(http::HEADER_TRANSFER_ENCODING) != 0 && !strcasecmp(headers_[http::HEADER_TRANSFER_ENCODING].c_str(), "chunked");

            // specify the host
            if (headers_.count(http::HEADER_HOST) == 0) {
                snprintf(buf, http::MAX_URL_LEN, "%s: %s", http::HEADER_HOST, host().c_str());
                sock.writeln(buf);
            }

            if (headers_.count(http::HEADER_ACCEPT) == 0) {
                snprintf(buf, http::MAX_URL_LEN, "%s: */*", http::HEADER_ACCEPT);
                sock.writeln(buf);
            }

            if (headers_.count(http::HEADER_CONNECTION) == 0) {
                snprintf(buf, http::MAX_URL_LEN, "%s: close", http::HEADER_CONNECTION);
                sock.writeln(buf);
            }

            // add the headers
            for (const auto &h : headers_) {
                snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());

                sock.writeln(buf);
            }

            // if we have a payload, add the size
            if (!chunked && !payload_.empty()) {
                snprintf(buf, http::MAX_URL_LEN, "%s: %zu", http::HEADER_CONTENT_SIZE, payload_.size());

                sock.writeln(buf);
            }

            // finish header
            sock.writeln();

            // add the payload
            if (!payload_.empty()) {
                sock.write(payload_);
            }

#ifdef DEBUG
            cout << string(sock.output().begin(), sock.output().end());
#endif

            if (!sock.write_from_buffer()) throw socket_exception("unable to write to socket");

            if (!sock.read_to_buffer()) throw socket_exception("unable to read from socket");

            auto input = sock.input();

            response_.parse(string(input.begin(), input.end()));

            // sock.close();
        }
#endif

        http_client &http_client::request(http::method method, const string &path)
        {
            response_.clear();

            if (host_.empty()) throw socket_exception("no host");

#ifdef HAVE_LIBCURL
            request_curl(method, path);
#else
            request_socket(method, path);
#endif

            return *this;
        }

        http_client &http_client::get(const string &path)
        {
            return request(http::GET, path);
        }

        http_client &http_client::post(const string &path)
        {
            return request(http::POST, path);
        }

        http_client &http_client::put(const string &path)
        {
            return request(http::PUT, path);
        }

        http_client &http_client::de1ete(const string &path)
        {
            return request(http::DELETE, path);
        }
    }
}
