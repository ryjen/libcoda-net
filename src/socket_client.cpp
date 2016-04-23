#include "socket_client.h"

namespace arg3
{
    namespace net
    {
        socket_client::~socket_client()
        {
            if (thread_ && thread_->joinable()) {
                thread_->join();
            }
        }
        void socket_client::run()
        {
            while (is_valid()) {
                if (!read_to_buffer()) {
                    close();
                    break;
                }

                if (!write_from_buffer()) {
                    close();
                    break;
                }
            }
        }

        void socket_client::start()
        {
            thread_ = std::make_shared<thread>(&socket_client::run, this);
        }
    }
}