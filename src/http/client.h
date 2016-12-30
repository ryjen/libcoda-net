#ifndef RJ_NET_HTTP_CLIENT_H
#define RJ_NET_HTTP_CLIENT_H

#include <functional>
#include <map>
#include <string>
#include "../uri.h"
#include "protocol.h"

namespace rj
{
    namespace net
    {
        namespace http
        {
            class transfer
            {
               public:
                transfer();
                transfer(const transfer &);
                transfer(transfer &&);
                virtual ~transfer();

                transfer &operator=(const transfer &);
                transfer &operator=(transfer &&other);

                /*!
                 * the post data usually
                 */
                std::string content() const;

                std::string version() const;

                void set_version(const std::string &value);

                /*!
                 * returns the value for an HTTP header for this request
                 */
                std::string header(const std::string &key);

                bool has_header(const std::string &key);

                const std::map<std::string, std::string> headers() const;

               protected:
                std::string content_;
                std::map<std::string, std::string> headers_;
                std::string version_;
            };

            class response : public transfer
            {
               public:
                response();
                response(const std::string &);
                response(const response &);
                response(response &&);
                virtual ~response();

                response &operator=(const response &);
                response &operator=(response &&other);

                std::string full_response() const;

                operator std::string() const;

                /*!
                 * the HTTP response code
                 */
                int code() const;

               private:
                void clear();

                void parse();
                void parse(const std::string &);

                std::string value_;

                int code_;

                friend class client;
            };

            class client : public transfer
            {
               public:
                typedef std::function<void(const response &)> callback;

                typedef std::function<std::string(http::client &, http::method, const std::string &)> implementation;

               public:
                client(const rj::net::uri &uri);
                client(const std::string &uri);
                virtual ~client();
                client(const client &other);
                client(client &&other);
                client &operator=(const client &other);
                client &operator=(client &&other);

                static void set_request_type(const implementation &impl);

                /*!
                 * adds an HTTP header to the request
                 */
                client &add_header(const std::string &key, const std::string &value);

                /*!
                 * adds an HTTP headers to the request
                 */
                client &add_headers(const std::map<std::string, std::string> &values);

                /*!
                 * Removes an HTTP header from the request
                 */
                void remove_header(const std::string &key);

                /*!
                 * returns the host used to connect
                 */
                rj::net::uri uri() const;

                /*!
                 * the response body
                 */
                http::response response() const;

                int timeout() const;

                /*!
                 * @returns true if the request is using HTTPS
                 */
                bool is_secure() const;

                /*!
                 * sets the host for this request
                 */
                client &set_uri(const rj::net::uri &uri);

                /*!
                 * sets the payload for this request
                 */
                client &set_content(const std::string &value);

                /*!
                 * performs a request
                 */
                client &request(http::method method, const std::string &path,
                                const client::callback &callback = nullptr);

                /*!
                 * performs a GET request
                 */
                client &get(const client::callback &callback = nullptr);

                /*!
                 * performs a GET request
                 */
                client &get(const std::string &path, const client::callback &callback = nullptr);

                /*!
                 * performs a POST request
                 */
                client &post(const client::callback &callback = nullptr);

                /*!
                 * performs a POST request
                 */
                client &post(const std::string &path, const client::callback &callback = nullptr);

                /*!
                 * performs a PUT request
                 */
                client &put(const client::callback &callback = nullptr);

                /*!
                 * performs a PUT request
                 */
                client &put(const std::string &path, const client::callback &callback = nullptr);

                /*!
                 * performs a DELETE request
                 */
                client &de1ete(const client::callback &callback = nullptr);

                /*!
                 * performs a DELETE request
                 */
                client &de1ete(const std::string &path, const client::callback &callback = nullptr);

                client &set_timeout(int value);

               private:
                static client::implementation impl_;
                rj::net::uri uri_;
                int timeout_;
                http::response response_;
            };

            namespace socket
            {
                std::string request(http::client &, http::method, const std::string &);
            }

#ifdef CURL_FOUND
            namespace curl
            {
                std::string request(http::client &, http::method, const std::string &);
            }
#endif
        }
    }
}

#endif
