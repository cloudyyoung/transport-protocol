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
#include <time.h>

// Reference library
#include "string.cc"

// Using namespace
using namespace std;
using namespace boost;

// Constants
#define BUFF_SIZE 2560
#define PORT 9999


// Candidates
struct Candidate {
    int candidate_id;
    string name;
    int votes;
};

// Vote
struct Vote {
    int vote_id;
    string name;
    bool vote_then_view;
    time_t start;
    time_t end;
    vector<Candidate> candidates;
};

// Global variable
vector<Vote> votes;
vector<pair<int, int>> clients_voted;
time_t timestamp;


string vote(string command) {
    map<string, int> req;
    req["all"] = 2;
    req["key"] = 2;
    req["view"] = 3;
    req["vote"] = 4;

    // Split command string
    vector<string> args = StringExtension::split(command, ' ');
    if (args.size() < 2) {
        return "Error: Command invalid";
    }
    
    // Variables
    string action(args[0]);

    // Action does not exist or argument amount is incorrect
    if (req[action] == 0 || req[action] != args.size()){
        return "Error: Command invalid";
    }

    // All the rest args have to be integers
    for (int t = 1; t < args.size(); t++) {
        if (!StringExtension::isInt(args[t])) {
            return "Error: Command invalid";
        }
    }

    // Client info
    int client_id = stoi(args.back());
    int client_key = client_id % 7 + 1;

    // List all the votes
    if (action == "all") {
        string result;
        for (Vote vote : votes) {
            result += to_string(vote.vote_id) + " " + vote.name + "\n";
        }
        return result;
    } else if (action == "key") {
        return to_string(client_key);
    }
    
    // Convert arguments
    int vote_id = stoi(args[1]);
    bool client_voted = (find(clients_voted.begin(), clients_voted.end(), pair<int, int>(client_id, vote_id)) != clients_voted.end());
    
    // Get vote
    Vote vote;
    vector<Vote>::iterator vote_iterator;
    bool find_vote = false;
    
    for (auto v = votes.begin(); v != votes.end(); v++) {
        if (v->vote_id == vote_id) {
            vote = (*v);
            vote_iterator = v;
            find_vote = true;
            break;
        }
    }

    // If vote is not found
    if (!find_vote) {
        return "Error: Vote is not found";
    }

    // For action
    if (action == "view") {
        if (args.size() != 3) {
            return "Error: Command invalid";
        }
        
        string result;
        char temp[32];
        bool has_ended = vote.end <= timestamp;

        strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S", localtime(&vote.start));
        string start(temp);
        
        strftime(temp, sizeof(temp), "%Y-%m-%d %H:%M:%S", localtime(&vote.end));
        string end(temp);

        result += to_string(vote.vote_id) + "\n";
        result += vote.name + "\n";
        result += start + "\n";
        result += end + "\n";
        
        
        if (vote.vote_then_view) {
            result += "vote-then-view\n";
        }

        if(has_ended){
            result += "has-ended\n";
        }
        
        if (client_voted) {
            result += "voted\n";
        }

        result += "\n";

        // Print candidates
        if ((client_voted && vote.vote_then_view) || has_ended) {
            // Print with votes, when client has voted and vote_then_view is enabled, or the vote has ended
            for(Candidate can : vote.candidates){
                result += to_string(can.candidate_id) + " " + can.name + " " + to_string(can.votes) + "\n";
            }
        }else{
            // Print without votes
            for(Candidate can : vote.candidates){
                result += to_string(can.candidate_id) + " " + can.name + "\n";
            }
        }

        return result;
    } else if (action == "vote") {
        if (client_voted) {
            return "Error: You have voted";
        }

        // Check start and end time
        if(timestamp < vote.start){
            return "Error: Vote has not started yet";
        }else if(vote.end < timestamp){
            return "Error: Vote has ended";
        }

        // Candidate id
        int candidate_id = stoi(args[2]) / client_key;

        // Vote and return
        for (auto c = vote_iterator->candidates.begin(); c != vote_iterator->candidates.end(); c++) {
            if (c->candidate_id == candidate_id) {
                c->votes++;
                clients_voted.push_back(pair<int, int>(client_id, vote.vote_id));

                return c->name;
            }
        }

        return "Error: Candidate is not found";
    }else{
        return "Error: Command invalid";
    }
}

void init_data() {
    // Candidates
    Candidate can1 = {12, "Valkyrae", 14944};
    Candidate can2 = {19, "Sukkuno", 11943};
    Candidate can3 = {23, "TinaKitten", 9304};
    Candidate can4 = {35, "Corpse", 10392};
    Candidate can5 = {46, "Fuslie", 4983};
    Candidate can6 = {53, "Pokimane"};
    Candidate can7 = {68, "Jacksepticeye"};
    Candidate can8 = {71, "Pewdiepie", 9804};
    Candidate can9 = {86, "Lilypichu", 4950};
    Candidate can10 = {92, "DisguisedToast", 6022};
    Candidate can11 = { 98, "Scarra", 9238 };
    Candidate can12 = { 104, "MichealReeves", 4952 };
    Candidate can13 = { 117, "Yvonnie", 6737 };
    Candidate can14 = { 143, "QuarterJade", 2334 };
    Candidate can15 = { 157, "iGumdrop", 3955 };
    Candidate can16 = { 179, "CodeMiko", 2055 };
    Candidate can17 = { 204, "itsRyanHiga", 13095 };
    Candidate can18 = { 223, "peterparkTV", 3959 };
    Candidate can19 = { 226, "xChocoBars", 9050 };
    Candidate can20 = { 238, "starsmitten", 8375 };
    Candidate can21 = { 250, "Marziapie", 14634 };
    Candidate can22 = { 263, "xQC", 4095 };

    // Vote 1 - Current (vote then view) (Wed Oct 20 2021 14:13:24 GMT+0000 ~ Sun Nov 21 2021 15:13:24 GMT+0000)
    Vote vote1 = { 1, "Current active vote with immediate result", true, 1634739204, 1637507604 };
    vote1.candidates.push_back(can1);
    vote1.candidates.push_back(can2);
    vote1.candidates.push_back(can3);
    vote1.candidates.push_back(can4);
    vote1.candidates.push_back(can5);
    vote1.candidates.push_back(can6);
    
    // Vote 2 - Current (Sat Sep 18 2021 14:13:24 GMT+0000 ~ Sat Dec 18 2021 15:13:24 GMT+0000)
    Vote vote2 = { 2, "Current active vote without immediate result", false, 1631974404, 1639840404 };
    vote2.candidates.push_back(can7);
    vote2.candidates.push_back(can8);
    vote2.candidates.push_back(can9);
    vote2.candidates.push_back(can10);
    vote2.candidates.push_back(can21);
    vote2.candidates.push_back(can22);

    // Vote 3 - Future (Thu Oct 20 2022 14:13:24 GMT+0000 ~ Tue Dec 20 2022 15:13:24 GMT+0000)
    Vote vote3 = { 3, "Future vote - has not started yet", false, 1666275204,  1671549204 };
    vote3.candidates.push_back(can11);
    vote3.candidates.push_back(can12);
    vote3.candidates.push_back(can13);
    vote3.candidates.push_back(can14);
    vote3.candidates.push_back(can15);

    // Vote 4 - Past (Wed Dec 18 2019 15:13:24 GMT+0000 ~ Tue Dec 31 2019 15:13:24 GMT+0000)
    Vote vote4 = { 4, "Past vote - has ended", false, 1576682004,  1577805204 };;
    vote4.candidates.push_back(can16);
    vote4.candidates.push_back(can17);
    vote4.candidates.push_back(can18);
    vote4.candidates.push_back(can19);
    vote4.candidates.push_back(can20);

    // Votes
    votes.push_back(vote1);
    votes.push_back(vote2);
    votes.push_back(vote3);
    votes.push_back(vote4);
}

int main() {
    // Current timestamp
    timestamp = std::time(0);

    // Initialize data
    init_data();

    // Socket
    struct addrinfo hints, * micro_info;
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE; // for wildcard IP address
    hints.ai_protocol = IPPROTO_UDP;       // any protocol
    getaddrinfo(NULL, to_string(PORT).c_str(), &hints, &micro_info);    // create a socket

    int micro_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (micro_socket == -1) {
        cout << "socket error" << endl;
        exit(1);
    }

    int micro_bind = bind(micro_socket, micro_info->ai_addr, micro_info->ai_addrlen);
    if (micro_bind == -1) {
        cout << "bind error" << endl;
        exit(1);
    }

    char message_in[BUFF_SIZE];
    char message_out[BUFF_SIZE];
    while (true) {
        bzero(message_in, BUFF_SIZE);
        bzero(message_out, BUFF_SIZE);

        size_t recv_size = recvfrom(micro_socket, message_in, BUFF_SIZE, 0, micro_info->ai_addr, &micro_info->ai_addrlen);

        // Process command string
        string command(reinterpret_cast<char const*>(message_in));
        command = StringExtension::trim(command, "\n\r"); // Remove trailing new line characters
        cout << "Received: " << command << endl;

        // Execute vote function
        string result = vote(command);
        
        // Send back result
        strcpy(message_out, result.c_str());
        size_t send_size = sendto(micro_socket, message_out, strlen(message_out), 0, micro_info->ai_addr, micro_info->ai_addrlen);
        cout << "Sent:     " << message_out << endl;

        cout << endl;
    }
}
