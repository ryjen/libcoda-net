#include "buffered_socket.h"

namespace arg3
{
    namespace net
    {

        static const socket::data_type NEWLINE[] = { '\r', '\n' };

        // buffered_socket::buffered_socket() : socket(), listeners_()
        // {
        // }

        buffered_socket::buffered_socket(SOCKET sock, const sockaddr_in &addr) : socket(sock, addr), listeners_()
        {
        }


        /*buffered_socket::buffered_socket(const buffered_socket &sock) : socket(sock),
            inBuffer_(sock.inBuffer_), outBuffer_(sock.outBuffer_), listeners_(sock.listeners_)
        {

        }*/

        buffered_socket::buffered_socket(buffered_socket &&other) : socket(std::move(other)), inBuffer_(std::move(other.inBuffer_)),
            outBuffer_(std::move(other.outBuffer_)), listeners_(std::move(other.listeners_))
        {
        }

        buffered_socket::~buffered_socket()
        {

        }

        /*buffered_socket &buffered_socket::operator=(const buffered_socket &other)
        {
            if (this != &other)
            {
                socket::operator=(other);

                inBuffer_ = other.inBuffer_;
                outBuffer_ = other.outBuffer_;

                listeners_ = other.listeners_;
            }

            return *this;
        }*/

        buffered_socket &buffered_socket::operator=(buffered_socket && other)
        {
            if (this != &other)
            {

                socket::operator=(std::move(other));

                inBuffer_ = std::move(other.inBuffer_);
                outBuffer_ = std::move(other.outBuffer_);

                listeners_ = std::move(other.listeners_);
            }

            return *this;
        }

        bool buffered_socket::read_to_buffer()
        {
            data_buffer chunk;

            notify_will_read();

            int status = socket::recv(chunk);

            while (status > 0)
            {
                inBuffer_.insert(inBuffer_.end(), chunk.begin(), chunk.end());

                status = socket::recv(chunk);
            }

            bool success = status == 0 || errno == EWOULDBLOCK;

            if (success)
                notify_did_read();

            return success;
        }

        string buffered_socket::readln()
        {
            if (inBuffer_.empty()) return string(inBuffer_.begin(), inBuffer_.end());

            data_type needle[] = { '\n', '\r' };

            auto pos = find_first_of(inBuffer_.begin(), inBuffer_.end(), needle, needle + 2);

            if (pos == inBuffer_.end())
                return string(inBuffer_.begin(), inBuffer_.end());


            while (pos != inBuffer_.end() &&
                    (*pos == '\n' || *pos == '\r'))
            {
                pos++;
            }

            string temp(inBuffer_.begin(), pos);

            inBuffer_.erase(inBuffer_.begin(), pos);

            return temp;
        }

        bool buffered_socket::has_input() const
        {
            return !inBuffer_.empty();
        }

        const socket::data_buffer &buffered_socket::input() const
        {
            return inBuffer_;
        }

        bool buffered_socket::has_output() const
        {
            return !outBuffer_.empty();
        }

        const socket::data_buffer &buffered_socket::output() const
        {
            return outBuffer_;
        }

        buffered_socket &buffered_socket::writeln(const string &value)
        {
            outBuffer_.insert(outBuffer_.end(), value.begin(), value.end());
            outBuffer_.insert(outBuffer_.end(), NEWLINE, NEWLINE + 2);
            return *this;
        }

        buffered_socket &buffered_socket::writeln()
        {
            outBuffer_.insert(outBuffer_.end(), NEWLINE, NEWLINE + 2);
            return *this;
        }

        buffered_socket &buffered_socket::write(void *pbuf, size_t sz)
        {
            socket::send(pbuf, sz);
            return *this;
        }

        buffered_socket &buffered_socket::write(const string &value)
        {
            outBuffer_.insert(outBuffer_.end(), value.begin(), value.end());
            return *this;
        }

        buffered_socket &buffered_socket::operator << ( const string &s )
        {
            return write(s);
        }

        buffered_socket &buffered_socket::operator >> ( string &s )
        {
            s.append(inBuffer_.begin(), inBuffer_.end());

            return *this;
        }

        void buffered_socket::flush()
        {
            send(outBuffer_);

            outBuffer_.clear();
        }

        void buffered_socket::close()
        {
            notify_close();

            flush();

            socket::close();
        }

        bool buffered_socket::write_from_buffer()
        {
            notify_will_write();

            if (send(outBuffer_) < 0)
            {
                return false;
            }

            outBuffer_.clear();

            notify_did_write();

            return true;
        }

        void buffered_socket::on_connect() {}
        void buffered_socket::on_close() {}
        void buffered_socket::on_will_read() {}
        void buffered_socket::on_did_read() {}
        void buffered_socket::on_will_write() {}
        void buffered_socket::on_did_write() {}


        void buffered_socket::add_listener(buffered_socket_listener *listener)
        {
            if (listener != NULL)
                listeners_.push_back(listener);
        }

        void buffered_socket::notify_connect()
        {
            on_connect();

            for (auto & l : listeners_)
            {
                l->on_connect(this);
            }
        }

        void buffered_socket::notify_will_read()
        {
            on_will_read();

            for (auto & l : listeners_)
            {
                l->on_will_read(this);
            }
        }

        void buffered_socket::notify_did_read()
        {
            on_did_read();

            for (auto & l : listeners_)
            {
                l->on_did_read(this);
            }
        }

        void buffered_socket::notify_will_write()
        {
            on_will_write();

            for (auto & l : listeners_)
            {
                l->on_will_write(this);
            }
        }

        void buffered_socket::notify_did_write()
        {
            on_did_write();

            for (auto & l : listeners_)
            {
                l->on_did_write(this);
            }
        }

        void buffered_socket::notify_close()
        {
            on_close();

            for (auto & l : listeners_)
            {
                l->on_close(this);
            }
        }

    }
}