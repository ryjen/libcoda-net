#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <functional>

#include <cinttypes>
#include "exception.h"
#include "http_client.h"
#ifndef HAVE_LIBCURL
#include <cstring>
#include "buffered_socket.h"
#else
#include <cstring>
#endif

using namespace std;

namespace arg3
{
		namespace net
		{
#define THIS_USER_AGENT "libarg3net"

				namespace helper
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

						void curl_set_opt_num(CURL *curl, CURLoption option, long number)
						{
								CURLcode code = curl_easy_setopt(curl, option, number);
								if (code != CURLE_OK) {
										throw rest_exception(curl_easy_strerror(code));
								}
						}
						void curl_set_opt(CURL *curl, CURLoption option, const void *value)
						{
								CURLcode code = curl_easy_setopt(curl, option, value);
								if (code != CURLE_OK) {
										throw rest_exception(curl_easy_strerror(code));
								}
						}
						void curl_set_opt_fun(CURL *curl, CURLoption option, size_t (*value)(void *, size_t, size_t, string *))
						{
								CURLcode code = curl_easy_setopt(curl, option, value);
								if (code != CURLE_OK) {
										throw rest_exception(curl_easy_strerror(code));
								}
						}

#endif
						size_t skip_newline(const string &s, size_t pos)
						{
								for (int i = 0; i < 2; i++, pos++) {
										if (s[pos] != '\r' && s[pos] != '\n') break;
								}

								return pos;
						}
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

						pos = helper::skip_newline(response_, pos);

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

								pos = helper::skip_newline(response_, next);
						}

						if (pos != string::npos) {
								payload_ = response_.substr(pos);
						}
				}


				http_client::http_client(const arg3::net::uri &uri) : uri_(uri), timeout_(http::DEFAULT_HTTP_TIMEOUT)
				{
						add_header(http::HEADER_USER_AGENT, THIS_USER_AGENT);
				}

				http_client::http_client(const std::string &uri) : http_client(arg3::net::uri(uri, http::PROTOCOL))
				{}

				http_client::~http_client()
				{
				}

				http_client::http_client(const http_client &other)
						: http_transfer(other), uri_(other.uri_), response_(other.response_), timeout_(other.timeout_)
				{
				}

				http_client::http_client(http_client &&other)
						: http_transfer(std::move(other)),
							uri_(std::move(other.uri_)),
							response_(std::move(other.response_)),
							timeout_(other.timeout_)
				{
				}

				http_client &http_client::operator=(const http_client &other)
				{
						http_transfer::operator=(other);
						uri_ = other.uri_;
						headers_ = other.headers_;
						timeout_ = other.timeout_;
						return *this;
				}

				http_client &http_client::operator=(http_client &&other)
				{
						http_transfer::operator=(std::move(other));
						uri_ = std::move(other.uri_);
						response_ = std::move(other.response_);
						timeout_ = other.timeout_;
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

				arg3::net::uri http_client::uri() const
				{
						return uri_;
				}

				http_response http_client::response() const
				{
						return response_;
				}

				bool http_client::is_secure() const
				{
						return uri_.scheme() == http::SECURE_PROTOCOL;
				}

				http_client &http_client::set_payload(const string &payload)
				{
						payload_ = payload;
						return *this;
				}

				http_client &http_client::set_timeout(int value)
				{
						timeout_ = value;
						return *this;
				}

#ifdef HAVE_LIBCURL
				void http_client::request_curl(http::method method, const string &path)
				{
						CURLcode code;

						char buf[http::MAX_URL_LEN + 1] = {0};

						struct curl_slist *headers = NULL;

						CURL *curl = NULL;

						curl = curl_easy_init();

						if (curl == NULL) {
								throw rest_exception("unable to initialize request");
						}

						if (path.empty())
								snprintf(buf, http::MAX_URL_LEN, "%s://%s", uri_.scheme().c_str(), uri_.host().c_str());
						else if (path[0] == '/')
								snprintf(buf, http::MAX_URL_LEN, "%s://%s%s", uri_.scheme().c_str(), uri_.host().c_str(), path.c_str());
						else
								snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s", uri_.scheme().c_str(), uri_.host().c_str(), path.c_str());

						helper::curl_set_opt(curl, CURLOPT_URL, buf);

						helper::curl_set_opt_fun(curl, CURLOPT_WRITEFUNCTION, helper::curl_append_response_callback);

						helper::curl_set_opt_num(curl, CURLOPT_HEADER, 1L);

#ifdef DEBUG
						helper::curl_set_opt_num(curl, CURLOPT_VERBOSE, 1L);
#endif

						switch (method) {
								case http::GET:
										helper::curl_set_opt_num(curl, CURLOPT_HTTPGET, 1L);
										break;
								case http::POST:
										helper::curl_set_opt_num(curl, CURLOPT_POST, 1L);
										if (!payload_.empty()) {
												helper::curl_set_opt(curl, CURLOPT_POSTFIELDS, payload_.c_str());
												helper::curl_set_opt_num(curl, CURLOPT_POSTFIELDSIZE, payload_.size());
										}
										break;
								case http::PUT:
										helper::curl_set_opt_num(curl, CURLOPT_PUT, 1L);
										if (!payload_.empty()) {
												helper::curl_set_opt(curl, CURLOPT_POSTFIELDS, payload_.c_str());
												helper::curl_set_opt_num(curl, CURLOPT_POSTFIELDSIZE, payload_.size());
										}
								case http::DELETE:
										helper::curl_set_opt(curl, CURLOPT_CUSTOMREQUEST, http::method_names[http::DELETE]);
										break;
						}

						for (auto &h : headers_) {
								snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());

								headers = curl_slist_append(headers, buf);
						}

						helper::curl_set_opt(curl, CURLOPT_HTTPHEADER, headers);

						helper::curl_set_opt_num(curl, CURLOPT_TIMEOUT, timeout_);

						helper::curl_set_opt(curl, CURLOPT_WRITEDATA, &response_.response_);

						CURLcode res = curl_easy_perform(curl);

						curl_slist_free_all(headers);

						if (res != CURLE_OK) {
								curl_easy_cleanup(curl);

								throw rest_exception(curl_easy_strerror(res));
						}

						curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_.responseCode_);

						curl_easy_cleanup(curl);

						response_.parse();
				}
#else
				void http_client::request_socket(http::method method, const string &path)
				{
						char buf[http::MAX_URL_LEN + 1] = {0};

						buffered_socket sock;

						if (is_secure()) {
								sock.set_secure(true);
						}

						if (!uri_.port().empty()) {
								int port = stoi(uri_.port());

								if (!sock.connect(uri_.host(), port)) {
										throw socket_exception("unable to connect to " + uri_.to_string());
								}
						} else {
								if (!sock.connect(uri_.host(), is_secure() ? http::DEFAULT_SECURE_PORT : http::DEFAULT_PORT)) {
										throw socket_exception("unable to connect to " + uri_.to_string());
								}
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
								snprintf(buf, http::MAX_URL_LEN, "%s: %s", http::HEADER_HOST, uri_.host().c_str());
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

						if (!sock.write_from_buffer()) {
								throw socket_exception("unable to write to socket");
						}

						if (!sock.read_to_buffer()) {
								throw socket_exception("unable to read from socket");
						}

						auto input = sock.input();

						response_.parse(string(input.begin(), input.end()));

						// sock.close();
				}
#endif

				http_client &http_client::request(http::method method, const std::string &path, const http_client_callback &callback)
				{
						response_.clear();

						if (!uri_.is_valid()) {
								throw socket_exception("invalid uri");
						}

#ifdef HAVE_LIBCURL
						request_curl(method, path);
#else
						request_socket(method, path);
#endif

						if (callback) {
								callback(response());
						}
						return *this;
				}

				http_client &http_client::get(const http_client_callback &callback)
				{
						return request(http::GET, uri_.path(), callback);
				}

				http_client &http_client::post(const http_client_callback &callback)
				{
						return request(http::POST, uri_.path(), callback);
				}

				http_client &http_client::put(const http_client_callback &callback)
				{
						return request(http::PUT, uri_.path(), callback);
				}

				http_client &http_client::de1ete(const http_client_callback &callback)
				{
						return request(http::DELETE, uri_.path(), callback);
				}
		}
}
