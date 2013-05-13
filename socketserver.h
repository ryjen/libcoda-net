#ifndef THIN

#include "socketfactory.h"
#include <vector>
#include <thread>
#include <sys/time.h>

using namespace std;

namespace arg3
{
    namespace net
    {
        class SocketServer;

        class SocketServerListener
        {
        public:
            virtual void onPoll(SocketServer *server) = 0;
        };

        class SocketServer : public Socket
        {
        public:
            SocketServer(int port, SocketFactory *factory = &defaultSocketFactory, int backlogSize = BACKLOG_SIZE);
            SocketServer(const SocketServer &other);
            SocketServer(SocketServer &&other);
            virtual ~SocketServer();
            SocketServer &operator=(const SocketServer &other);
            SocketServer &operator=(SocketServer &&other);

            void start();

            void loop();

            void stop();

            void setPollFrequency(unsigned value);

            void addListener(SocketServerListener *listener);
        protected:

            virtual void onPoll();

            virtual void onStart();

            virtual void onStop();

        private:

            void notifyPoll();

            unsigned pollFrequency_;

            thread listenThread_;

            SocketFactory *factory_;

            vector<SocketServerListener*> listeners_;
        };
    }
}

#endif
