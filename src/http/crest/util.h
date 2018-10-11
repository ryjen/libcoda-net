#ifndef CODA_NET_JEST_UTIL_H
#define CODA_NET_JEST_UTIL_H

#include <functional>
#include <memory>

#ifdef WIN32
#define PATH_SEP "\\"
#else
#define PATH_SEP "/"
#endif

namespace coda
{
    namespace net
    {
        class jester;

        namespace jest
        {
            class interactive
            {
               public:
                interactive(jester *jest);

                void start();

               private:
                void ask_method() const;

                bool get_method();

                void ask_headers() const;

                bool get_headers();

                void ask_data() const;

                bool get_data();

                void prompt() const;

                std::function<bool()> state_;
                std::function<void()> prompt_;
                jester *jester_;
            };

            typedef std::pair<std::string, std::string> arg_pair;

            std::shared_ptr<arg_pair> split_arg(const std::string &arg, const std::string &delimiter);
        }
    }
}

#endif
