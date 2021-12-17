#include <iostream>
#include <string>

using namespace std;

#include "uring_server.h"

void on_connection_established(UringChannel *channel) {
    cout << "connection from client " << channel->client_address() << " established\n";
}

void on_message(UringChannel *channel) {
    string line = string{channel->buffer_head_line_view()};
    cout << "received from client " << channel->client_address() << " : " << line;

    channel->send(line);
}

void on_connection_closed(UringChannel *channel) {
    cout << "connection from client " << channel->client_address() << " closed\n";
}

int main() {
    int port{8888};
    UringServer server{port};
    server.connection_callback = on_connection_established;
    server.message_callback = on_message;
    server.connection_closed_callback = on_connection_closed;
    server.start();
}