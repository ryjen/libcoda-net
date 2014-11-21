#include "config.h"
#include "buffered_socket.h"
#include <algorithm>

namespace arg3
{
    namespace net
    {

        static const socket::data_buffer NEWLINE = { '\r', '\n' };

        buffered_socket::buffered_socket() : socket()
        {}

        buffered_socket::buffered_socket(SOCKET sock, const sockaddr_storage &addr) : socket(sock, addr)
        {
        }

        buffered_socket::buffered_socket(const std::string &host, const int port) : socket(host, port)
        {
        }

        buffered_socket::buffered_socket(buffered_socket &&other) : socket(std::move(other)), inBuffer_(std::move(other.inBuffer_)),
            outBuffer_(std::move(other.outBuffer_)), listeners_(std::move(other.listeners_))
        {
        }

        buffered_socket::~buffered_socket()
        {
        }

        buffered_socket &buffered_socket::operator=(buffered_socket && other)
        {
            socket::operator=(std::move(other));

            inBuffer_ = std::move(other.inBuffer_);
            outBuffer_ = std::move(other.outBuffer_);

            listeners_ = std::move(other.listeners_);

            return *this;
        }

        bool buffered_socket::connect(const string &host, const int port)
        {
            if (socket::connect(host, port))
            {
                notify_connect();
                return true;
            }
            return false;
        }

        bool buffered_socket::read_to_buffer()
        {
            data_buffer chunk;

            notify_will_read();

            int status = socket::recv(chunk);

            size_t size = 0;

            while (status > 0)
            {
                size += chunk.size();

                inBuffer_.insert(inBuffer_.end(), chunk.begin(), chunk.end());

                status = socket::recv(chunk);
            }

            bool success = status == 0 || errno == EWOULDBLOCK;

            if (success && size > 0)
                notify_did_read();

            return success;
        }

        string buffered_socket::readln()
        {
            if (inBuffer_.empty()) return string();

            /* find a new line character */
            auto pos = find_first_of(inBuffer_.begin(), inBuffer_.end(), NEWLINE.begin(), NEWLINE.end());

            if (pos == inBuffer_.end())
            {
                string temp(inBuffer_.begin(), inBuffer_.end());

                inBuffer_.clear();

                return temp;
            }

            string temp(inBuffer_.begin(), pos);

            /* Skip all new line characters */
            while (pos != inBuffer_.end() && find(NEWLINE.begin(), NEWLINE.end(), *pos) != NEWLINE.end())
            {
                pos++;
            }

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
            outBuffer_.insert(outBuffer_.end(), NEWLINE.begin(), NEWLINE.end());
            return *this;
        }

        buffered_socket &buffered_socket::writeln()
        {
            outBuffer_.insert(outBuffer_.end(), NEWLINE.begin(), NEWLINE.end());
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
            if (is_valid())
                send(outBuffer_);

            outBuffer_.clear();
        }

        void buffered_socket::close()
        {
            if (is_valid())
            {
                notify_close();

                flush();

                socket::close();
            }
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

        /*!
         * default implementations do nothing
         */
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

            for (auto l : listeners_)
            {
                l->on_close(this);
            }
        }

    }
}
