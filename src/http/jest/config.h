#ifndef RJ_NET_JEST_CONFIG_H
#define RJ_NET_JEST_CONFIG_H

#include <fstream>
#include <map>
#include <string>

namespace std
{
    std::istream &operator>>(std::istream &is, std::pair<std::string, std::string> &ps);
    std::ostream &operator<<(std::ostream &os, const std::pair<std::string, std::string> &ps);
}

namespace rj
{
    namespace net
    {
        namespace jest
        {
            std::string get_config_folder();

            typedef std::map<std::string, std::string> config_type;
            typedef std::pair<std::string, std::string> config_entry;

            class config_io
            {
               protected:
                std::fstream io_;

               public:
                config_io(const std::string &path);
                virtual ~config_io();
                void read(config_type &config);
                void write(const config_type &config);
            };

            class config
            {
               private:
                config_type values_;
                std::string path_;

               public:
                config(const std::string &name);

                config &init();

                std::string get(const std::string &key);

                config &put(const std::string &key, const std::string &value);

                void save();
            };
        }
    }
}

#endif
