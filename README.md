# Transport Protocols

## Implementations
All the parts of the assignment, including the extra feature, are implemented:
1. The indirection server
2. The client
3. Translator and Currency conversion micro server
4. The voting system, with universal time

## Compile

Run the following code to compile all micro servers, indirection server and client:
```
g++ micro-translate.cpp -o micro-translate.o
g++ micro-currency.cpp -o micro-currency.o
g++ micro-voting.cpp -o micro-voting.o

g++ server.cpp -o server.o
g++ client.cpp -o client.o
```

## Run

First, launch all three micro servers:

```
./micro-translate.o
./micro-currency.o
./micro-voting.o
```

Then, run the indirection server and the client:
```
./server.o
./client.o <ip of server> <port of server>
```

## Testings
The following tests are done on `linux.cpsc.ucalgary.ca` server and all worked:
1. Launch client and server + micro-servers on two different linux servers.
2. Successfully make connections among the above.
3. Test translator and currency functionality
4. Test voting system, 4 different votings for 4 different scenarios

## Notes

All the requirements in the assignment are implemented. Some features are implemented but in a slightly different way because some aspects are not specified, so they'll be explained below:

1. The client program `client.cpp` is a very very user friendly command line interface. User does not input a raw command, instead, user will be asked questions and expcted to give response. The program will then use the user inputs to compose the command that server would understand. 
    * When user is asked `Enter an English word: ` and input is `hello`, the command `translate hello` will be sent to the server
    * The process would look like: `[client] translate hello` -> `[server] hello` -> `[Translate]`
2. The server program `server.cpp` will receive commands in a format of `<micro server name> <value> <more value> <...>`. For example: `translate hello` or `voting view 1`. The program will read the micro server name, and decide which micro server to communicate. The program will take away the micro server name, and pass the rest of the values to the requested micro server. 
3. The micro servers `micro-translate.cpp`, `micro-currency.cpp` and `micro-voting.cpp` will receive commands in a format of `<value> <more value> <...>`. One exception for `micro-voting.cpp`, it will receive `<value> <more value> <...> <client id>`.
4. Voting System
    * The client id is generated and sent by the indirection server (it's actually just the fork pid, but the uniqueness is all it needs)
        * The process would look like: `[Client] voting view 1` -> `[Server] view 1 493755` -> `[Voting]`
    * A client cannot vote for the same voting twice.
    * The client id is needed so that the micro server can save the client id for a specific vote, and next time prevents the same client id to vote for the same voting
    * Each client also has their own secure voting key (the value is just `client_id % 7 + 1`, plus one to prevent `0`)
    * There are 4 votings and each has different candidates (they are all streamers or content creators)
    * The 4 votings are for testing different scenarios (for extra features: universal time): 
        * Currently active voting and can request result immediately after vote
        * Currently active voting but can only view result after the voting has ended
        * Voting that happened in the past (ended)
        * Voting that is going to happen in the future (not started)
    * When viewing a voting, the result will be requested **automatically** when it **can be**, so that client do not need to mannually request.
        * The two cases where the result will be shown: either the client has voted and the "immadiately show result after vote" is enabled for that voting, or the voting is ended.
    