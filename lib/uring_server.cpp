#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <csignal>
#include <cerrno>
#include <iostream>

using namespace std;

#include "uring_server.h"

void UringServer::_listen(int port) {
    if (_listen_fd != -1) {
        cerr << "TcpListener already listening with listen_fd = " << _listen_fd << endl;
        return;
    }

    _listen_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    int on = 1;
    setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    int err = bind(_listen_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if (err < 0) {
        cerr << "bind error: " << strerror(errno) << endl;
        exit(1);
    }

    err = ::listen(_listen_fd, 1024);
    if (err < 0) {
        cerr << "listen error: " << strerror(errno) << endl;
        exit(1);
    }

    signal(SIGPIPE, SIG_IGN);
}

void UringServer::start() {
    struct io_uring_cqe *cqe;
    auto acceptor = new UringChannel;
    acceptor->fd = _listen_fd;
    acceptor->client_addr_len = sizeof(sockaddr_storage);

    submit_accept(acceptor);
    while (true) {
        int err = io_uring_wait_cqe(&_ring, &cqe);
        if (err < 0) {
            cerr << "io_uring_wait_cqe error: " << strerror(errno) << endl;
            exit(1);
        }

        auto *channel = (UringChannel *) cqe->user_data;
        if (cqe->res < 0) {
            cerr << "request failed: " << strerror(-cqe->res) << " for event " << channel->type << endl;
            exit(1);
        }

        auto send_this = [this](auto &&p1) { submit_write(std::forward<decltype(p1)>(p1)); };
        if (channel->type == ACCEPT) {
            auto *read_channel = new UringChannel;
            read_channel->type = READ;
            read_channel->fd = cqe->res;
            read_channel->client_addr = acceptor->client_addr;
            read_channel->client_addr_len = acceptor->client_addr_len;

            if (connection_callback != nullptr) {
                connection_callback(read_channel);
            }

            submit_accept(acceptor);
            submit_read(read_channel);
        } else if (channel->type == READ) {
            if (!cqe->res) {
                cerr << "empty request read\n";
                exit(1);
            }

            if (message_callback != nullptr) {
                channel->send_this = send_this;
                message_callback(channel);
            }
        } else if (channel->type == WRITE) {
            ::close(channel->fd);

            if (connection_closed_callback != nullptr) {
                connection_closed_callback(channel);
            }

            delete channel;
        } else {
            cerr << "invalid channel type\n";
            exit(1);
        }
        io_uring_cqe_seen(&_ring, cqe);
    }
}

void UringServer::submit_accept(UringChannel *accept_channel) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
    accept_channel->type = ACCEPT;
    io_uring_prep_accept(sqe, accept_channel->fd, (struct sockaddr *) &accept_channel->client_addr,
                         &accept_channel->client_addr_len, 0);
    io_uring_sqe_set_data(sqe, accept_channel);
    io_uring_submit(&_ring);
}

void UringServer::submit_read(UringChannel *read_channel) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
    read_channel->type = READ;
    io_uring_prep_read(sqe, read_channel->fd, &read_channel->buffer, read_channel->buffer_size, 0);
    io_uring_sqe_set_data(sqe, read_channel);
    io_uring_submit(&_ring);
}

void UringServer::submit_write(UringChannel *channel) {
    struct io_uring_sqe *sqe = io_uring_get_sqe(&_ring);
    channel->type = WRITE;
    io_uring_prep_write(sqe, channel->fd, &channel->buffer, channel->buffer_size, 0);
    io_uring_sqe_set_data(sqe, channel);
    io_uring_submit(&_ring);
}
