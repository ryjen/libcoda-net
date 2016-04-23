#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <algorithm>
#include <cassert>
#include <cstring>
#include "exception.h"
#include "socket_server.h"

namespace arg3
{
    namespace net
    {
        socket_server::socket_server(const factory_type &factory) : factory_(factory), backgroundThread_(nullptr), sockets_(), listeners_()
        {
        }

        socket_server::socket_server(socket_server &&other)
            : socket(std::move(other)),
              factory_(other.factory_),
              backgroundThread_(std::move(other.backgroundThread_)),
              sockets_(std::move(other.sockets_)),
              listeners_(std::move(other.listeners_))
        {
            // invalidate the moved instance
            other.sock_ = INVALID;
            other.factory_ = NULL;
            other.backgroundThread_ = nullptr;
        }

        socket_server::~socket_server()
        {
            stop();
        }

        socket_server &socket_server::operator=(socket_server &&other)
        {
            socket::operator=(std::move(other));

            factory_ = other.factory_;
            backgroundThread_ = std::move(other.backgroundThread_);
            sockets_ = std::move(other.sockets_);
            listeners_ = std::move(other.listeners_);

            // invalidate moved instance
            other.sock_ = INVALID;
            other.factory_ = NULL;
            other.backgroundThread_ = nullptr;

            return *this;
        }

        bool socket_server::operator==(const socket_server &other)
        {
            if (!is_valid() || !other.is_valid()) {
                return false;
            }

            return port() == other.port();
        }

        bool socket_server::operator!=(const socket_server &other)
        {
            return !operator==(other);
        }

        bool socket_server::is_valid() const
        {
            return socket::is_valid() && factory_ != NULL;
        }

        void socket_server::stop()
        {
            if (is_valid()) {
                notify_stop();
            }

            close();

            if (backgroundThread_ != nullptr) {
                if (backgroundThread_->joinable()) {
                    backgroundThread_->join();
                }

                backgroundThread_ = nullptr;
            }

            std::lock_guard<std::mutex> lock(sockets_mutex_);
            sockets_.clear();
        }

        socket_server &socket_server::add_listener(const listener_type &listener)
        {
            std::lock_guard<std::mutex> lock(listeners_mutex_);

            if (listener != NULL && find(listeners_.begin(), listeners_.end(), listener) == listeners_.end()) {
                listeners_.push_back(listener);
            }
            return *this;
        }

        void socket_server::set_socket_factory(const factory_type &factory)
        {
            factory_ = factory;
        }

        void socket_server::notify_start()
        {
            on_start();

            std::lock_guard<std::mutex> lock(listeners_mutex_);

            for (const auto &listener : listeners_) {
                listener->on_start(this);
            }
        }

        void socket_server::notify_stop()
        {
            on_stop();

            std::lock_guard<std::mutex> lock(listeners_mutex_);

            for (const auto &listener : listeners_) {
                listener->on_stop(this);
            }
        }

        bool socket_server::listen(const int port, const int backlogSize)
        {
            bool success = socket::listen(port, backlogSize);

            if (success) {
                notify_start();
            }

            return success;
        }

        void socket_server::start_in_background(int port, int backlogSize)
        {
            if (is_valid()) {
                throw socket_exception("server already started");
            }

            if (!listen(port, backlogSize)) {
                throw socket_exception("unable to listen on port");
            }

            backgroundThread_ = make_shared<thread>(&socket_server::run, this);
        }

        void socket_server::start(int port, int backlogSize)
        {
            if (is_valid()) {
                throw socket_exception("server already started");
            }

            if (!listen(port, backlogSize)) {
                throw socket_exception("unable to listen on port");
            }

            run();
        }

        void socket_server::on_poll()
        {
        }

        void socket_server::on_start()
        {
        }

        void socket_server::on_stop()
        {
        }

        void socket_server::run()
        {
            struct timeval last_time;

            gettimeofday(&last_time, NULL);

            assert(sockets_.empty());

            while (is_valid()) {
                sockaddr_storage addr;

                auto sock = factory_->create_socket(this, accept(addr), addr);

                sock->notify_connect();

                std::lock_guard<std::mutex> lock(sockets_mutex_);

                sockets_.push_back(sock);
            }
        }
    }
}
