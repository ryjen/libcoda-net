
// Definition of the Socket class

#ifndef _ARG3_SOCKET_H_
#define _ARG3_SOCKET_H_

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include <iostream>
#include <sstream>
#include <string>
#include <exception>

namespace arg3
{
    namespace net
    {

#ifdef _WIN32
        typedef int socklen_t;
#else
        typedef int SOCKET;
        int closesocket(SOCKET socket);
#endif

        class Socket
        {
        public:
            Socket(const Socket &);
            virtual ~Socket();
            Socket &operator=(const Socket &);

            // Data Transimission
            int send ( const std::string&, int flags = 0 );
            int send ( unsigned const char *, size_t, int flags = 0);

            // Recieves a block of input
            int recv(std::string &);

            bool is_valid() const;

            const Socket& operator << ( const std::string& );
            const Socket& operator >> ( std::string& );

            SOCKET getSocket() const;

            const char *getIP() const;

            int getPort() const;

            void setPort(const int port);
            void setIP(const std::string &ip);
            //void setSocket(SOCKET sock);
            void close();

        protected:

            static const int MAXHOSTNAME = 200;
            static const int MAXRECV = 500;
            static const int INVALID = -1;

            Socket(SOCKET sock, const sockaddr_in &addr);

            Socket();

            void set_non_blocking ( const bool );

            SOCKET sock_;
            sockaddr_in addr_;

            friend class ServerSocket;
        };

    }
}

#endif

