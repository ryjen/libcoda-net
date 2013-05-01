#ifndef ARG3_NET_BUFFERED_SOCKET_H_
#define ARG3_NET_BUFFERED_SOCKET_H_

#include "socket.h"
#include <string>

using namespace std;

namespace arg3
{

    namespace net
    {

        class BufferedSocket : public Socket
        {
        public:
            BufferedSocket();
            BufferedSocket(SOCKET sock, const sockaddr_in &addr);
            BufferedSocket(const BufferedSocket &);
            BufferedSocket(BufferedSocket &&other);
            virtual ~BufferedSocket();
            BufferedSocket &operator=(const BufferedSocket &);

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

        private:

            string inBuffer_;
            string outBuffer_;

            friend class EventServerSocket;
        };
    }
}

#endif
