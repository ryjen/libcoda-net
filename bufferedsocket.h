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
        class BufferedSocket;

        class BufferedSocketListener
        {
        public:
            virtual void onWillRead(BufferedSocket *sock) = 0;

            virtual void onDidRead(BufferedSocket *sock) = 0;

            virtual void onWillWrite(BufferedSocket *sock) = 0;

            virtual void onDidWrite(BufferedSocket *sock) = 0;

            virtual void onConnect(BufferedSocket *sock) = 0;

            virtual void onClose(BufferedSocket *sock) = 0;
        };

        class BufferedSocket : public Socket
        {
        public:
            BufferedSocket();
            BufferedSocket(SOCKET sock, const sockaddr_in &addr);
            BufferedSocket(const BufferedSocket &);
            BufferedSocket(BufferedSocket &&other);
            virtual ~BufferedSocket();
            BufferedSocket &operator=(const BufferedSocket &other);
            BufferedSocket &operator=(BufferedSocket &&other);

            void close();

            bool readToBuffer();
            string readLine();

            BufferedSocket& writeLine(const string &value);
            BufferedSocket& write(const string &value);

            string getInput() const;
            bool hasInput() const;

            string getOutput() const;
            bool hasOutput() const;

            bool writeFromBuffer();


            BufferedSocket& operator << ( const std::string& );
            BufferedSocket& operator >> ( std::string& );

            void addListener(BufferedSocketListener *listener);

        private:

            void notifyWillRead();

            void notifyDidRead();

            void notifyWillWrite();

            void notifyDidWrite();

            void notifyConnect();

            void notifyClose();

            vector<BufferedSocketListener *> listeners_;

            string inBuffer_;
            string outBuffer_;
        };
    }
}

#endif
