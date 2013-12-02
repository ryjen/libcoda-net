#ifndef ARG3_NET_EXCEPTION_H
#define ARG3_NET_EXCEPTION_H

#include <exception>
#include <string>

namespace arg3
{
    namespace net
    {
        class SocketException : public std::exception
        {
        public:
            SocketException ( const std::string &s ) : message_ ( s ) {};
            virtual ~SocketException() throw() {}
            SocketException (const SocketException &e) : std::exception(e), message_(e.message_)
            {}
            SocketException (SocketException &&e) : std::exception(std::move(e)), message_(std::move(e.message_))
            {}

            SocketException &operator=(const SocketException &e)
            {
                if (this != &e)
                {
                    std::exception::operator=(e);
                    message_ = e.message_;
                }
                return *this;
            }

            SocketException &operator=(SocketException && e)
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


        class RESTException : public std::exception
        {
        public:
            RESTException ( const std::string &s ) : message_ ( s ) {};
            virtual ~RESTException() throw() {}
            RESTException (const RESTException &e) : std::exception(e), message_(e.message_)
            {}
            RESTException (RESTException &&e) : std::exception(std::move(e)), message_(std::move(e.message_))
            {}

            RESTException &operator=(const RESTException &e)
            {
                if (this != &e)
                {
                    std::exception::operator=(e);
                    message_ = e.message_;
                }
                return *this;
            }

            RESTException &operator=(RESTException && e)
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

