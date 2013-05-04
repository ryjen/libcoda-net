#include "bufferedsocket.h"
#include <vector>
#include <thread>
#include <sys/time.h>

using namespace std;

namespace arg3
{
    namespace net
    {
        class SocketServerListener
        {
        public:
            virtual void onWillRead(BufferedSocket &sock) = 0;

            virtual void onDidRead(BufferedSocket &sock) = 0;

            virtual void onWillWrite(BufferedSocket &sock) = 0;

            virtual void onDidWrite(BufferedSocket &sock) = 0;

            virtual void onConnect(BufferedSocket &sock) = 0;

            virtual void onClose(BufferedSocket &sock) = 0;
        };

        class SocketServer : public Socket
        {
        public:
            SocketServer(int port, int queueSize = QUEUE_SIZE);
            SocketServer(const SocketServer &other);
            SocketServer(SocketServer &&other);
            virtual ~SocketServer();
            SocketServer &operator=(const SocketServer &other);
            SocketServer &operator=(SocketServer &&other);

            void start(bool inBackground = true);

            void stop();

            void setPollFrequency(unsigned value);

            void addListener(SocketServerListener *listener);

        private:
            void notifyWillRead(BufferedSocket &sock);

            void notifyDidRead(BufferedSocket &sock);

            void notifyWillWrite(BufferedSocket &sock);

            void notifyDidWrite(BufferedSocket &sock);

            void notifyConnect(BufferedSocket &sock);

            void notifyClose(BufferedSocket &sock);

            void loop();

            unsigned pollFrequency_;

            thread listenThread_;

            vector<BufferedSocket> connections_;

            vector<SocketServerListener*> listeners_;
        };
    }
}