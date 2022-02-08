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

bool is_response_error(string response) {
    return response.rfind("Error: ", 0) == 0;
}

void error(string message) {
    cout << fg::red << message << fg::reset << endl;
}

string input(string line) {
    cout << line;
    string input;
    cin >> input;
    return input;
}

void output(string response, string line) {
    if (is_response_error(response)) {
        error(response);
    } else {
        cout << fg::green << line << fg::reset << endl;
    }
}

void press_to_continue() {
    cout << endl;
    cout << "Press any key to continue...";
    cin.ignore();
    cin.get();
    system("clear");
}

int server_socket_connect(string ip, string port) {
    // Create address struct
    struct sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(stoi(port));
    server.sin_addr.s_addr = inet_addr(ip.c_str());

    // Construct socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        return -1;
    }

    // Connect socket
    int server_connect = connect(server_socket, (struct sockaddr*)&server, sizeof(struct sockaddr_in));
    if (server_connect == -1) {
        return -1;
    }

    return server_socket;
}

string server_command(int server_socket, string command) {
    // Variables
    char message_in[BUFF_SIZE];
    char message_out[BUFF_SIZE];
    bzero(message_in, BUFF_SIZE);
    bzero(message_out, BUFF_SIZE);

    // Send user command
    strcpy(message_in, command.c_str());
    int client_send = send(server_socket, message_in, BUFF_SIZE, 0);
    if (client_send == -1) {
        error("Error: send() failed.");
        return "";
    }

    // Receive response
    int client_recv = recv(server_socket, message_out, BUFF_SIZE, 0);
    if (client_recv == -1) {
        error("Error: recv() failed.");
        return "";
    }
    
    string ret(message_out);
    return ret;
}

string action(map<int, map<string, string>> actions) {
    cout << "Select an action:" << endl;
    for (auto [key, action_info] : actions) {
        cout << key << ". " << action_info["text"] << endl;
    }

    cout << endl;
    cout << "Action: ";

    string action_no_raw;
    cin >> action_no_raw;

    if (!StringExtension::isInt(action_no_raw)) {
        return "";
    }

    int command_no = stoi(action_no_raw);
    if (!actions.count(command_no)) {
        return "";
    }

    map<string, string> action_info = actions[command_no];
    string action_code = action_info["code"];
    return action_code;
}

void translate(int server_socket) {
    system("clear");
    
    cout << "English-French Translator" << endl;
    string english = input("Enter an English word: ");
    string request = "translate " + english;
    string response = server_command(server_socket, request);
    output(response, "French translation: " + response);
    
    press_to_continue();
}

void currency(int server_socket) {
    system("clear");

    cout << "Currency Exchange" << endl;
    cout << "Available currencies are: CAD, USD, EUR, GBP, and BIT." << endl;

    string source_currency = input("What is your source currency? ");
    string destination_currency = input("What is your destination currency? ");
    string amount = input("What is the amount? ");

    string request = "currency " + amount + " " + source_currency + " " + destination_currency;
    string response = server_command(server_socket, request);
    output(response, "The amount converted is: " + response);

    press_to_continue();
}

string pad(string str, int size, bool right = true, char character = ' ') {
    if (size > str.size()) {
        if(right){
            str.insert(0, size - str.size(), character);
        }else{
            str.insert(str.size(), size - str.size(), character);
        }
    }
    return str;
}

void voting(int server_socket) {
    while (true) {
        system("clear");

        string votes_list = server_command(server_socket, "voting all");
        cout << "Voting System" << endl;
        cout << endl;

        cout << style::bold << pad("ID", 2, false) << "  " << pad("Name", 46, false) << style::reset << endl;
        cout << pad("", 50, true, '-') << endl;
        
        vector<string> votes_list_lines = StringExtension::split(votes_list, '\n');
        for(string line : votes_list_lines){
            vector<string> vote = StringExtension::split(line);
            
            vector<string> name_vector(vote.begin() + 1, vote.end());
            string name(join(name_vector, " "));
            
            cout << fg::yellow << pad(vote[0], 2) << fg::reset << "  " << pad(name, 46, false) << endl;
        }

        cout << endl;
        string vote_id = input("View by vote id (or enter return): ");

        if (vote_id == "return") {
            break;
        }

        while (true) {
            system("clear");
            string vote_content = server_command(server_socket, "voting view " + vote_id);
            if (is_response_error(vote_content)) {
                error(vote_content);
                press_to_continue();
                continue;
            }

            // Format vote content into user friendly display
            vector<string> vote_content_lines = StringExtension::split(vote_content, '\n');
            cout << style::bold << fg::cyan << vote_content_lines[1] << fg::reset << style::reset << endl;
            cout << "Start: " << vote_content_lines[2] << endl;
            cout << "End:   " << vote_content_lines[3] << endl;

            bool handle_candidates = false;
            for (int t = 4; t < vote_content_lines.size(); t++) {
                string line = StringExtension::trim(vote_content_lines[t], "\n");
                if (line == "") {
                    handle_candidates = true;
                    cout << endl;
                    cout << style::bold << pad("ID", 3, false) << "  " << pad("Name", 15, false) << "  " << pad("Votes", 6) << style::reset << endl;
                    cout << pad("", 28, true, '-') << endl;
                } else if (line == "voted") {
                    cout << fg::green << "You have voted." << fg::reset << endl;
                } else if (line == "vote-then-view") {
                    cout << "You can immediately view the vote result after you vote." << endl;
                } else if (line == "has-ended") {
                    cout << fg::magenta << "This vote has ended." << fg::reset << endl;
                } else if (handle_candidates) {
                    vector<string> candidate = StringExtension::split(line);

                    if(candidate.size() >= 2){
                        cout << fg::yellow << pad(candidate[0], 3) << fg::reset << "  ";
                        cout << pad(candidate[1], 15, false) << "  ";
                    }

                    if(candidate.size() >= 3){
                        cout << pad(candidate[2], 6);
                    }

                    cout << endl;
                } else {
                    cout << line;
                }
            }

            cout << endl;

            string candidate_id_string = input("The id of the candidate you want to vote for (or enter return): ");
            if(candidate_id_string == "return"){
                break;
            }

            // Check validation of candidate id
            if (!StringExtension::isInt(candidate_id_string)) {
                error("Error: Candidate id is invalid");
                press_to_continue();
                continue;
            }

            // Request for secure vote key
            string secure_key_string = server_command(server_socket, "voting key");

            if (is_response_error(secure_key_string)) {
                error(secure_key_string);
                press_to_continue();
                continue;
            } else if (!StringExtension::isInt(secure_key_string)) {
                error("Error: Secure key is invalid");
                press_to_continue();
                continue;
            }

            // Vote
            int secure_key = stoi(secure_key_string);
            int candidate_id = stoi(candidate_id_string);
            string voted = server_command(server_socket, "voting vote " + vote_id + " " + to_string(secure_key * candidate_id));

            if(is_response_error(voted)){
                error(voted);
                press_to_continue();
                continue;
            }

            output(voted, "You have voted " + voted);
            press_to_continue();
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <ip> <port>" << endl;
        return 1;
    }

    system("clear");

    string ip(argv[1]);
    string port(argv[2]);
    int server_socket = server_socket_connect(ip, port);
    if (server_socket == -1) {
        error("Error: server socket conenction failed");
        return 1;
    }

    map<int, map<string, string>> actions;
    actions[1] = { {"text", "Translate"}, {"code", "translate"} };
    actions[2] = { {"text", "Currency Exchange"}, {"code", "currency"} };
    actions[3] = { {"text", "Votings"}, {"code", "voting"} };
    actions[9] = { {"text", "Exit"}, {"code", "exit"} };


    // Loop for interaction
    while (true) {
        system("clear");
        
        string action_code = action(actions);
        if (action_code == "") {
            error("The input action is invalid.");
            press_to_continue();
            continue;
        }

        cout << endl;

        if (action_code == "translate") {
            translate(server_socket);
        } else if (action_code == "currency") {
            currency(server_socket);
        } else if (action_code == "voting") {
            voting(server_socket);
        } else if (action_code == "exit") {
            break;
        }
    }

    system("clear");
    close(server_socket);
}