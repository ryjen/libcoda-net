rj_net
======

[![Build Status](http://img.shields.io/travis/ryjen/net/master.svg)](https://travis-ci.org/ryjen/net)
[![Coverage Status](http://img.shields.io/coveralls/ryjen/net/master.svg)](https://coveralls.io/github/ryjen/net?branch=master)
[![License](http://img.shields.io/:license-mit-blue.svg)](http://ryjen.mit-license.org)

A c++11 networking library.

A working example is [Yahtsee](http://github.com/ryjen/yahtsee).

Building
========

You can use [cmake](https://cmake.org) to generate for the build system of your choice.

```bash
mkdir debug; cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
make test
```

options supported are:

    -DENABLE_COVERAGE=OFF :   enable code coverage using lcov
    -DENABLE_MEMCHECK=OFF :   enable valgrind memory checking on tests
    -DWITH_CURL=ON        :   enable curl usage for http client
    -DWITH_SSL=ON         :   enable sockets with OpenSSL support
    -DWITH_URIPARSER=ON   :   enable uriparser library for parsing (otherwise will do its own)

Model
=====

A **socket** adds c++ interface to a system socket

A **buffered_socket** adds i/o buffering to a socket.

A **socket_listener** can be attached to a buffered_socket for i/o events.

A **socket_factory** implementation should create a new socket type for a server.

An **async_server** is a server that will run sockets in an i/o thread loop.

A **polling_server** is a server that executes i/o synchronously for each socket in intervals.

Examples
========

### Implement a factory and a listener

```c++
/**
 * create connections and listen to connection events
 */
class example_factory : public rj::net:socket_factory, public rj::net::buffered_socket_listener,
                        public enable_shared_from_this<example_factory>
{
public:
    /* creates a client on a new connection and adds a listener */
    socket_factory::socket_type create_socket(const socket_factory::server_type &server,
                                              SOCKET sock, const sockaddr_in &addr) {
            // create a new socket
            auto sock = std::make_shared<buffered_socket>(sock, addr);

            // sets the socket the same as the server blocking mode
            socket->set_non_blocking(server->is_non_blocking());

            // add this instance as a listener
            sock->add_listener(shared_from_this());

            return sock;
        }
  	}

    void on_did_read(const socket_type &sock) {
      // TODO: Perform input parsing here
      cout << "read from client socket" << endl;
    }

    void on_did_write(const socket_type &sock) {
      // TODO: Perform post i/o actions here
      cout << "wrote to client socket" << endl;
    }

    void on_connect(const socket_type &sock) {
      // TODO: Perform initialization here
      cout << "client socket connected" << endl;
    }
};
```

#### Run an async server in the background

```c++
int main() {
    /* create a factory */
    std::shared_ptr<example_factory> mySocketFactory = std::make_shared<example_factory>();

    /* create a server */
    rj::net::async_server example_server(1337, mySocketFactory);

    /* start running */
    example_server.start_in_background();

    /* wait till done */
    while(example_server.is_valid()) {
        sleep(1);
    }

    return 0;
}

```


#### Run a polling server in the foreground

```c++
int main() {
    /* create a factory */
    std::shared_ptr<example_factory> mySocketFactory = std::make_shared<example_factory>();

    /* create a server */
    rj::net::polling_server example_server(1337, mySocketFactory);

    /* start running */
    example_server.start();

    return 0;
}

```

Other
=====

##### http::client
 
 a very basic implementation of an HTTP client:

```c++

http::client client("api.somehost.com/version/resource/id");

// get some resource
client.get([](const http_response &response) {
     cout << response.code() << ": " << response << endl;
});
```

```c++

http::client client("https://api.somehost.com/version/resource/id");

map<string,string> data = {{"key1", "value1"}, {"key2", "value2"}};

client.add_header(http::CONTENT_TYPE, "application/json");

client.set_content(encoding::json().encode(data));

auto response = client.post().response();

cout << response.code() << ": " << response << endl;

```

##### jest

jest is a simple command line util for testing REST services.  It will remember your last request (headers,etc), leaving you free to just specify the path.

TODO
====

* more socket RFC implementations (SMTP, WebSockets)
* more testing
