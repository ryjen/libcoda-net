
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

            Socket(SOCKET sock, const sockaddr_in &addr);

            Socket(const std::string &host, const int port);
            Socket (int port, int queueSize = BACKLOG_SIZE);
            Socket();

            Socket(const Socket &);
            Socket (Socket &&other);

            virtual ~Socket();
            Socket &operator=(const Socket &);
            Socket &operator=(Socket &&);

            // Data Transimission
            int send ( const std::string&, int flags = 0 );
            int send ( unsigned const char *, size_t, int flags = 0);

            // Recieves a block of input
            int recv(std::string &);

            bool is_valid() const;

            Socket& operator << ( const std::string& );
            Socket& operator >> ( std::string& );

            SOCKET getSocket() const;

            const char *getIP() const;

            int getPort() const;

            void setPort(const int port);
            void setIP(const std::string &ip);

            virtual void close();

            // Client initialization
            bool connect ( const std::string &host, const int port );

            SOCKET accept(sockaddr_in &addr) const;

            bool listen();

            void set_non_blocking ( const bool );

        protected:

            static const int MAXHOSTNAME = 200;
            static const int MAXRECV = 500;
            static const int INVALID = -1;
            static const int BACKLOG_SIZE = 10;

            SOCKET sock_;
            sockaddr_in addr_;
            unsigned *references_;

            bool create ();
            bool bind ();

            int backlogSize_;
            int port_;

        private:
            void update_reference_count();

            friend class SocketServer;
        };

    }
}

#endif

