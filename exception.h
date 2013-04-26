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

            virtual const char *what() const throw()
            {
                return message_.c_str();
            }
        private:

            std::string message_;

        };
    }
}
