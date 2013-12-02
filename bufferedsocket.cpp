#include "bufferedsocket.h"

namespace arg3
{
    namespace net
    {
        BufferedSocket::BufferedSocket() : Socket(), listeners_()
        {
        }

        BufferedSocket::BufferedSocket(SOCKET sock, const sockaddr_in &addr) : Socket(sock, addr), listeners_()
        {
        }


        BufferedSocket::BufferedSocket(const BufferedSocket &sock) : Socket(sock),
            inBuffer_(sock.inBuffer_), outBuffer_(sock.outBuffer_), listeners_(sock.listeners_)
        {

        }

        BufferedSocket::BufferedSocket(BufferedSocket &&other) : Socket(other), inBuffer_(std::move(other.inBuffer_)),
            outBuffer_(std::move(other.outBuffer_)), listeners_(std::move(other.listeners_))
        {
        }

        BufferedSocket::~BufferedSocket()
        {

        }
        BufferedSocket &BufferedSocket::operator=(const BufferedSocket &other)
        {
            if (this != &other)
            {
                Socket::operator=(other);

                inBuffer_ = other.inBuffer_;
                outBuffer_ = other.outBuffer_;

                listeners_ = other.listeners_;
            }

            return *this;
        }
        BufferedSocket &BufferedSocket::operator=(BufferedSocket && other)
        {
            if (this != &other)
            {

                Socket::operator=(std::move(other));

                inBuffer_ = std::move(other.inBuffer_);
                outBuffer_ = std::move(other.outBuffer_);

                listeners_ = std::move(other.listeners_);
            }

            return *this;
        }

        bool BufferedSocket::readToBuffer()
        {
            string chunk;

            notifyWillRead();

            int status = Socket::recv(chunk);

            while (status > 0)
            {
                inBuffer_.append(chunk);

                status = Socket::recv(chunk);
            }

            bool success = status == 0 || errno == EWOULDBLOCK;

            if (success)
                notifyDidRead();

            return success;
        }

        string BufferedSocket::readLine()
        {
            if (inBuffer_.empty()) return inBuffer_;

            auto pos = inBuffer_.find_first_of("\n\r");

            if (pos == string::npos) return inBuffer_;

            string temp = inBuffer_.substr(0, pos);

            while (pos < inBuffer_.length() &&
                    (inBuffer_[pos] == '\n' || inBuffer_[pos] == '\r'))
            {
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

        BufferedSocket &BufferedSocket::writeLine(const string &value)
        {
            outBuffer_.append(value).append(NEWLINE);
            return *this;
        }

        BufferedSocket &BufferedSocket::writeLine()
        {
            outBuffer_.append(NEWLINE);
            return *this;
        }

        BufferedSocket &BufferedSocket::write(void *pbuf, size_t sz)
        {
            Socket::send(pbuf, sz);
            return *this;
        }

        BufferedSocket &BufferedSocket::write(const string &value)
        {
            outBuffer_.append(value);
            return *this;
        }

        BufferedSocket &BufferedSocket::operator << ( const string &s )
        {
            return write(s);
        }

        BufferedSocket &BufferedSocket::operator >> ( string &s )
        {
            s.append(inBuffer_);

            return *this;
        }

        void BufferedSocket::flush()
        {
            send(outBuffer_);

            outBuffer_.clear();
        }

        void BufferedSocket::close()
        {
            notifyClose();

            flush();

            Socket::close();
        }

        bool BufferedSocket::writeFromBuffer()
        {
            notifyWillWrite();

            if (send(outBuffer_) < 0)
            {
                return false;
            }

            outBuffer_.clear();

            notifyDidWrite();

            return true;
        }

        void BufferedSocket::onConnect() {}
        void BufferedSocket::onClose() {}
        void BufferedSocket::onWillRead() {}
        void BufferedSocket::onDidRead() {}
        void BufferedSocket::onWillWrite() {}
        void BufferedSocket::onDidWrite() {}


        void BufferedSocket::addListener(BufferedSocketListener *listener)
        {
            if (listener != NULL)
                listeners_.push_back(listener);
        }

        void BufferedSocket::notifyConnect()
        {
            onConnect();

            for (auto & l : listeners_)
            {
                l->onConnect(this);
            }
        }

        void BufferedSocket::notifyWillRead()
        {
            onWillRead();

            for (auto & l : listeners_)
            {
                l->onWillRead(this);
            }
        }

        void BufferedSocket::notifyDidRead()
        {
            onDidRead();

            for (auto & l : listeners_)
            {
                l->onDidRead(this);
            }
        }

        void BufferedSocket::notifyWillWrite()
        {
            onWillWrite();

            for (auto & l : listeners_)
            {
                l->onWillWrite(this);
            }
        }

        void BufferedSocket::notifyDidWrite()
        {
            onDidWrite();

            for (auto & l : listeners_)
            {
                l->onDidWrite(this);
            }
        }

        void BufferedSocket::notifyClose()
        {
            onClose();

            for (auto & l : listeners_)
            {
                l->onClose(this);
            }
        }

    }
}