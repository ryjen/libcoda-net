#include "Socket.h"

namespace arg3
{
    namespace net
    {
        class ServerSocket : public Socket
        {
        private:
            static const int QUEUE_SIZE = 10;
        public:

            ServerSocket (int queueSize = QUEUE_SIZE);
            ServerSocket(const ServerSocket &);
            virtual ~ServerSocket();
            ServerSocket& operator=(const ServerSocket&);

            // Server initialization
            bool accept(Socket &) const;
            void start(int);
        protected:
            bool listen() const;
            bool create();
            bool bind (int);

            int backlogSize_;
        };

    }
}

