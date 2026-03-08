Multiplayer Event-Driven Server
  A client–server application written in C that enables real-time multiplayer interaction using TCP sockets and Linux epoll.
The server uses an event-driven architecture to efficiently handle multiple concurrent client connections and manage multiple independent two-player rooms.

Features
  ->Event-driven server using Linux epoll
  ->TCP client–server communication
  ->Multi-room two-player session management
  ->Custom message protocol for structured client-server communication
  ->Reliable networking layer with partial send/receive handling
  ->Support for room creation, joining, gameplay, rematch, and leaving rooms
  ->Modular design separating networking, event handling, and game logic

Workflow
  ->Clients connect to the server via TCP.
  ->The server monitors connections using epoll.
  ->Players can create or join rooms.
  ->Each room supports two players interacting in real-time.
  ->The server processes moves and sends results to both players

Project Structure

client.c          Client entry point
server.c          Server event loop and request handling
network.c         Socket utilities and send/receive logic
epoll_utils.c     epoll helper functions
game.c            Game logic implementation
error_handle.c    Input validation utilities

Technologies Used
  ->C
  ->Linux
  ->TCP/IP Socket Programming
  ->epoll (I/O multiplexing)

Build Instructions
  make
Running the Application
  ./bin/gameserver
  ./bin/gameclient

Possible Improvements
  ->Automatic matchmaking system
  ->Timeout handling for inactive players
