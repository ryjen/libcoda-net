#include "util.h"
#include "../../socket.h"
#include "../../uri.h"
#include "../protocol.h"
#include "jester.h"

namespace coda {
  namespace net {
    namespace jest {
      std::shared_ptr<arg_pair> split_arg(const std::string &arg,
                                          const std::string &delimiter) {
        auto pos = arg.find(delimiter);

        if (pos == std::string::npos) {
          return nullptr;
        }

        std::string key(arg.begin(), arg.begin() + pos);

        if (key.empty()) {
          return nullptr;
        }

        std::string value(arg.begin() + pos + delimiter.length(), arg.end());

        return std::make_shared<arg_pair>(key, value);
      }
      interactive::interactive(coda::net::jester *jest)
          : state_(std::bind(&interactive::get_method, this)),
            prompt_(std::bind(&interactive::ask_method, this)), jester_(jest) {}

      void interactive::ask_method() const {
        puts("HTTP Method?");
        printf("(Default GET, Current %s) ",
               http::method_names[jester_->method()]);
      }

      bool interactive::get_method() {
        std::string input;
        getline(std::cin, input);
        if (!input.empty()) {
          if (!jester_->set_method(input)) {
            puts("Invalid method, try again.");
            return false;
          }
        }
        state_ = std::bind(&interactive::get_headers, this);
        prompt_ = std::bind(&interactive::ask_headers, this);
        return true;
      }

      void interactive::ask_headers() const {
        puts("HTTP Headers? (End with a ~)");
      }

      bool interactive::get_headers() {
        std::string input;
        getline(std::cin, input);

        if (input == "~") {
          state_ = std::bind(&interactive::get_data, this);
          prompt_ = std::bind(&interactive::ask_data, this);
          return true;
        }

        auto pair = jest::split_arg(input, ":");

        if (!pair) {
          puts("invalid header format (key: value).  Try again.");
        } else {
          jester_->append_header(*pair);
        }

        return false;
      }

      void interactive::ask_data() const { puts("HTTP Data? (End with a ~)"); }

      bool interactive::get_data() {
        std::string input;
        getline(std::cin, input);

        if (input == "~") {
          state_ = nullptr;
          prompt_ = nullptr;
          return true;
        }
        auto pair = jest::split_arg(input, "=");

        if (!pair) {
          puts("Invalid data format (key=value). Try again.");
        } else {
          jester_->append_data(*pair);
        }
        return false;
      }

      void interactive::prompt() const {
        if (prompt_ != nullptr) {
          prompt_();
        }
      }

      void interactive::start() {
        prompt();

        while (state_ != nullptr) {
          if (state_()) {
            prompt();
          }
        }
      }
    } // namespace jest
  }   // namespace net
} // namespace coda