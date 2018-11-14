
#include "../../socket.h"
#include "../../uri.h"
#include "config.h"
#include "jester.h"
#include "uri.h"
#include "util.h"
#include <sys/stat.h>

#include "../../exception.h"

using namespace coda::net;

int main(int argc, char *argv[]) {
  std::string dir = jest::get_config_folder();
  coda::net::jester jester;
  jest::config history("history");

  int err = mkdir(dir.c_str(), 0777);

  if (err == -1 && errno != EEXIST) {
    perror("unable to create config directory");
    return EXIT_FAILURE;
  }

  history.init();

  jester.set_method(history.get("method"));
  jester.set_uri(history.get("uri"));

  if (jester.parse_options(argc, argv)) {
    return EXIT_FAILURE;
  }

  if (jester.is_interactive()) {
    jest::interactive prompter(&jester);
    prompter.start();
  }

  if (!jester.validate()) {
    jester.syntax(argv[0]);
    puts("For first time usage, please specify a base uri with -u.");
    return EXIT_FAILURE;
  }

  history.put("method", http::method_names[jester.method()]);
  history.put("uri", jester.uri());
  history.save();

  try {
    auto response =
        jester.execute((argc > optind && argv[optind]) ? argv[optind] : "");

    std::cout << "Response:" << std::endl;

    std::cout << response.content() << std::endl;

    std::cout << "Debug:" << std::endl;

    std::cout << response.full_response() << std::endl;

    return response.code() == http::OK;
  } catch (const socket_exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
}
