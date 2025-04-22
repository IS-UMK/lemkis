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
