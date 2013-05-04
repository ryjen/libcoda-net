#include "bufferedsocket.h"

namespace arg3
{
    namespace net
    {
        BufferedSocket::BufferedSocket()
        {
        }

        BufferedSocket::BufferedSocket(SOCKET sock, const sockaddr_in &addr) : Socket(sock, addr)
        {}


        BufferedSocket::BufferedSocket(const BufferedSocket &sock) : Socket(sock),
            inBuffer_(sock.inBuffer_), outBuffer_(sock.outBuffer_)
        {

        }

        BufferedSocket::BufferedSocket(BufferedSocket &&other) : Socket(other), inBuffer_(other.inBuffer_),
            outBuffer_(other.outBuffer_)
        {
            other.inBuffer_.clear();
            other.outBuffer_.clear();
        }

        BufferedSocket::~BufferedSocket()
        {

        }
        BufferedSocket &BufferedSocket::operator=(const BufferedSocket &other)
        {
            if(this != &other)
            {
                Socket::operator=(other);

                inBuffer_ = other.inBuffer_;
                outBuffer_ = other.outBuffer_;
            }

            return *this;
        }
        BufferedSocket &BufferedSocket::operator=(BufferedSocket &&other)
        {
            if(this != &other)
            {
                Socket::operator=(std::move(other));

                inBuffer_ = std::move(other.inBuffer_);
                outBuffer_ = std::move(other.outBuffer_);
            }

            return *this;
        }

        bool BufferedSocket::readToBuffer()
        {
            string chunk;

            int status = Socket::recv(chunk);

            while(status > 0)
            {
                inBuffer_.append(chunk);

                status = Socket::recv(chunk);
            }

            return status == 0 || errno == EWOULDBLOCK;
        }

        string BufferedSocket::readLine()
        {
            if(inBuffer_.empty()) return inBuffer_;

            auto pos = inBuffer_.find_first_of("\n\r");

            if(pos == string::npos) return inBuffer_;

            string temp = inBuffer_.substr(0, pos);

            while(pos < inBuffer_.length() &&
                    (inBuffer_[pos] == '\n' || inBuffer_[pos] == '\r')) {
                pos++;
            }

            inBuffer_.erase(0, pos);

            return temp;
        }

        bool BufferedSocket::hasInput() const
        {
            return !inBuffer_.empty();
        }

        string BufferedSocket::getInput() const
        {
            return inBuffer_;
        }

        bool BufferedSocket::hasOutput() const
        {
            return !outBuffer_.empty();
        }

        string BufferedSocket::getOutput() const
        {
            return outBuffer_;
        }

        BufferedSocket& BufferedSocket::writeLine(const string &value)
        {
            outBuffer_.append(value).append("\n\r");
            return *this;
        }

        BufferedSocket& BufferedSocket::write(const string &value)
        {
            outBuffer_.append(value);
            return *this;
        }

        BufferedSocket& BufferedSocket::operator << ( const string& s )
        {
            return write(s);
        }

        BufferedSocket& BufferedSocket::operator >> ( string& s )
        {
            s.append(inBuffer_);

            return *this;
        }

        bool BufferedSocket::writeFromBuffer()
        {
            if(send(outBuffer_) < 0)
            {
                return false;
            }

            outBuffer_.clear();
            return true;
        }

    }
}