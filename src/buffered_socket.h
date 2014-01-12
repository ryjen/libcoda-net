#ifndef ARG3_NET_BUFFERED_SOCKET_H_
#define ARG3_NET_BUFFERED_SOCKET_H_

#include "socket.h"
#include <string>
#include <vector>

using namespace std;

namespace arg3
{

    namespace net
    {
        class buffered_socket;

        class buffered_socket_listener
        {
        public:
            virtual void on_will_read(buffered_socket *sock) = 0;

            virtual void on_did_read(buffered_socket *sock) = 0;

            virtual void on_will_write(buffered_socket *sock) = 0;

            virtual void on_did_write(buffered_socket *sock) = 0;

            virtual void on_connect(buffered_socket *sock) = 0;

            virtual void on_close(buffered_socket *sock) = 0;
        };

        class buffered_socket : public socket
        {
        public:
            //buffered_socket();
            buffered_socket(SOCKET sock, const sockaddr_in &addr);
            buffered_socket(const buffered_socket &) = delete;
            buffered_socket(buffered_socket &&other);
            virtual ~buffered_socket();
            buffered_socket &operator=(const buffered_socket &other) = delete;
            buffered_socket &operator=(buffered_socket && other);

            void close();

            bool read_to_buffer();
            string readln();

            buffered_socket &writeln(const string &value);
            buffered_socket &writeln();
            buffered_socket &write(const string &value);
            buffered_socket &write(void *pbuf, size_t sz);

            const data_buffer &input() const;
            bool has_input() const;

            const data_buffer &output() const;
            bool has_output() const;

            bool write_from_buffer();

            buffered_socket &operator << ( const std::string & );
            buffered_socket &operator >> ( std::string & );

            void add_listener(buffered_socket_listener *listener);

            void notify_connect();

        protected:

            virtual void on_will_read();
            virtual void on_did_read();
            virtual void on_will_write();
            virtual void on_did_write();
            virtual void on_connect();
            virtual void on_close();

            data_buffer inBuffer_;
            data_buffer outBuffer_;
        private:

            void flush();

            void notify_will_read();

            void notify_did_read();

            void notify_will_write();

            void notify_did_write();

            void notify_close();

            vector<buffered_socket_listener *> listeners_;
        };
    }
}

#endif
