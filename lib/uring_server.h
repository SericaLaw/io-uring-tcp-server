#ifndef URING_SERVER_H
#define URING_SERVER_H
#include <functional>
#include <liburing.h>

#include "uring_channel.h"
const int QUEUE_SIZE{256};

class UringServer {
public:
    UringServer() = delete;
    explicit UringServer(int port) {
        io_uring_queue_init(QUEUE_SIZE, &_ring, 0);
        _listen(port);
    }
    ~UringServer() { io_uring_queue_exit(&_ring); }

    void start();
    std::function<void(UringChannel *)> connection_callback{nullptr};
    std::function<void(UringChannel *)> message_callback{nullptr};
    std::function<void(UringChannel *)> connection_closed_callback{nullptr};

private:
    void _listen(int port);
    void submit_accept(UringChannel *);
    void submit_read(UringChannel *);
    void submit_write(UringChannel *);
    int _listen_fd{-1};
    struct io_uring _ring{};
};


#endif //LINUX_IO_EXAMPLES_URING_SERVER_H
