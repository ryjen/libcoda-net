#ifndef ARG3_NET_EXCEPTION_H
#define ARG3_NET_EXCEPTION_H

#include <exception>
#include <string>

namespace arg3
{
    namespace net
    {
        class socket_exception : public std::exception
        {
        public:
            socket_exception ( const std::string &s ) : message_ ( s ) {};
            virtual ~socket_exception() throw() {}
            socket_exception (const socket_exception &e) : std::exception(e), message_(e.message_)
            {}
            socket_exception (socket_exception &&e) : std::exception(std::move(e)), message_(std::move(e.message_))
            {}

            socket_exception &operator=(const socket_exception &e)
            {
                if (this != &e)
                {
                    std::exception::operator=(e);
                    message_ = e.message_;
                }
                return *this;
            }

            socket_exception &operator=(socket_exception && e)
            {
                if (this != &e)
                {
                    std::exception::operator=(std::move(e));
                    message_ = std::move(e.message_);
                }
                return *this;
            }

            virtual const char *what() const throw()
            {
                return message_.c_str();
            }
        private:

            std::string message_;

        };


        class rest_exception : public std::exception
        {
        public:
            rest_exception ( const std::string &s ) : message_ ( s ) {};
            virtual ~rest_exception() throw() {}
            rest_exception (const rest_exception &e) : std::exception(e), message_(e.message_)
            {}
            rest_exception (rest_exception &&e) : std::exception(std::move(e)), message_(std::move(e.message_))
            {}

            rest_exception &operator=(const rest_exception &e)
            {
                if (this != &e)
                {
                    std::exception::operator=(e);
                    message_ = e.message_;
                }
                return *this;
            }

            rest_exception &operator=(rest_exception && e)
            {
                if (this != &e)
                {
                    std::exception::operator=(std::move(e));
                    message_ = std::move(e.message_);
                }
                return *this;
            }

            virtual const char *what() const throw()
            {
                return message_.c_str();
            }
        private:

            std::string message_;

        };
    }
}

#endif

