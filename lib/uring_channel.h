#ifndef URING_CHANNEL_H
#define URING_CHANNEL_H

#include <functional>
#include <string>
#include <string_view>
#include <netinet/in.h>
#include <arpa/inet.h>

enum channel_type {
    INVALID,
    ACCEPT,
    READ,
    WRITE,
};

const int BUFFER_SIZE{8192};

class UringChannel {
public:
    channel_type type{INVALID};
    int fd{-1};
    sockaddr_in client_addr{};
    socklen_t client_addr_len{};
    unsigned int buffer_size{BUFFER_SIZE};
    char buffer[BUFFER_SIZE]{};
    std::function<void(UringChannel *)> send_this{nullptr};

    string_view buffer_head_line_view() {
        size_t size = 0;
        for (size_t i = 0; i < buffer_size; i++) {
            size++;
            if (buffer[i] == '\n') {
                break;
            }
        }
        return {buffer, size};
    }

    void send(std::string &msg) {
        size_t n = msg.size();
        for (buffer_size = 0; buffer_size < BUFFER_SIZE; buffer_size++) {
            if (buffer_size == n) {
                break;
            }

            buffer[buffer_size] = msg[buffer_size];
        }

        send_this(this);
    }

    string client_address() {
        return string(inet_ntoa(client_addr.sin_addr)) + ":" + to_string(ntohs(client_addr.sin_port));
    }
};

#endif //URING_CHANNEL_H
