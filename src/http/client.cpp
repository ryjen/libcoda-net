
#include <algorithm>
#include <functional>

#include "../exception.h"
#include "../socket.h"
#include "../uri.h"
#include "client.h"
#include <cinttypes>

using namespace std;

#define THIS_USER_AGENT "libcoda_net"

namespace coda {
  namespace net {
    namespace http {
      namespace helper {
        size_t skip_newline(const string &s, size_t pos) {
          for (int i = 0; i < 2; i++, pos++) {
            if (s[pos] != '\r' && s[pos] != '\n')
              break;
          }

          return pos;
        }
      } // namespace helper

      transfer::transfer() : version_(http::VERSION_1_1) {}

      transfer::transfer(const transfer &other)
          : content_(other.content_), headers_(other.headers_),
            version_(other.version_) {}

      transfer::transfer(transfer &&other)
          : content_(std::move(other.content_)),
            headers_(std::move(other.headers_)),
            version_(std::move(other.version_)) {}

      transfer::~transfer() {}

      transfer &transfer::operator=(const transfer &other) {
        content_ = other.content_;
        headers_ = other.headers_;
        version_ = other.version_;
        return *this;
      }

      transfer &transfer::operator=(transfer &&other) {
        content_ = std::move(other.content_);
        headers_ = std::move(other.headers_);
        version_ = std::move(other.version_);
        return *this;
      }

      string transfer::header(const string &key) { return headers_[key]; }

      bool transfer::has_header(const string &key) {
        return headers_.count(key);
      }

      const map<string, string> transfer::headers() const { return headers_; }

      string transfer::content() const { return content_; }

      string transfer::version() const { return version_; }

      void transfer::set_version(const string &value) { version_ = value; }

      response::response() : code_(0) {}

      response::response(const string &fullResponse)
          : value_(fullResponse), code_(0) {
        parse();
      }

      response::response(const response &other)
          : transfer(other), value_(other.value_), code_(other.code_) {}

      response::response(response &&other)
          : transfer(std::move(other)), value_(std::move(other.value_)),
            code_(std::move(other.code_)) {}

      response::~response() {}

      response &response::operator=(const response &other) {
        transfer::operator=(other);

        value_ = other.value_;

        code_ = other.code_;

        return *this;
      }

      response &response::operator=(response &&other) {
        transfer::operator=(std::move(other));

        value_ = std::move(other.value_);

        code_ = std::move(other.code_);

        return *this;
      }

      string response::full_response() const { return value_; }

      response::operator string() const { return content_; }

      int response::code() const { return code_; }

      void response::parse(const string &value) {
        clear();

        value_ = value;

        parse();
      }

      void response::clear() {
        value_.clear();
        code_ = 0;
        headers_.clear();
        content_.clear();
      }

      void response::parse() {
        auto pos = value_.find("\r\n");

        if (pos == string::npos) {
          content_ = value_;
          return;
        }

        string line = value_.substr(0, pos);

        char buf[http::MAX_URL_LEN + 1] = {0};

        char version[http::MAX_URL_LEN + 1] = {0};

        if (sscanf(line.c_str(), http::RESPONSE_PREAMBLE, version, &code_,
                   buf) != 3) {
          content_ = value_;
          return;
        }

        version_ = version;

        pos = helper::skip_newline(value_, pos);

        while (!line.empty()) {
          auto next = value_.find("\r\n", pos);

          if (next == string::npos)
            break;

          line = value_.substr(pos, next - pos);

          auto sep = line.find(':');

          if (sep != string::npos) {
            auto key = line.substr(0, sep);

            auto value = line.substr(sep + 2);

            headers_[key] = value;
          }

          pos = helper::skip_newline(value_, next);
        }

        if (pos != string::npos) {
          content_ = value_.substr(pos);
        }
      }

      client::client(const coda::net::uri &uri)
          : uri_(uri), timeout_(http::DEFAULT_HTTP_TIMEOUT) {
        add_header(http::HEADER_USER_AGENT, THIS_USER_AGENT);
      }

      client::client(const std::string &uri)
          : client(coda::net::uri(uri, http::PROTOCOL)) {}

      client::~client() {}

      client::client(const client &other)
          : transfer(other), uri_(other.uri_), response_(other.response_),
            timeout_(other.timeout_) {}

      client::client(client &&other)
          : transfer(std::move(other)), uri_(std::move(other.uri_)),
            response_(std::move(other.response_)), timeout_(other.timeout_) {}

      client &client::operator=(const client &other) {
        transfer::operator=(other);
        uri_ = other.uri_;
        headers_ = other.headers_;
        timeout_ = other.timeout_;
        return *this;
      }

      client &client::operator=(client &&other) {
        transfer::operator=(std::move(other));
        uri_ = std::move(other.uri_);
        response_ = std::move(other.response_);
        timeout_ = other.timeout_;
        return *this;
      }

      client &client::add_header(const string &key, const string &value) {
        headers_[key] = value;
        return *this;
      }

      client &
      client::add_headers(const std::map<std::string, std::string> &value) {
        headers_.insert(value.begin(), value.end());
        return *this;
      }

      void client::remove_header(const string &key) { headers_.erase(key); }

      coda::net::uri client::uri() const { return uri_; }

      response client::response() const { return response_; }

      int client::timeout() const { return timeout_; }

      bool client::is_secure() const {
        return uri_.scheme() == http::SECURE_PROTOCOL;
      }

      client &client::set_content(const string &content) {
        content_ = content;
        return *this;
      }

      client &client::set_timeout(int value) {
        timeout_ = value;
        return *this;
      }

      client &client::request(http::method method, const std::string &path,
                              const client::callback &callback) {
        if (!impl_) {
          throw socket_exception("invalid implementation");
        }

        if (!uri_.is_valid()) {
          throw socket_exception("invalid uri");
        }

        auto content = impl_(*this, method, path);

        response_.parse(content);

        if (callback) {
          callback(response_);
        }
        return *this;
      }

      client &client::get(const client::callback &callback) {
        return request(http::GET, uri_.path(), callback);
      }
      client &client::get(const std::string &path,
                          const client::callback &callback) {
        return request(http::GET, uri_.path() + "/" + path, callback);
      }
      client &client::post(const client::callback &callback) {
        return request(http::POST, uri_.path(), callback);
      }
      client &client::post(const std::string &path,
                           const client::callback &callback) {
        return request(http::POST, uri_.path() + "/" + path, callback);
      }
      client &client::put(const client::callback &callback) {
        return request(http::PUT, uri_.path(), callback);
      }
      client &client::put(const std::string &path,
                          const client::callback &callback) {
        return request(http::PUT, uri_.path() + "/" + path, callback);
      }
      client &client::de1ete(const client::callback &callback) {
        return request(http::DELETE, uri_.path(), callback);
      }
      client &client::de1ete(const std::string &path,
                             const client::callback &callback) {
        return request(http::DELETE, uri_.path() + "/" + path, callback);
      }

      void client::set_request_type(const implementation &impl) {
        impl_ = impl;
      }
    } // namespace http
  }   // namespace net
} // namespace coda
