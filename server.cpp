// Built-in library
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <cstring>
#include <string>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sys/mman.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <boost/algorithm/string/join.hpp>

// Reference library
#include "color.hpp"
#include "string.cc"

// Using namespace
using namespace std;
using namespace rang;
using namespace boost;

// Constants
#define BUFF_SIZE 2560
#define MICRO_TRANSLATOR_PORT 9990
#define MICRO_CURRENCY_PORT 9995
#define MICRO_VOTE_PORT 9999

struct micro_server_socket{
    int micro_socket;
    struct addrinfo *micro_info;
};

// Initialize the server socket
struct micro_server_socket micro_socket_initialize(int port) {
    struct addrinfo hints, *micro_info;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // for wildcard IP address
    hints.ai_protocol = IPPROTO_UDP;       // any protocol
    getaddrinfo(NULL, to_string(port).c_str(), &hints, &micro_info);    // create a socket

    // Connect with the micro server
    int micro_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Set timeout
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(micro_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    // Construct ret
    struct micro_server_socket ret;
    ret.micro_socket = micro_socket;
    ret.micro_info = micro_info;
    return ret;
}

// Initialize client socket with designated port
int client_socket_initialize(int port) {
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));
    client.sin_family = AF_INET;
    client.sin_port = htons(port);
    client.sin_addr.s_addr = htonl(INADDR_ANY);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) return -1;

    int client_bind = bind(client_socket, (struct sockaddr*)&client, sizeof(struct sockaddr_in));
    if (client_bind == -1) return -1;

    int client_listen = listen(client_socket, 5);
    if (client_listen == -1) return -1;

    return client_socket;
}

string server(string command) {
    // Split command string
    vector<string> args = StringExtension::split(command, ' ');
    if (args.size() < 2) {
        return "Error: Command invalid";
    }

    // Buffer
    char message_in[BUFF_SIZE];
    char message_out[BUFF_SIZE];
    bzero(message_in, BUFF_SIZE);
    bzero(message_out, BUFF_SIZE);
    int client_id = getpid();

    // Handle message_in, values
    vector<string> values_vector(args.begin() + 1, args.end());

    // Decide micro server
    string micro_server(args[0]);
    int port = 0;
    if (micro_server == "translate") {
        port = MICRO_TRANSLATOR_PORT;
    } else if (micro_server == "currency") {
        port = MICRO_CURRENCY_PORT;
    } else if (micro_server == "voting") {
        port = MICRO_VOTE_PORT;
        values_vector.push_back(to_string(client_id));
    } else {
        return "Error: Command invalid";
    }

    // Compose message_in
    string values(join(values_vector, " "));
    strcpy(message_in, values.c_str());

    // Connect to micro server & Send to micro server
    struct micro_server_socket ret = micro_socket_initialize(port);
    int micro_socket = ret.micro_socket;
    struct addrinfo* micro_info = ret.micro_info;
    size_t send_size = sendto(micro_socket, message_in, strlen(message_in), 0, micro_info->ai_addr, micro_info->ai_addrlen);
    cout << "Sent (" + micro_server + "): " << values << endl;

    // Recv from micro server & Close micro server socket
    size_t recv_size = recvfrom(micro_socket, message_out, BUFF_SIZE, 0, micro_info->ai_addr, &micro_info->ai_addrlen);
    if (recv_size == -1) {
        strcpy(message_out, "Error: micro server timeout");
    }
    close(micro_socket);
    cout << "Received (" + micro_server + "): " << message_out << endl;

    // Handle message_out
    string result(message_out);
    return result;
}


int main(){
    // Proxy <-> Client socket connection
    int client_socket = -1;
    int client_port = 9939;
    while (client_socket == -1) {
        client_port++;
        client_socket = client_socket_initialize(client_port);
    }

    cout << "CONNECT at PORT: " << style::bold << fg::green << client_port << style::reset << fg::reset << endl;
    cout << endl;

    // Accepting new socket connections by clients
    int c = sizeof(struct sockaddr_in);
    int request = 0;
    struct sockaddr_in browser;
    while (true) {
        int child_client_socket = accept(client_socket, (struct sockaddr*)&browser, (socklen_t*)&c);
        if (child_client_socket == -1) {
            cout << fg::red << "Proxy-client: accept() call failed." << fg::reset << endl;
            close(child_client_socket);
            exit(1);
        }

        request++;
        int request_no = request;

        // Fork new child process to handle the new connection
        int pid = fork();
        if (pid < 0) { // Child process creation failed
            cout << fg::red << "Proxy-client: fork() call failed." << fg::reset << endl;
            close(child_client_socket);
            exit(1);
        } else if (pid == 0) { // Child process
            char message_in[BUFF_SIZE];
            char message_out[BUFF_SIZE];

           // Keep receiving commands
            while (true) {
                bzero(message_in, BUFF_SIZE);
                bzero(message_out, BUFF_SIZE);

                // Recv from client
                int client_recv_bytes = recv(child_client_socket, message_in, BUFF_SIZE, 0);
                if (client_recv_bytes == -1) {
                    cout << fg::red << "Proxy-client: recv() call failed." << fg::reset << endl;
                }

                // Process command string
                string command(reinterpret_cast<char const*>(message_in));
                command = StringExtension::trim(command, "\n\r"); // Remove trailing new line characters
                cout << "Received (client): " << command << endl;

                // Run server
                string result = server(command);
                strcpy(message_out, result.c_str());

                // Send back to client
                int client_send = send(child_client_socket, message_out, strlen(message_out), 0);
                if (client_send < 0) continue;
                cout << "Sent (client): " << result << endl;
            }

            close(child_client_socket);
            exit(0);
        } else { // Parent process
            // Does nothing
        }
    }
}
