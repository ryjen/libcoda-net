
// Definition of the socket class

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
#include <vector>

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

        class socket
        {
        public:
            typedef char data_type;
            typedef std::vector<data_type> data_buffer;

            socket(SOCKET sock, const sockaddr_in &addr);

            socket(const std::string &host, const int port);
            socket (int port, int queueSize = BACKLOG_SIZE);
            socket();

            socket(const socket &) = delete;
            socket (socket &&other);

            virtual ~socket();
            socket &operator=(const socket &) = delete;
            socket &operator=(socket && );

            // Data Transimission
            int send ( const data_buffer &, int flags = 0 );
            int send ( void *, size_t, int flags = 0);

            // Recieves a block of input
            int recv(data_buffer &);

            bool is_valid() const;

            socket &operator << ( const data_buffer & );
            socket &operator >> ( data_buffer & );

            SOCKET raw_socket() const;

            const char *ip() const;

            int port() const;

            void set_port(const int port);
            void set_ip(const std::string &ip);

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
            //unsigned *references_;

            bool create ();
            bool bind ();

            int backlogSize_;
            int port_;

        private:
            void update_reference_count();

            friend class socket_server;
        };

    }
}

#endif

