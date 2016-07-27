#ifndef RJ_NET_BUFFERED_SOCKET_H_
#define RJ_NET_BUFFERED_SOCKET_H_

#include <memory>
#include <string>
#include <vector>
#include "socket.h"

namespace rj
{
    namespace net
    {
        class buffered_socket;

        /*!
         * An interface to listen to a buffered socket
         */
        class buffered_socket_listener
        {
           public:
            typedef buffered_socket *socket_type;

            /*!
             * Runs just before a read
             */
            virtual void on_will_read(const socket_type &sock) = 0;

            /*!
             * Runs just after a read
             */
            virtual void on_did_read(const socket_type &sock) = 0;

            /*!
             * Runs just before a write
             */
            virtual void on_will_write(const socket_type &sock) = 0;

            /*!
             * Runs just after a write
             */
            virtual void on_did_write(const socket_type &sock) = 0;

            /*!
             * Runs when connected
             */
            virtual void on_connect(const socket_type &sock) = 0;

            /*!
             * Runs when closed
             */
            virtual void on_close(const socket_type &sock) = 0;
        };

        /*!
         * A socket that buffers its input and output.
         */
        class buffered_socket : public socket
        {
           public:
            typedef std::shared_ptr<buffered_socket_listener> listener_type;

            /*!
             * Default constructor accepts a raw socket and its address
             */
            buffered_socket(SOCKET sock, const sockaddr_storage &addr);

            buffered_socket(const std::string &host, const int port);

            buffered_socket();

            /*!
            * Non copyable
            */
            buffered_socket(const buffered_socket &) = delete;

            /*!
             * Move constructor
             */
            buffered_socket(buffered_socket &&other);

            /*!
            * Destructor
            */
            virtual ~buffered_socket();

            /*!
             * Non copy-assignable
             */
            buffered_socket &operator=(const buffered_socket &other) = delete;

            /*!
             * Move assigment
             */
            buffered_socket &operator=(buffered_socket &&other);

            /*!
             * Will close the raw socket
             */
            void close();

            /*!
             * Client initialization, connects to a host and port
             */
            virtual bool connect(const std::string &host, const int port);

            /*!
             * Reads data from the socket into the read buffer
             * @returns true if no errors occured
             */
            bool read_to_buffer();

            /*!
             * Reads a line from the read buffer
             */
            std::string readln();

            /*!
             * Appends some data to the write buffer with a new line.
             */
            buffered_socket &writeln(const std::string &value);

            /*!
            * Appends a new line to the write buffer
            */
            buffered_socket &writeln();

            /*!
             * Appends some data to the write buffer.
             */
            buffered_socket &write(const std::string &value);

            /*!
             * Appends some bytes to the write buffer
             */
            buffered_socket &write(void *pbuf, size_t sz);

            /*!
             * @returns the read buffer
             */
            const data_buffer &input() const;

            /*!
             * @returns true if the read buffer contains data
             */
            bool has_input() const;

            /*!
             * @returns the write buffer
             */
            const data_buffer &output() const;

            /*!
             * @returns true if the write buffer contains data
             */
            bool has_output() const;

            /*!
             * Will write the buffer to the actual socket
             * @returns true if no errors occured
             */
            bool write_from_buffer();

            /*!
             * Appends some data to the write buffer
             */
            buffered_socket &operator<<(const std::string &);

            /*!
             * Appends the read buffer to a string
             */
            buffered_socket &operator>>(std::string &);

            /*!
             * Adds a listener to this socket
             */
            buffered_socket &add_listener(const listener_type &listener);

            buffered_socket &remove_listener(const listener_type &listener);
            /*!
             * Notifies listeners of a connection.  This must be called after
             * any listeners have been added.
             */
            void notify_connect();

           protected:
            /* These can be overrided, instead of adding a listener*/
            virtual void on_will_read();
            virtual void on_did_read();
            virtual void on_will_write();
            virtual void on_did_write();
            virtual void on_connect();
            virtual void on_close();

            /* the actual buffers */
            data_buffer inBuffer_;
            data_buffer outBuffer_;

           private:
            /*!
             * Sends the write buffer to the socket
             */
            void flush();

            void notify_will_read();

            void notify_did_read();

            void notify_will_write();

            void notify_did_write();

            void notify_close();

            bool read_chunk(data_buffer &chunk);

            std::vector<listener_type> listeners_;
        };
    }
}

#endif
