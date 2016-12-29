#ifndef RJ_NET_EXCEPTION_H
#define RJ_NET_EXCEPTION_H

#include <exception>
#include <string>

namespace rj
{
    namespace net
    {
        /*!
         * custom exception
         */
        class socket_exception : public std::exception
        {
           public:
            socket_exception(const std::string &s) : message_(s){};
            virtual ~socket_exception() throw()
            {
            }
            socket_exception(const socket_exception &e) : std::exception(e), message_(e.message_)
            {
            }
            socket_exception(socket_exception &&e) : std::exception(std::move(e)), message_(std::move(e.message_))
            {
            }

            socket_exception &operator=(const socket_exception &e)
            {
                if (this != &e) {
                    std::exception::operator=(e);
                    message_ = e.message_;
                }
                return *this;
            }

            socket_exception &operator=(socket_exception &&e)
            {
                if (this != &e) {
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
