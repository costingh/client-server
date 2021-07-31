# Client-Server Application written in c++

## Description
This app was build to show how a server and a client communicate through non-blocking sockets.

## Functionality
* Client
  - sends a request to server to server to connect
  - if the server accepts the request, the user receives a confirmation
  - will set a delay of 15s, in which, if no message is received from server, it will automatically disconnect
  - will response with a PONG message each time it receives a PING message from server
* Server
  - will create a vector with all the clients connected to it
  - after a new user connects to server, it will keep alive the connection (sending a PING message to the server)
  - will start monitoring connection performance such as: average delay between messages, how many PONG messages it has received in the last 10s
  - will set a timer of 15s, in which if it won't receive a PONG message from client, it will disconnect the client
  - it stores messages in a list and parses them, each message containing in it's first byte the type: CONNECT, PING, PONG, etc.

## Run locally
To run the app, open two visual studio instances and run the server.cpp file first on one instance, then the client.cpp on the other.
