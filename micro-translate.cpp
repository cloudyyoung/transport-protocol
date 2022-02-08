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
#define PORT 9990



int main(){
    map<string, string> dictionary;
    dictionary["hello"] = "bonjour";
    dictionary["goodbye"] = "an revoir";
    dictionary["yes"] = "oui";
    dictionary["no"] = "non";
    dictionary["love"] = "amour";
    dictionary["strawberry"] = "fraise";


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
        cout << "Received: " << message_in << endl;

        string english(message_in);
        english = StringExtension::trim(english, "\n\r");
        to_lower(english);

        string french;
        french = dictionary[english];
        french = StringExtension::capitalize(french);

        if(french == ""){
            strcpy(message_out, "Error: Word is not found in dictionary");
        }else{
            strcpy(message_out, french.c_str());
        }

        size_t send_size = sendto(micro_socket, message_out, strlen(message_out), 0, micro_info->ai_addr, micro_info->ai_addrlen);
        cout << "Sent:     " << message_out << endl;

        cout << endl;
    }
}
