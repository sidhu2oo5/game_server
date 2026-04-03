# 🚀 Event-Driven Multiplayer Game Server (C + epoll)

A high-performance **client–server multiplayer application** built in **C**, using **TCP sockets** and **Linux epoll** for efficient event-driven I/O.

This project demonstrates low-level systems programming, networking, and real-time communication by implementing a **two-player game server with room management and matchmaking logic**.

---

## 📌 Features

* ⚡ **Event-driven architecture** using Linux `epoll`
* 🌐 **TCP client-server communication**
* 🧩 **Multi-room system** (supports multiple concurrent games)
* 🎮 Real-time **two-player gameplay**
* 🔁 **Rematch system**
* 🧠 Custom **binary protocol design**
* 📡 Reliable networking with **partial send/receive handling**
* 🧱 Modular code structure (network, game, server, client)

---

## 🧠 Architecture Overview

```
Client  <----TCP---->  Server (epoll-based event loop)
                         │
                         ├── Room Management
                         ├── Game Logic
                         └── Protocol Handling
```

* The server uses **epoll** to handle multiple clients efficiently.
* Each room supports **exactly 2 players**.
* Communication is handled via a **custom protocol (`data` struct)**.

---

## 📂 Project Structure

```
.
├── client.c          # Client entry point
├── server.c          # Server event loop
├── network.c         # Socket + send/receive logic
├── epoll_utils.c     # epoll helper functions
├── game.c            # Game logic (Rock-Paper-Scissors)
├── error_handle.c    # Input validation
├── protocol.h        # Message protocol definition
├── include/          # Header files
└── README.md
```

---

## ⚙️ Technologies Used

* **C (C99)**
* **Linux System Programming**
* **TCP/IP Sockets**
* **epoll (I/O multiplexing)**

---

## 🛠️ Build Instructions

```bash
make
```

---

## ▶️ Running the Application

### Start Server

```bash
./bin/gameserver
```

### Start Client

```bash
./bin/gameclient
```

---

## 🎮 Gameplay Flow

1. Client connects to server
2. Player:

   * Creates a room OR joins one
3. Two players enter a room
4. Players mark ready → game starts
5. Both submit moves (Rock / Paper / Scissors)
6. Server computes result and sends outcome
7. Players can:

   * Rematch
   * Leave room

---

## ⚡ Key Highlights

* Uses **epoll instead of threads** → scalable & efficient
* Handles **partial TCP sends/receives correctly**
* Implements a **state machine-based protocol**
* Clean **modular separation of concerns**

---

## ⚠️ Limitations

* CLI-based UI (basic user experience)
* Localhost configuration 

---

## 🚀 Future Improvements

* ⏱️ Timeout handling for inactive players
* 🧾 Player usernames & scoreboard
* 💬 In-game chat
* 📊 Performance benchmarking (load testing)

---

## 📈 Learning Outcomes

This project demonstrates:

* Systems-level programming in C
* Event-driven server design
* Socket programming and networking fundamentals
* Designing scalable and modular backend systems

---
