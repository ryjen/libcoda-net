#include "jester.h"
#include <getopt.h>
#include "../../encoders.h"
#include "../client.h"
#include "util.h"

namespace rj
{
    namespace net
    {
        jester::jester() : method_(http::GET), interactive_(false)
        {
        }

        jester &jester::set_uri(const std::string &value)
        {
            uri_ = rj::net::uri(value);
            return *this;
        }
        jester &jester::set_method(http::method value)
        {
            method_ = value;
            return *this;
        }
        bool jester::set_method(const std::string &value)
        {
            for (int i = 0; i < (sizeof(http::method_names) / sizeof(http::method_names[0])); i++) {
                if (value == http::method_names[i]) {
                    method_ = static_cast<http::method>(i);
                    return true;
                }
            }
            return false;
        }

        jester &jester::set_interactive(bool value)
        {
            interactive_ = value;
            return *this;
        }
        bool jester::is_interactive() const
        {
            return interactive_;
        }

        jester &jester::append_data(const std::string &key, const std::string &value)
        {
            data_[key] = value;
            content_.clear();
            return *this;
        }
        jester &jester::append_data(const std::pair<std::string, std::string> &value)
        {
            return append_data(value.first, value.second);
        }
        jester &jester::append_header(const std::string &key, const std::string &value)
        {
            headers_[key] = value;
            return *this;
        }
        jester &jester::append_header(const std::pair<std::string, std::string> &value)
        {
            return append_header(value.first, value.second);
        }
        jester &jester::set_content(const std::string &value)
        {
            content_ = value;
            data_.clear();
            return *this;
        }
        http::method jester::method() const
        {
            return method_;
        }

        uri jester::uri() const
        {
            return uri_;
        }

        void jester::syntax(const char *bin) const
        {
            printf("Syntax: %s\n\n", bin);
            puts(
                "\t[path]\trest path to test.  if a uri option has been specified at least once, it will append to "
                "its value.");
            puts("\nOptions:");
            puts("\t-u\t\tspecify the uri or base url.");
            puts("\t-m [method]\tspecify the method");
            puts("\t-d [data]\tadd some data to the form in key=value format");
            puts("\t-i\t\tenable interactive mode to prompt for inputs");
            puts("\t-h [header]\tadd a header to the request in key:value format");
            puts("\n");
        }

        int jester::parse_options(int argc, char *argv[])
        {
            static const struct option longopts[] = {{"method", required_argument, NULL, 'm'},
                                                     {"uri", required_argument, NULL, 'u'},
                                                     {"data", required_argument, NULL, 'd'},
                                                     {"interactive", no_argument, NULL, 'i'},
                                                     {"content", required_argument, NULL, 'c'},
                                                     {"header", required_argument, NULL, 'h'},
                                                     {NULL, 0, NULL, 0}};
            int opt;
            std::shared_ptr<jest::arg_pair> pair;

            while ((opt = getopt_long(argc, argv, "u:m:d:ih:", longopts, NULL)) != -1) {
                switch (opt) {
                    case 'u':
                        set_uri(optarg);
                        break;
                    case 'm':
                        if (!set_method(optarg)) {
                            printf("Unknown HTTP method %s\n", optarg);
                            return EXIT_FAILURE;
                        }
                        break;
                    case 'd':
                        pair = jest::split_arg(optarg, "=");
                        if (!pair) {
                            printf("Data should be in key=value format.");
                            return EXIT_FAILURE;
                        }
                        append_data(*pair);
                        break;
                    case 'c':
                        set_content(optarg);
                        break;
                    case 'i':
                        set_interactive(true);
                        break;
                    case 'h':
                        pair = jest::split_arg(optarg, ":");
                        if (!pair) {
                            printf("header should be in key:value format.");
                            return EXIT_FAILURE;
                        }
                        append_header(*pair);
                        break;
                    default:
                        syntax(argv[0]);
                        return EXIT_FAILURE;
                }
            }
            return EXIT_SUCCESS;
        }

        bool jester::validate() const
        {
            return uri_.is_valid();
        }

        http::response jester::execute(const std::string &path) const
        {
            http::client client(uri_);

            client.add_headers(headers_);

            jsonencoder encoder;


            if (!data_.empty()) {
                client.set_content(encoder.encode(data_));
            } else if (!content_.empty()) {
                client.set_content(encoder.encode(content_));
            }

            client.request(method_, path);

            return client.response();
        }
    }
}