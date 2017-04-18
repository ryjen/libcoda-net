#include "client.h"

namespace rj
{
    namespace net
    {
        namespace async
        {
            void default_client::run()
            {
                while (is_valid()) {
                    if (!write_from_buffer()) {
                        close();
                        break;
                    }

                    if (!is_valid()) {
                        break;
                    }

                    if (!read_to_buffer()) {
                        close();
                        break;
                    }
                }
            }
        }
    }
}
