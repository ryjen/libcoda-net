#ifndef CODA_NET_EXCEPTION_H
#define CODA_NET_EXCEPTION_H

#include <exception>
#include <string>

namespace coda
{
    namespace net
    {
        /*!
         * custom exception
         */
        class socket_exception : public std::exception
        {
           public:
            socket_exception(const std::string &s) noexcept : message_(s){};
            virtual ~socket_exception() noexcept
            {
            }
            socket_exception(const socket_exception &e) noexcept : std::exception(e), message_(e.message_)
            {
            }
            socket_exception(socket_exception &&e) noexcept : std::exception(std::move(e)), message_(std::move(e.message_))
            {
            }

            socket_exception &operator=(const socket_exception &e) noexcept
            {
                if (this != &e) {
                    std::exception::operator=(e);
                    message_ = e.message_;
                }
                return *this;
            }

            socket_exception &operator=(socket_exception &&e) noexcept
            {
                if (this != &e) {
                    std::exception::operator=(std::move(e));
                    message_ = std::move(e.message_);
                }
                return *this;
            }

            virtual const char *what() const noexcept
            {
                return message_.c_str();
            }

           private:
            std::string message_;
        };
    }
}

#endif
