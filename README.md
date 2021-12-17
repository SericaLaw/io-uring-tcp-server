# io-uring-tcp-server

This is an example TCP server framework based on `io_uring`, the most vogue Linux asynchronous I/O interface. To learn more about `io_uring`: https://kernel.dk/io_uring.pdf

## Prerequisites

- Linux kernel version >= 5.7 (If not work, please try higher version)
- [`liburing`](https://github.com/axboe/liburing)

## Quick Start
Build and install [`liburing`](https://github.com/axboe/liburing) from source:

```bash
git clone https://github.com/axboe/liburing

cd liburing

./configure
make
make install
```

Build the server:

```bash
mkdir build
cd build

cmake ..
make
```

Run and test the example echo server ...

```bash
$ ./echo_uring.out
connection from client 127.0.0.1:34994 established
received from client 127.0.0.1:34994 : hello, world!
connection from client 127.0.0.1:34994 closed
```

... with telnet:

```bash
$ telnet 127.0.0.1 8888
Trying 127.0.0.1...
Connected to 127.0.0.1.
Escape character is '^]'.
hello, world!
hello, world!
Connection closed by foreign host.
```

Feel free to modify the callback logic or write another server!

```c++
int main() {
    int port{8888};
    UringServer server{port};
    server.connection_callback = on_connection_established;
    server.message_callback = on_message;
    server.connection_closed_callback = on_connection_closed;
    server.start();
}
```