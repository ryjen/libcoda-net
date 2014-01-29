
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

#ifdef WIN32
        typedef int socklen_t;
#else
        typedef int SOCKET;
        int closesocket(SOCKET socket);
#endif

        /*!
         * A wrapper for a raw socket.  Includes both client and server functionality
         */
        class socket
        {
        public:
            /*!
             * the base data type for sockets
             */
            typedef char data_type;

            /*!
             * data type for a data buffer
             */
            typedef std::vector<data_type> data_buffer;

            /*!
             * constructor to take a raw socket and its address
             */
            socket(SOCKET sock, const sockaddr_in &addr);

            /*!
             * Constructor to open a socket given the host and port
             */
            socket(const std::string &host, const int port);

            /*!
             * Constructor to listen on a port
             * @param port the port to listen on
             * @param backlog the max number of simultaneous new connections (not the same as the num of current connections)
             */
            socket (int port, int backlog = BACKLOG_SIZE);

            /*!
             * default constructor
             */
            socket();

            /*!
             * Non copyable
             */
            socket(const socket &) = delete;

            /*!
             * move constructor
             */
            socket (socket &&other);

            /*!
             * destructor will close the socket RAII style
             */
            virtual ~socket();

            /*!
             * Non-copyable assignment operator
             */
            socket &operator=(const socket &) = delete;

            /*!
             * Move assignment operator
             */
            socket &operator=(socket && );

            // Data Transimission

            /*!
             * Will write a block of data to the socket
             * @return the number of bytes written
             */
            int send ( const data_buffer &, int flags = 0 );

            /*!
             * Will write a block of data to the socket
             * @return the number of bytes written
             */
            int send ( void *, size_t, int flags = 0);

            /*!
             * Recieves a block of input
             * @returns the number of bytes read
             */
            int recv(data_buffer &);

            /*!
             * @returns true if the socket is alive and connected
             */
            bool is_valid() const;

            /*!
             * Writes a data buffer to the socket
             */
            socket &operator << ( const data_buffer & );

            /*!
             * Reads data from the socket into a data buffer
             */
            socket &operator >> ( data_buffer & );

            /*!
             * @returns the raw socket
             */
            SOCKET raw_socket() const;


            /*!
             * @retuns the ip address of the socket
             */
            const char *ip() const;

            /*!
             * @returns the port of the socket
             */
            int port() const;

            /*!
             * sets the port of the socket
             */
            void set_port(const int port);

            /*!
             * sets the ip of the socket
             */
            void set_ip(const std::string &ip);

            /*!
             * closes the socket
             */
            virtual void close();

            /*!
             * Client initialization, connects to a host and port
             */
            bool connect ( const std::string &host, const int port );

            /*!
             * Accepts a socket
             * @param addr the address structure to populate
             * @returns the connected socket
             */
            SOCKET accept(sockaddr_in &addr) const;

            /*!
             * puts the socket in listen mode
             */
            bool listen();

            /*!
             * sets the socket in blocking or non blocking mode
             */
            void set_non_blocking ( const bool );

        protected:

            static const int MAXHOSTNAME = 200;
            static const int MAXRECV = 500;
            static const int INVALID = -1;
            static const int BACKLOG_SIZE = 10;

            // the raw socket
            SOCKET sock_;
            // the socket address
            sockaddr_in addr_;

            /*!
             * creates a new socket
             */
            bool create ();

            /*!
             * binds the socket to its address
             */
            bool bind ();

            int backlogSize_;
            int port_;

            friend class socket_server;
        };
    }
}

#endif

