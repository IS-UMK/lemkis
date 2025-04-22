# Guide to Using UNIX Sockets in C

---

## **What Are UNIX Sockets?**

UNIX sockets are a type of **Inter-Process Communication (IPC)** mechanism used for communication between processes running on the same machine. Unlike **network sockets** that rely on IP addresses and port numbers, UNIX sockets use a file in the filesystem as the address.

### **Key Features of UNIX Sockets**
1. **Local IPC**:
   - They are optimized for communication between processes on the same machine.
   - Faster than network sockets since they bypass the network stack.

2. **File-Based Addressing**:
   - UNIX sockets are identified by a special file in the filesystem (e.g., `/tmp/socket_file`).

3. **Bidirectional Communication**:
   - Like network sockets, UNIX sockets support bidirectional (full-duplex) communication.

4. **Stream and Datagram Modes**:
   - **SOCK_STREAM**: Provides a reliable byte stream (similar to TCP).
   - **SOCK_DGRAM**: Provides unreliable datagrams (similar to UDP).

---

## **How to Use UNIX Sockets**

Here’s a step-by-step guide to creating a simple server and client using UNIX sockets.

---

### **Step 1: Server Code**

The server creates a socket, binds it to a file, listens for incoming connections, and communicates with the client.

```c name=unix_socket_server.c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/unix_socket" // Path to the socket file
#define BUFFER_SIZE 128

int main() {
    int server_fd, client_fd;
    struct sockaddr_un address;
    char buffer[BUFFER_SIZE] = {0};

    // Create the socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the socket address structure
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    // Bind the socket to the file path
    unlink(SOCKET_PATH); // Remove any existing socket file
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 5) == -1) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on %s...\n", SOCKET_PATH);

    // Accept a connection
    if ((client_fd = accept(server_fd, NULL, NULL)) == -1) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Read data from the client
    read(client_fd, buffer, BUFFER_SIZE);
    printf("Server received: %s\n", buffer);

    // Send a response to the client
    const char *response = "Hello from server!";
    write(client_fd, response, strlen(response));

    // Clean up
    close(client_fd);
    close(server_fd);
    unlink(SOCKET_PATH); // Remove the socket file
    return 0;
}
```

---

### **Step 2: Client Code**

The client connects to the server's socket file and exchanges messages.

```c name=unix_socket_client.c
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define SOCKET_PATH "/tmp/unix_socket" // Path to the socket file
#define BUFFER_SIZE 128

int main() {
    int client_fd;
    struct sockaddr_un address;
    char buffer[BUFFER_SIZE] = {0};

    // Create the socket
    if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Configure the socket address structure
    memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&address, sizeof(address)) == -1) {
        perror("Connect failed");
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Send data to the server
    const char *message = "Hello from client!";
    write(client_fd, message, strlen(message));

    // Read the server's response
    read(client_fd, buffer, BUFFER_SIZE);
    printf("Client received: %s\n", buffer);

    // Clean up
    close(client_fd);
    return 0;
}
```

---

## **Explanation of the Code**

**1. Socket Creation (`socket`)**:
   - Both server and client create a socket using `socket(AF_UNIX, SOCK_STREAM, 0)`.
   - `AF_UNIX`: Specifies the UNIX domain socket family.
   - `SOCK_STREAM`: Specifies a reliable byte stream (similar to TCP).

**2. Binding the Socket (`bind`)**:
   - The server binds the socket to a specific file path (`SOCKET_PATH`).
   - `unlink(SOCKET_PATH)` ensures the file path is cleaned up before binding.

**3. Listening for Connections (`listen`)**:
   - The server listens for incoming connections using `listen(server_fd, 5)`.

**4. Accepting a Connection (`accept`)**:
   - The server accepts a client's connection using `accept(server_fd, NULL, NULL)`.

**5. Connecting to the Server (`connect`)**:
   - The client connects to the server using `connect(client_fd, (struct sockaddr*)&address, sizeof(address))`.

**6. Data Exchange (`read` & `write`)**:
   - Both server and client use `read` and `write` to exchange messages.

**7. Cleanup**:
   - The server removes the socket file using `unlink(SOCKET_PATH)` when done.

---

## **Output for the Example**

1. **Server Output**:
   ```
   Server listening on /tmp/unix_socket...
   Server received: Hello from client!
   ```

2. **Client Output**:
   ```
   Client received: Hello from server!
   ```

---

## **Comparison: UNIX Sockets vs Pipes**

| **Feature**                 | **UNIX Sockets**                                    | **Pipes**                                   |
|-----------------------------|----------------------------------------------------|--------------------------------------------|
| **Directionality**          | Bidirectional (full-duplex).                       | Unidirectional (single pipe).              |
| **Scope**                   | Local IPC only.                                    | Local IPC only.                            |
| **Communication Type**      | Supports both stream (reliable) and datagram modes.| Supports raw byte streams only.            |
| **Addressing**              | File path in the filesystem.                      | No explicit addressing (parent-child processes share). |
| **Performance**             | Slightly slower than pipes due to additional flexibility.| Faster due to simplicity.                  |
| **Use Case**                | Complex IPC between unrelated processes.           | Simple IPC between parent-child processes. |

---

## **When to Use UNIX Sockets**

1. **Advantages**:
   - Full-duplex communication.
   - Suitable for communication between unrelated processes.
   - More flexible than pipes (can use stream or datagram modes).

2. **Use Cases**:
   - Applications requiring robust IPC (e.g., databases communicating with applications).
   - Communication between unrelated processes (no parent-child relationship).

---

## **Summary**

UNIX sockets are a powerful and flexible IPC mechanism for processes running on the same machine. While slightly more complex to use than pipes, they offer bidirectional communication, file-based addressing, and support for various communication modes, making them ideal for advanced IPC scenarios.

# Sockets in C and Their Comparison to Pipes

---

## **What Are Sockets?**

**Sockets** are endpoints for communication between processes, either on the same machine or across different machines over a network. They are a low-level interface provided by operating systems for inter-process communication (IPC) and network communication using protocols such as **TCP/IP** or **UDP**.

### **Key Characteristics of Sockets**

1. **Bidirectional**:
   - Sockets allow full-duplex (bidirectional) communication, meaning data can flow in both directions simultaneously.

2. **Network-Based Communication**:
   - Sockets enable communication between processes running on different machines via a network (e.g., Internet or LAN).
   - They can also be used for local communication (on the same machine) through **UNIX domain sockets**.

3. **Protocol Support**:
   - Sockets support several protocols, most commonly:
     - **TCP** (Transmission Control Protocol) for reliable, connection-oriented communication.
     - **UDP** (User Datagram Protocol) for fast, connectionless communication.

4. **Addressing**:
   - Sockets use IP addresses and port numbers to identify endpoints for communication over a network.
   - For local communication, UNIX domain sockets use pathnames in the filesystem.

5. **File Descriptor-Based**:
   - Like pipes, sockets are represented by file descriptors, and operations like `read()` and `write()` can be used.

---

## **Basic Example of Sockets in C**

Here’s an example of a simple client-server program using TCP sockets:

### **Server Code**
```c name=server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Accept a connection
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Read data from client
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Server received: %s\n", buffer);

    // Send a response to the client
    const char *response = "Hello from server!";
    send(new_socket, response, strlen(response), 0);

    close(new_socket);
    close(server_fd);
    return 0;
}
```

### **Client Code**
```c name=client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    // Configure server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Send data to server
    const char *message = "Hello from client!";
    send(sock, message, strlen(message), 0);

    // Read response from server
    read(sock, buffer, BUFFER_SIZE);
    printf("Client received: %s\n", buffer);

    close(sock);
    return 0;
}
```

---

### **Output for Client-Server Example**

1. **Server Output**:
   ```
   Server received: Hello from client!
   ```

2. **Client Output**:
   ```
   Client received: Hello from server!
   ```

---

## **Comparison of Sockets and Pipes**

| **Feature**                 | **Pipes**                                                                 | **Sockets**                                                                                       |
|-----------------------------|---------------------------------------------------------------------------|--------------------------------------------------------------------------------------------------|
| **Directionality**          | Unidirectional (data flows in one direction) by default.                 | Bidirectional (data flows in both directions simultaneously).                                    |
| **Scope**                   | Limited to parent-child processes or threads.                            | Can communicate across machines over a network or locally (e.g., UNIX domain sockets).           |
| **Communication Type**      | Local communication only.                                                | Local and remote communication (network-based).                                                  |
| **Protocol**                | No protocol, just raw data transfer.                                     | Supports protocols like TCP (reliable) and UDP (fast, connectionless).                           |
| **Addressing**              | No addressing needed (shared by parent-child processes).                 | Uses IP addresses and port numbers for network communication or filesystem paths for UNIX sockets. |
| **Performance**             | Faster for local communication due to simplicity and lower overhead.     | Slightly slower due to protocol stack and network overhead.                                      |
| **Complexity**              | Simple API (just `read` and `write`).                                    | More complex (e.g., `socket`, `bind`, `listen`, `connect`).                                       |
| **Security**                | Limited to local system processes.                                       | Requires proper security measures (e.g., encryption) for safe use over networks.                 |

---

## **When to Use Sockets vs. Pipes**

1. **Use Pipes When**:
   - Communication is limited to processes on the same machine.
   - Parent and child processes need to exchange data.
   - Low-latency, simple communication is required.

2. **Use Sockets When**:
   - Communication must occur between processes on different machines.
   - You need to use network protocols (e.g., TCP, UDP).
   - Bidirectional communication is required.

---

## **Summary**

- **Pipes** are simpler and faster for local IPC between parent-child processes but are unidirectional and limited to the same machine.
- **Sockets** are more versatile, supporting bidirectional communication both locally and over a network, but they are more complex to use and have slightly higher overhead.

Both tools serve distinct use cases and are essential for inter-process and network communication in C programs.
