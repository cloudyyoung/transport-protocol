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
#include <map>
#include <unordered_map>
#include <sys/mman.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <boost/algorithm/string.hpp>

// Reference library
#include "string.cc"

// Using namespace
using namespace std;
using namespace boost;

// Constants
#define BUFF_SIZE 256
#define PORT 9995



int main(){
    map<pair<string, string>, double> dictionary;

    // CAD
    dictionary[pair<string, string>("CAD", "USD")] = 0.79;
    dictionary[pair<string, string>("CAD", "EUR")] = 0.69;
    dictionary[pair<string, string>("CAD", "GBP")] = 0.58;
    dictionary[pair<string, string>("CAD", "BIT")] = 0.000016;
    
    // USD
    dictionary[pair<string, string>("USD", "CAD")] = 1.22;
    dictionary[pair<string, string>("USD", "EUR")] = 0.85;
    dictionary[pair<string, string>("USD", "GBP")] = 0.72;
    dictionary[pair<string, string>("USD", "BIT")] = 0.000015;
    
    // EUR
    dictionary[pair<string, string>("EUR", "CAD")] = 1.42;
    dictionary[pair<string, string>("EUR", "USD")] = 1.15;
    dictionary[pair<string, string>("EUR", "GBP")] = 0.83;
    dictionary[pair<string, string>("EUR", "BIT")] = 0.000018;
    
    // GBP
    dictionary[pair<string, string>("GBP", "CAD")] = 1.69;
    dictionary[pair<string, string>("GBP", "USD")] = 1.37;
    dictionary[pair<string, string>("GBP", "EUR")] = 1.17;
    dictionary[pair<string, string>("GBP", "BIT")] = 0.000029;
    
    // BIT
    dictionary[pair<string, string>("BIT", "CAD")] = 80258.84;
    dictionary[pair<string, string>("BIT", "USD")] = 65133.00;
    dictionary[pair<string, string>("BIT", "EUR")] = 55897.47;
    dictionary[pair<string, string>("BIT", "GBP")] = 47145.55;


    struct addrinfo hints, *micro_info;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // for wildcard IP address
    hints.ai_protocol = IPPROTO_UDP;       // any protocol
    getaddrinfo(NULL, to_string(PORT).c_str(), &hints, &micro_info);    // create a socket

    int micro_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(micro_socket == -1){
        cout << "socket error" << endl;
        exit(1);
    }

    int micro_bind = bind(micro_socket, micro_info->ai_addr, micro_info->ai_addrlen);
    if(micro_bind == -1){
        cout << "bind error" << endl;
        exit(1);
    }

    cout << endl;

    char message_in[BUFF_SIZE];
    char message_out[BUFF_SIZE];
    while(true){
        bzero(message_in, BUFF_SIZE);
        bzero(message_out, BUFF_SIZE);

        size_t recv_size = recvfrom(micro_socket, message_in, BUFF_SIZE, 0, micro_info->ai_addr, &micro_info->ai_addrlen);

        // Process command string
        string command(reinterpret_cast<char const*>(message_in));
        command = StringExtension::trim(command, "\n\r"); // Remove trailing new line characters
        cout << "Received: " << command << endl;

        // Split command string
        vector<string> args = StringExtension::split(command, ' ');
        if (args.size() < 3 || !StringExtension::isFloat(args[0])) {
            // Size of the command is invalid, the command is invalid
            strcpy(message_out, "Error: Command invalid");
            send(micro_socket, message_out, strlen(message_out), 0);
            cout << "Error: Command invalid" << endl;
            continue;
        }

        double source_amount = stod(args[0]);
        string source_currency(args[1]);
        string destination_currency(args[2]);

        double rate = dictionary[pair<string, string>(source_currency, destination_currency)];
        double destination_amount = source_amount * rate;

        cout << "Result:   " << source_amount << " " << source_currency << " -> " << destination_amount << " " << destination_currency << endl;

        if(rate == 0.0){
            strcpy(message_out, "Error: Currency is not found");
        }else{
            string result = to_string(destination_amount);
            strcpy(message_out, result.c_str());
        }

        // Send back result
        size_t send_size = sendto(micro_socket, message_out, strlen(message_out), 0, micro_info->ai_addr, micro_info->ai_addrlen);
        cout << "Sent:     " << message_out << endl;

        cout << endl;
    }
}
