#include <strings.h>
#include "../buffered_socket.h"
#include "../exception.h"
#include "client.h"

using namespace std;

namespace rj
{
    namespace net
    {
        namespace http
        {
            namespace socket
            {
                std::string request(http::client &client, http::method method, const string &userPath)
                {
                    char buf[http::MAX_URL_LEN + 1] = {0};

                    buffered_socket sock;

                    std::string path = userPath;

                    net::uri uri = client.uri();

                    std::string content = client.content();

                    if (client.is_secure()) {
                        sock.set_secure(true);
                    }

                    if (!uri.port().empty()) {
                        int port = stoi(uri.port());

                        if (!sock.connect(uri.host(), port)) {
                            throw socket_exception("unable to connect to " + uri.to_string());
                        }
                    } else {
                        if (!sock.connect(uri.host(),
                                          client.is_secure() ? http::DEFAULT_SECURE_PORT : http::DEFAULT_PORT)) {
                            throw socket_exception("unable to connect to " + uri.to_string());
                        }
                    }

                    if (path.empty()) {
                        path = uri.full_path();
                    }

                    // send the method and path

                    if (path.empty())
                        snprintf(buf, http::MAX_URL_LEN, http::REQUEST_PREAMBLE, http::method_names[method], "/",
                                 client.version().c_str());
                    else if (path[0] == '/')
                        snprintf(buf, http::MAX_URL_LEN, http::REQUEST_PREAMBLE, http::method_names[method],
                                 path.c_str(), client.version().c_str());
                    else
                        snprintf(buf, http::MAX_URL_LEN, http::REQUEST_PREAMBLE, http::method_names[method],
                                 ("/" + path).c_str(), client.version().c_str());

                    sock.writeln(buf);

                    bool chunked = client.has_header(http::HEADER_TRANSFER_ENCODING) &&
                                   !strcasecmp(client.header(http::HEADER_TRANSFER_ENCODING).c_str(), "chunked");

                    // specify the host
                    if (!client.has_header(http::HEADER_HOST)) {
                        snprintf(buf, http::MAX_URL_LEN, "%s: %s", http::HEADER_HOST, uri.host().c_str());
                        sock.writeln(buf);
                    }

                    if (!client.has_header(http::HEADER_ACCEPT)) {
                        snprintf(buf, http::MAX_URL_LEN, "%s: */*", http::HEADER_ACCEPT);
                        sock.writeln(buf);
                    }

                    if (!client.has_header(http::HEADER_CONNECTION)) {
                        snprintf(buf, http::MAX_URL_LEN, "%s: close", http::HEADER_CONNECTION);
                        sock.writeln(buf);
                    }

                    // add the headers
                    for (const auto &h : client.headers()) {
                        snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());
                        sock.writeln(buf);
                    }

                    // if we have a content, add the size
                    if (!chunked && !content.empty()) {
                        snprintf(buf, http::MAX_URL_LEN, "%s: %zu", http::HEADER_CONTENT_SIZE, content.size());
                        sock.writeln(buf);
                    }

                    // finish header
                    sock.writeln();

                    // add the content
                    if (!content.empty()) {
                        sock.write(content);
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

                    return string(input.begin(), input.end());
                }
            }

            client::implementation client::impl_ = socket::request;
        }
    }
}