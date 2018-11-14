
#include "uri.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>

using namespace std;

namespace coda {
  namespace net {
    namespace helper {
#ifdef URIPARSER_FOUND
      static std::string fromRange(const UriTextRangeA &rng) {
        if (rng.first && rng.afterLast) {
          return std::string(rng.first, rng.afterLast);
        } else {
          return std::string();
        }
      }

      static std::string fromList(UriPathSegmentA *xs,
                                  const std::string &delim) {
        UriPathSegmentStructA *head(xs);
        std::string accum;

        while (head) {
          accum += delim + fromRange(head->text);
          head = head->next;
        }

        return accum;
      }
#endif
      char from_hex(char ch) {
        return isdigit(ch) ? ch - '0' : tolower(ch) - 'a' + 10;
      }
    } // namespace helper
    uri::uri() noexcept : isValid_(false) {}

    uri::uri(const std::string &uri, const std::string &defaultScheme)
        : uri_(uri) {
      isValid_ = parse(uri, defaultScheme);
    }

    std::string uri::to_string() const noexcept { return uri_; }

    uri::operator std::string() const noexcept { return uri_; }

    std::string uri::full_path() const {
      std::string temp = path();
      if (!query().empty()) {
        temp += '?';
        temp += query_;
      }
      if (!fragment().empty()) {
        temp += '#';
        temp += fragment();
      }
      return temp;
    }
    bool uri::parse(const std::string &uri_s,
                    const std::string &defaultScheme) {
      static const string prot_end("://");

      if (uri_s.empty()) {
        return false;
      }

      string::const_iterator pos_i =
          search(uri_s.begin(), uri_s.end(), prot_end.begin(), prot_end.end());
#ifdef URIPARSER_FOUND
      UriUriA uri;
      UriParserStateA state;
      state.uri = &uri;
      std::string url;

      // ensure we always have a scheme
      if (pos_i == uri_s.end()) {
        url.append(defaultScheme).append(prot_end).append(uri_s);
      } else {
        url = uri_s;
      }

      bool rval = uriParseUriA(&state, url.c_str()) == URI_SUCCESS;

      if (rval) {
        scheme_ = helper::fromRange(uri.scheme);
        auto userInfo = helper::fromRange(uri.userInfo);
        if (!userInfo.empty()) {
          user_ = userInfo.substr(0, userInfo.find(':'));
          password_ = userInfo.substr(userInfo.find(':') + 1);
        }
        host_ = helper::fromRange(uri.hostText);
        port_ = helper::fromRange(uri.portText);
        path_ = helper::fromList(uri.pathHead, "/");
        query_ = helper::fromRange(uri.query);
        fragment_ = helper::fromRange(uri.fragment);
      }

      uriFreeUriMembersA(&uri);

      return rval;
#else
      // do the manual implementation from stack overflow
      // with some mods for the port
      if (pos_i == uri_s.end()) {
        if (defaultScheme.empty()) {
          return false;
        } else {
          scheme_ = defaultScheme;
          pos_i = uri_s.begin();
        }
      } else {
        scheme_.reserve(distance(uri_s.begin(), pos_i));
        transform(uri_s.begin(), pos_i, back_inserter(scheme_),
                  ptr_fun<int, int>(tolower)); // protocol is icase
        advance(pos_i, prot_end.length());
      }

      string::const_iterator user_i = find(pos_i, uri_s.end(), '@');
      string::const_iterator path_i;

      if (user_i != uri_s.end()) {
        string::const_iterator pwd_i = find(pos_i, user_i, ':');

        if (pwd_i != user_i) {
          password_.assign(pwd_i, user_i);
          user_.assign(pos_i, pwd_i);
        } else {
          user_.assign(pos_i, user_i);
        }

        pos_i = user_i + 1;
      }

      path_i = find(pos_i, uri_s.end(), '/');

      string::const_iterator port_i = find(pos_i, path_i, ':');
      string::const_iterator host_end;
      if (port_i != uri_s.end()) {
        port_.assign(*port_i == ':' ? (port_i + 1) : port_i, path_i);
        host_end = port_i;
      } else {
        host_end = path_i;
      }
      host_.reserve(distance(pos_i, host_end));
      transform(pos_i, host_end, back_inserter(host_),
                ptr_fun<int, int>(tolower));
      string::const_iterator query_i = find(path_i, uri_s.end(), '?');
      path_.assign(*path_i == '/' ? (path_i + 1) : path_i, query_i);
      if (query_i != uri_s.end())
        ++query_i;
      string::const_iterator frag_i = find(query_i, uri_s.end(), '#');
      query_.assign(query_i, frag_i);
      if (frag_i != uri_s.end()) {
        fragment_.assign(frag_i, uri_s.end());
      }
      return true;
#endif
    }

    uri::~uri() noexcept {}

    bool uri::is_valid() const noexcept { return isValid_; }

    std::string uri::scheme() const noexcept { return scheme_; }

    std::string uri::username() const noexcept { return user_; }

    std::string uri::password() const noexcept { return password_; }

    std::string uri::host() const noexcept { return host_; }
    std::string uri::port() const noexcept { return port_; }

    std::string uri::host_with_port() const {
      std::string hostname = host();

      if (!port().empty()) {
        return hostname + ":" + port();
      }
      return hostname;
    }

    std::string uri::path() const noexcept { return path_; }
    std::string uri::query() const noexcept { return query_; }
    std::string uri::fragment() const noexcept { return fragment_; }

    std::string uri::encode(const std::string &value) {
      // TODO: test. Grabbed from stack overflow
      ostringstream escaped;
      escaped.fill('0');
      escaped << hex;

      for (string::const_iterator i = value.begin(), n = value.end(); i != n;
           ++i) {
        string::value_type c = (*i);

        // Keep alphanumeric and other accepted characters intact
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
          escaped << c;
          continue;
        }

        // Any other characters are percent-encoded
        escaped << uppercase;
        escaped << '%' << setw(2) << int((unsigned char)c);
        escaped << nouppercase;
      }

      return escaped.str();
    }

    std::string uri::decode(const std::string &value) {
      // TODO: test. grabbed from stack overflow
      char h;
      ostringstream escaped;
      escaped.fill('0');

      for (auto i = value.begin(), n = value.end(); i != n; ++i) {
        string::value_type c = (*i);

        if (c == '%') {
          if (i[1] && i[2]) {
            h = helper::from_hex(i[1]) << 4 | helper::from_hex(i[2]);
            escaped << h;
            i += 2;
          }
        } else if (c == '+') {
          escaped << ' ';
        } else {
          escaped << c;
        }
      }

      return escaped.str();
    }
  } // namespace net
} // namespace coda
