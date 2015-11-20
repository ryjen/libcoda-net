libarg3net
==========

[![Build Status](https://travis-ci.org/c0der78/arg3net.svg?branch=master)](https://travis-ci.org/c0der78/arg3net)

useful code for network related activities

[View Testing Code Coverage](http://htmlpreview.github.com/?https://github.com/c0der78/arg3net/blob/master/coverage/index.html)

A good working example is [Yahtsee](http://github.com/c0der78/yahtsee).

Building
========

I use [autotools](http://en.wikipedia.org/wiki/GNU_build_system).

```bash
autoreconf

./configure

make
```

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

funky_factory funkyFactory;

arg3::net::socket_server funky_server(1337, &funkyFactory);

funky_server.start(); 
```

