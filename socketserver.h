#include "bufferedsocket.h"
#include <vector>
#include <thread>
#include <sys/time.h>

using namespace std;

namespace arg3
{
    namespace net
    {
        /* factory class to control sockets in a server */
        class SocketFactory
        {
        public:
            /* creates a buffered socket from a raw socket */
            virtual BufferedSocket* createSocket(SOCKET sock, const sockaddr_in &addr)= 0;

            /* returns a list of created sockets */
            virtual vector<BufferedSocket>& getSockets() = 0;

            /*
             * runs a delegate across all sockets.
             * return value dictates whether the socket should be removed or not.
             */
            void run(std::function<bool(BufferedSocket &)> delegate);
        };

        /* default implementation of a socket factory */
        class DefaultSocketFactory : public SocketFactory
        {
        public:
            virtual BufferedSocket* createSocket(SOCKET sock, const sockaddr_in &addr);

            vector<BufferedSocket>& getSockets();
        private:
            vector<BufferedSocket> connections_;
        };

        /* default factory instance */
        extern DefaultSocketFactory defaultSocketFactory;

        class SocketServer : public Socket
        {
        public:
            SocketServer(int port, SocketFactory *factory = &defaultSocketFactory, int queueSize = QUEUE_SIZE);
            SocketServer(const SocketServer &other);
            SocketServer(SocketServer &&other);
            virtual ~SocketServer();
            SocketServer &operator=(const SocketServer &other);
            SocketServer &operator=(SocketServer &&other);

            void start(bool inBackground = true);

            void stop();

            void setPollFrequency(unsigned value);

        private:
            void loop();

            unsigned pollFrequency_;

            thread listenThread_;

            SocketFactory *factory_;
        };
    }
}