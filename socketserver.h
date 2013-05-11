#include "bufferedsocket.h"
#include <vector>
#include <thread>
#include <sys/time.h>

using namespace std;

namespace arg3
{
    namespace net
    {
        class SocketFactory
        {
        public:
            virtual BufferedSocket* createSocket(SOCKET sock, const sockaddr_in &addr)= 0;

            virtual vector<BufferedSocket>& getSockets() = 0;

            void run(std::function<bool(BufferedSocket &)> delegate);
        };

        class DefaultSocketFactory : public SocketFactory
        {
        public:
            virtual BufferedSocket* createSocket(SOCKET sock, const sockaddr_in &addr);

            vector<BufferedSocket>& getSockets();
        private:
            vector<BufferedSocket> connections_;
        };

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