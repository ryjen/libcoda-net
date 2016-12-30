
#include <fstream>
#include <iostream>
#include <iterator>
#include "util.h"

namespace std
{
    std::istream &operator>>(std::istream &is, std::pair<std::string, std::string> &ps)
    {
        string line;
        is >> line;
        auto pos = line.find('=');
        if (pos != std::string::npos) {
            ps.first = line.substr(0, pos);
            ps.second = line.substr(pos + 1);
        }
        return is;
    }
    std::ostream &operator<<(std::ostream &os, const std::pair<std::string, std::string> &ps)
    {
        os << ps.first << '=' << ps.second;
        return os;
    }
}

namespace rj
{
    namespace net
    {
        namespace jest
        {
            std::string get_config_folder()
            {
                std::string path = getenv("HOME");
                path += PATH_SEP;
                path += ".jest";
                return path;
            }

            config_io::config_io(const std::string &path) : io_(path)
            {
                if (!io_.is_open()) {
                    io_.clear();
                    io_.open(path, std::ios::out);  // Create file.
                    io_.close();
                    io_.open(path);
                }
            }
            config_io::~config_io()
            {
                io_.close();
            }
            void config_io::read(config_type &config)
            {
                std::insert_iterator<config_type> mpsi(config, config.begin());

                const std::istream_iterator<config_entry> eos;
                std::istream_iterator<config_entry> its(io_);

                std::copy(its, eos, mpsi);
            }
            void config_io::write(const config_type &config)
            {
                std::copy(config.begin(), config.end(), std::ostream_iterator<config_entry>(io_, "\n"));
            }

            config::config(const std::string &name) : path_(get_config_folder())
            {
                path_ += PATH_SEP;
                path_ += name;
            }

            config &config::init()
            {
                config_io reader(path_);
                reader.read(values_);
                return *this;
            }

            std::string config::get(const std::string &key)
            {
                if (key.empty()) {
                    return key;
                }

                return values_[key];
            }

            config &config::put(const std::string &key, const std::string &value)
            {
                if (!key.empty()) {
                    values_[key] = value;
                }
                return *this;
            }

            void config::save()
            {
                config_io writer(path_);
                writer.write(values_);
            }
        }
    }
}