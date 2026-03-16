# IPortbook - Progetto Sistemi di Elaborazione e Trasmissione dell'Informazione 2025/2026

This repository contains the C-based client-server messaging application built for the "IPortbook" SETI 2025/2026 university project. The system simulates a social network environment where clients can register, establish friendships, and exchange messages.

It has been developed with strict adherence to the provided project specifications, ensuring robust network communication, exact message formatting, and compatibility with standard laboratory environments.

## 📌 Strict Specification Compliance

As requested by the assignment guidelines, this project prioritizes protocol accuracy and backend logic over visual presentation:
* **No GUI:** The application is entirely command-line based, featuring a required "verbose" mode (`-v`) to print raw protocol exchanges and debug information during the oral exam.
* **Robustness & Limits:** The server safely handles concurrent connections up to a hard limit of 100 registered clients, and is built to gracefully handle malformed packets without crashing.
* **Exact TCP Formatting:** All TCP communications strictly utilize 5-byte command headers (e.g., `REGIS`, `CONSU`, `MESS?`) and are correctly terminated with the 3-byte `+++` (ASCII 43) string.
* **Exact UDP Formatting:** Push notifications are precisely 3 bytes long in the format `[YXX]`, where `Y` is the stream code and `XX` is the little-endian hexadecimal count of pending streams.
See specifications file: progetto-seti25.pdf

## ⚙️ Core Features

* **Authentication:** Clients register and connect using a strict 8-character alphanumeric ID, a local UDP port for notifications, and a numeric password (0-65535).
* **Social Graph:** Clients can send friend requests (`FRIE?`), which are queued as streams for the target user to accept (`OKIRF`) or reject (`NOKRF`).
* **Messaging & Flooding:**
  * Direct messages (`MESS?`) can be sent to confirmed friends (max 200 characters).
  * A flood feature (`FLOO?`) recursively broadcasts a message to all friends, and friends-of-friends.
* **Stream Consultation:** The server acts as a queue. When a client receives a UDP notification, they use the `CONSU` command to pull and consume pending friend requests or messages from the server one by one.

## 🏗️ Architecture

The codebase is organized into clean abstraction layers to separate logic from raw infrastructure:
* **Application Layer (`apps/`)**: Contains `clientMain.c` and `serverMain.c`. Handles the CLI menus, user input, and the core threaded connection loops.
* **Logic Layer (`src/IPBclientAPI.c`, `src/IPBserverData.c`)**: Manages the friend adjacency matrix, thread-safe mutex locks, and exposes clean C functions for the client.
* **Infrastructure Layer (`src/IPBnetwork.c`, `src/IPBparser.c`)**: Manages raw TCP/UDP socket creation, byte-level serialization, and protocol string formatting.

### Abstraction layers and dependency graph:
```

================================================================================
                                APPLICATION LAYER
                (The "Main" files, that handle high level logic)
================================================================================

        [ apps/clientMain.c ]                      [ apps/serverMain.c ]
      (User Interface / Menus)                   (Connection Loop / Threads) ----------------------+
                 |                                          |                                      |
                 |                                          |                                      |
================================================================================                   |
                                   LOGIC LAYER                                                     |
 (The "Brains" that handle lower level logic / data structures implementations)                    |
================================================================================                   |
                 v                                          v                                      |
     [ src/IPBclientAPI.c ]                       [ src/IPBserverData.c ]                          |
     (Full client api)				                        (Manages Users, Friends, Mutex)                  |
    - Client functionalities                     - Server backend funtionalities                   |
                 |                                                                                 |
                 |                                                                                 |
================================================================================                   |
                              INFRASTRUCTURE LAYER                                                 |
                    (The "Workers" that do the heavy lifting)                                      |
================================================================================                   |
                 v                                                                                 |
        [ src/IPBparser.c ]                        [ src/IPBnetwork.c ]                            |
        (The "Translator")                         (The "Postman")          <----------------------+
      - Packs Structs -> Bytes                    - Opens Sockets (TCP/UDP)
      - Unpacks Bytes -> Structs                  - Sends/Receives Raw Data
      - Validates Data                            - Handles Timeouts
                 |                                          |
                 |                                          |
                 +-------------------+----------------------+
                                     |
                                     |
================================================================================
                                FOUNDATION LAYER
                (Definitions used by everyone, protocol rules)
================================================================================
                                     v
                      [ include/IPBtypes.h & IPBprotocol.h ]
                    - Packet Structures (IPBpacket)
                    - Constants (MAX_CLIENTS, PORT)
                    - Error Codes (IPB_ERROR_...)
```

## 🚀 Building and Running

The project includes a `Makefile` configured to compile the programs for standard Linux lab environments.

**1. Compile the binaries:**
```bash
make
```
*(This generates the `server` and `client` executables inside the `bin/` directory).*

**2. Start the Server:**
The server requires a single listening port. Use the `-v` flag to enable the verbose output required for the exam evaluation.
```bash
./bin/server -v <Port>
```

**3. Start a Client:**
The client requires the server's IP, the server's TCP port, and a local UDP port to bind for incoming notifications.
```bash
./bin/client -v <ServerIP> <ServerPort> <LocalUDPPort>
```

**4. Run Automated Tests:**
You can run the built-in test suite to verify the server's adherence to buffer limits, message rejection, and flood propagation.
```bash
make run-tests
```
