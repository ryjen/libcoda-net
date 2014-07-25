libarg3net
==========

useful code for network related activities

[View Testing Code Coverage](http://htmlpreview.github.com/?https://github.com/c0der78/arg3net/blob/master/coverage/index.html)

Building
========

I use [autotools](http://en.wikipedia.org/wiki/GNU_build_system).

```bash
./configure --prefix=/usr/local

make
```

Coding Style
============

- class/struct/method names are all lower case with underscores separating words
- non public members are camel case with and underscore at end of the name
- macros, enums and constants are all upper case with underscores seperating words
- braces on a new line


Examples
========

*Simple example*

```c++
// a client listener
class funky_socket_listener : public arg3::net::buffered_socket_listener
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

class funky_factory : public arg3::net:socket_factory
{
private:
	funky_socket_listener clientListener_;
public:
    /* creates a client on a new connection and adds our listener */
    std::shared_ptr<buffered_socket> create_socket(socket_server *server, SOCKET sock, const sockaddr_in &addr) 
    {
    	auto connection = std::make_shared<buffered_socket>(sock, addr);

    	connection->add_listener(&clientListener_);

    	return connection;
	}
};

funky_factory funkyFactory;

arg3::net::socket_server funky_server(1337, &funkyFactory);

funky_server.start(); // run in thread, use loop() for single thread
```

