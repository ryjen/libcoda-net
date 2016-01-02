libarg3net
==========

[![Build Status](http://img.shields.io/travis/ryjen/arg3net.svg)](https://travis-ci.org/ryjen/arg3net)
[![Coverage Status](https://coveralls.io/repos/ryjen/arg3net/badge.svg?branch=master&service=github)](https://coveralls.io/github/ryjen/arg3net?branch=master)
[![License](http://img.shields.io/:license-mit-blue.svg)](http://ryjen.mit-license.org)

A c++11 networking library.

A good working example is [Yahtsee](http://github.com/ryjen/yahtsee).

Building
========

You can use [cmake](https://cmake.org) to generate for the build system of your choice.

```bash
mkdir debug; cd debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
make test
```

for homebrew you can add the install prefix:

```bash
mkdir release; cd release
cmake $(cd..;brew diy --version=0.5.0) -DCMAKE_BUILD_TYPE=Release ..
make
make install
brew link arg3net
```

options supported are:

    -DCODE_COVERAGE=ON   :   enable code coverage using lcov
    -DMEMORY_CHECK=ON    :   enable valgrind memory checking on tests
    -DWITH_CURL=ON       :   enable curl usage for http client
    -DWITH_SSL=ON        :   enable openssl usage for non-curl http client

Examples
========

*Simple example*

```c++
// a client listener
class example_listener : public arg3::net::buffered_socket_listener
{
public:
    void on_will_read(buffered_socket *sock) 
    {
    	cout << "going to read from client socket" << endl;
  	}

    void on_did_read(buffered_socket *sock) 
    {
    	cout << "read from client socket" << endl;
    }

    void on_will_write(buffered_socket *sock) 
    {
    	cout << "going to write to client socket" << endl;
    }

    void on_did_write(buffered_socket *sock)
    {
    	cout << "wrote to client socket" << endl;
    }

    void on_connect(buffered_socket *sock) 
    {
    	cout << "client socket connected" << endl;
    }

    void on_close(buffered_socket *sock) 
    {
    	cout << "client socket closed" << endl;
    }
};

class example_factory : public arg3::net:socket_factory
{
private:
	example_listener listener_;
public:
    /* creates a client on a new connection and adds our listener */
    std::shared_ptr<buffered_socket> create_socket(socket_server *server, SOCKET sock, const sockaddr_in &addr) 
    {
    	auto connection = std::make_shared<buffered_socket>(sock, addr);

    	connection->add_listener(&listener_);

    	return connection;
	}
};

example_factory mySocketFactory;

arg3::net::socket_server example_server(1337, &mySocketFactory);

example_server.start(); 
```

and the HTTP/Rest client looks like this:

```c++

http_client api("api.somehost.com");

// get some resource
api.get("version/resource/id");

auto response = api.response();

cout << response.code() << ": " << response << endl;

// post some resource
api.post("version/resource")

// etc
```

