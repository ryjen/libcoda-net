#include "../socket.h"
#include "../exception.h"
#include "../uri.h"
#include "client.h"

#ifdef CURL_FOUND

#include <curl/curl.h>
#include <cstring>
#endif

using namespace std;

namespace rj
{
    namespace net
    {
        namespace http
        {
            namespace curl
            {
#ifdef CURL_FOUND
                namespace helper
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

                    void curl_set_opt_num(CURL *curl, CURLoption option, long number)
                    {
                        CURLcode code = curl_easy_setopt(curl, option, number);
                        if (code != CURLE_OK) {
                            throw socket_exception(curl_easy_strerror(code));
                        }
                    }
                    void curl_set_opt(CURL *curl, CURLoption option, const void *value)
                    {
                        CURLcode code = curl_easy_setopt(curl, option, value);
                        if (code != CURLE_OK) {
                            throw socket_exception(curl_easy_strerror(code));
                        }
                    }
                    void curl_set_opt_fun(CURL *curl, CURLoption option,
                                          size_t (*value)(void *, size_t, size_t, string *))
                    {
                        CURLcode code = curl_easy_setopt(curl, option, value);
                        if (code != CURLE_OK) {
                            throw socket_exception(curl_easy_strerror(code));
                        }
                    }
                }

                std::string request(http::client &client, http::method method, const std::string &userPath)
                {
                    CURLcode code;

                    char buf[http::MAX_URL_LEN + 1] = {0};

                    struct curl_slist *headers = NULL;

                    CURL *curl = NULL;

                    std::string path = userPath;

                    net::uri uri = client.uri();

                    std::string content = client.content();

                    std::string response;

                    curl = curl_easy_init();

                    if (curl == NULL) {
                        throw socket_exception("unable to initialize curl request");
                    }

                    // check if a path was specified
                    if (path.empty()) {
                        path = uri.full_path();
                    }

                    if (path.empty()) {
                        snprintf(buf, http::MAX_URL_LEN, "%s://%s", uri.scheme().c_str(), uri.host_with_port().c_str());
                    } else if (path[0] == '/') {
                        snprintf(buf, http::MAX_URL_LEN, "%s://%s%s", uri.scheme().c_str(),
                                 uri.host_with_port().c_str(), path.c_str());
                    } else {
                        snprintf(buf, http::MAX_URL_LEN, "%s://%s/%s", uri.scheme().c_str(),
                                 uri.host_with_port().c_str(), path.c_str());
                    }

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
                            if (!content.empty()) {
                                helper::curl_set_opt(curl, CURLOPT_POSTFIELDS, content.c_str());
                                helper::curl_set_opt_num(curl, CURLOPT_POSTFIELDSIZE, content.size());
                            }
                            break;
                        case http::PUT:
                            helper::curl_set_opt_num(curl, CURLOPT_PUT, 1L);
                            if (!content.empty()) {
                                helper::curl_set_opt(curl, CURLOPT_POSTFIELDS, content.c_str());
                                helper::curl_set_opt_num(curl, CURLOPT_POSTFIELDSIZE, content.size());
                            }
                            break;
                        default:
                            helper::curl_set_opt(curl, CURLOPT_CUSTOMREQUEST, http::method_names[method]);
                            break;
                    }

                    for (auto &h : client.headers()) {
                        snprintf(buf, http::MAX_URL_LEN, "%s: %s", h.first.c_str(), h.second.c_str());
                        headers = curl_slist_append(headers, buf);
                    }

                    helper::curl_set_opt(curl, CURLOPT_HTTPHEADER, headers);

                    helper::curl_set_opt_num(curl, CURLOPT_TIMEOUT, client.timeout());

                    helper::curl_set_opt(curl, CURLOPT_WRITEDATA, &response);

                    CURLcode res = curl_easy_perform(curl);

                    curl_slist_free_all(headers);

                    if (res != CURLE_OK && res != CURLE_PARTIAL_FILE) {
                        curl_easy_cleanup(curl);

                        throw socket_exception(curl_easy_strerror(res));
                    }

                    curl_easy_cleanup(curl);

                    return response;
                }
#else

                std::string request(http::client &client, http::method method, const std::string &userPath)
                {
                    throw socket_exception("curl implementation not enabled");
                }
#endif
            }
        }
    }
}
