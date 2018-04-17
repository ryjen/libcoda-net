
#include "buffered_socket.h"
#include <algorithm>
#include <cstring>
#include "exception.h"

using namespace std;

namespace coda
{
    namespace net
    {
        static const socket::data_buffer NEWLINE = {'\r', '\n'};

        buffered_socket::buffered_socket() : socket()
        {
        }

        buffered_socket::buffered_socket(SOCKET sock, const sockaddr_storage &addr) : socket(sock, addr)
        {
        }

        buffered_socket::buffered_socket(const std::string &host, const int port) : socket(host, port)
        {
        }

        buffered_socket::buffered_socket(buffered_socket &&other)
            : socket(std::move(other)),
              inBuffer_(std::move(other.inBuffer_)),
              outBuffer_(std::move(other.outBuffer_)),
              listeners_(std::move(other.listeners_))
        {
        }

        buffered_socket::~buffered_socket()
        {
        }

        buffered_socket &buffered_socket::operator=(buffered_socket &&other)
        {
            socket::operator=(std::move(other));

            inBuffer_ = std::move(other.inBuffer_);
            outBuffer_ = std::move(other.outBuffer_);
            listeners_ = std::move(other.listeners_);

            return *this;
        }

        //! for client connections, this will connect the socket to a host/port
        /*!
         * @param   host    the hostname to connect to
         * @param   port    the port to connect to
         *
         * @returns     true if successful
         */
        bool buffered_socket::connect(const string &host, const int port)
        {
            if (socket::connect(host, port)) {
                notify_connect();
                return true;
            }
            return false;
        }


        bool buffered_socket::read_chunk(data_buffer &chunk)
        {
            if (!is_valid()) {
                return false;
            }

            int status = socket::recv(chunk);

            if (status < 0) {
                if (errno == EINTR) {
                    return false; /* perfectly normal */
                }

                if (errno == EAGAIN) {
                    return false;
                }

                if (errno == EWOULDBLOCK) {
                    return false;
                }

                throw socket_exception(strerror(errno));
            }

            return status > 0;
        }

        //! reads from the socket into an internal input buffer
        /*
         * @returns     true if successful
         */
        bool buffered_socket::read_to_buffer()
        {
            data_buffer chunk;

            notify_will_read();

            try {
                if (!read_chunk(chunk)) {
                    return true;
                }

                // while not an error or the peer connection was closed
                do {
                    inBuffer_.insert(inBuffer_.end(), chunk.begin(), chunk.end());
                } while (is_non_blocking() && read_chunk(chunk));

                notify_did_read();
            } catch (const socket_exception &e) {
                return false;
            }

            return true;
        }

        //! Reads a line from the internal input buffer
        /*!
         * @returns a line from the buffer
         */
        string buffered_socket::readln()
        {
            if (inBuffer_.empty()) return string();

            /* find a new line  */
            auto pos = find_first_of(inBuffer_.begin(), inBuffer_.end(), NEWLINE.begin(), NEWLINE.end());

            if (pos == inBuffer_.end()) {
                string temp(inBuffer_.begin(), inBuffer_.end());
                inBuffer_.clear();
                return temp;
            }

            string temp(inBuffer_.begin(), pos);

            /* Skip all new line characters, squelching blank lines */
            while (pos != inBuffer_.end() && find(NEWLINE.begin(), NEWLINE.end(), *pos) != NEWLINE.end()) {
                pos++;
            }

            inBuffer_.erase(inBuffer_.begin(), pos);

            return temp;
        }

        //! tests if the internal input buffer has content
        bool buffered_socket::has_input() const
        {
            return !inBuffer_.empty();
        }

        //! gets the internal data buffer
        const socket::data_buffer &buffered_socket::input() const
        {
            return inBuffer_;
        }

        //! tests internal output buffer for content
        bool buffered_socket::has_output() const
        {
            return !outBuffer_.empty();
        }

        //! the internal output buffer
        const socket::data_buffer &buffered_socket::output() const
        {
            return outBuffer_;
        }

        //! writes a string to the output buffer and an appending new line
        buffered_socket &buffered_socket::writeln(const string &value)
        {
            outBuffer_.insert(outBuffer_.end(), value.begin(), value.end());
            outBuffer_.insert(outBuffer_.end(), NEWLINE.begin(), NEWLINE.end());
            return *this;
        }

        //! writes a new line to the output buffer
        buffered_socket &buffered_socket::writeln()
        {
            outBuffer_.insert(outBuffer_.end(), NEWLINE.begin(), NEWLINE.end());
            return *this;
        }

        //!
        buffered_socket &buffered_socket::write(const string &value)
        {
            outBuffer_.insert(outBuffer_.end(), value.begin(), value.end());
            return *this;
        }

        //!  append to the output buffer operator
        buffered_socket &buffered_socket::operator<<(const string &s)
        {
            return write(s);
        }

        //! read from the input buffer operator
        buffered_socket &buffered_socket::operator>>(string &s)
        {
            s.append(inBuffer_.begin(), inBuffer_.end());
            return *this;
        }

        //! flush the output buffer
        void buffered_socket::flush()
        {
            write_from_buffer();
        }

        //! close the socket
        /*!
         * Will update listeners, flush the output, and close if valid
         */
        void buffered_socket::close()
        {
            if (is_valid()) {
                notify_close();
                flush();
                socket::close();
            }
        }

        //! will write the output buffer to the socket
        bool buffered_socket::write_from_buffer()
        {
            if (!is_valid()) {
                return false;
            }

            if (outBuffer_.empty()) {
                return true;
            }

            notify_will_write();

            if (send(outBuffer_) != static_cast<int>(outBuffer_.size())) {
                return false;
            }

            outBuffer_.clear();
            notify_did_write();
            return true;
        }

        /*!
         * default implementations do nothing
         */
        void buffered_socket::on_connect()
        {
        }
        void buffered_socket::on_close()
        {
        }
        void buffered_socket::on_will_read()
        {
        }
        void buffered_socket::on_did_read()
        {
        }
        void buffered_socket::on_will_write()
        {
        }
        void buffered_socket::on_did_write()
        {
        }


        //! add a listener to the socket
        buffered_socket &buffered_socket::add_listener(const listener_type &listener)
        {
            if (listener != NULL && find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
                listeners_.push_back(listener);
            }
            return *this;
        }

        buffered_socket &buffered_socket::remove_listener(const listener_type &listener)
        {
            if (listener == nullptr) {
                return *this;
            }

            std::remove(listeners_.begin(), listeners_.end(), listener);

            return *this;
        }

        void buffered_socket::notify_connect()
        {
            on_connect();

            for (const auto &l : listeners_) {
                l->on_connect(this);
            }
        }

        void buffered_socket::notify_will_read()
        {
            on_will_read();

            for (const auto &l : listeners_) {
                l->on_will_read(this);
            }
        }

        void buffered_socket::notify_did_read()
        {
            on_did_read();

            for (const auto &l : listeners_) {
                l->on_did_read(this);
            }
        }

        void buffered_socket::notify_will_write()
        {
            on_will_write();

            for (const auto &l : listeners_) {
                l->on_will_write(this);
            }
        }

        void buffered_socket::notify_did_write()
        {
            on_did_write();

            for (const auto &l : listeners_) {
                l->on_did_write(this);
            }
        }

        void buffered_socket::notify_close()
        {
            on_close();

            for (const auto &l : listeners_) {
                l->on_close(this);
            }
        }
    }
}
