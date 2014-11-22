#ifndef _ARG3_NET_URI_H_
#define _ARG3_NET_URI_H_

#include "config.h"

#ifdef HAVE_LIBURIPARSER
#include <uriparser/Uri.h>
#endif

#include <string>

namespace arg3
{
    namespace net
    {
        class uri
        {
        public:
            uri(std::string uri);

            ~uri();

            bool is_valid() const;

            std::string scheme()   const;
            std::string host()     const;
            std::string port()     const;
            std::string path()     const;
            std::string query()    const;
            std::string fragment() const;

        private:
            std::string uri_;
            bool        isValid_;
#ifdef HAVE_LIBURIPARSER
            UriUriA     uriParse_;

            std::string fromRange(const UriTextRangeA &rng) const;

            std::string fromList(UriPathSegmentA *xs, const std::string &delim) const;
#endif
        };
    }
}

#endif
