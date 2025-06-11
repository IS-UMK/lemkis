# UNIX Sockets in C

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


# Cpp wrapper

I asked AI to write a modern wrapper for unix socket. Is this implementation correct? Can you see a potential problem?
```cpp
#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <filesystem>
#include <ranges>
#include <iostream>
#include <format>
#include <span>

namespace communication {

class unix_socket {
public:

unix_socket() : socket_fd_(-1) {
        socket_fd_ = ::socket(AF_UNIX, SOCK_STREAM, 0);
        if (socket_fd_ == -1) {
            throw std::runtime_error("Failed to create UNIX socket");
        }
    }

    explicit unix_socket(int fd) : socket_fd_(fd) {}

    ~unix_socket() {
        close();
    }

    unix_socket(const unix_socket&) = delete;

    unix_socket& operator=(const unix_socket&) = delete;

    unix_socket(unix_socket&& other) noexcept : socket_fd_(other.socket_fd_) {
        other.socket_fd_ = -1;
    }

    unix_socket& operator=(unix_socket&& other) noexcept {
        if (this != &other) {
            close();
            socket_fd_ = other.socket_fd_;
            other.socket_fd_ = -1;
        }
        return *this;
    }


   void bind(const std::filesystem::path& path) {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;

        if (path.string().size() >= sizeof(addr.sun_path)) {
            throw std::runtime_error("Socket path too long");
        }
        std::ranges::copy_n(path.string().begin(), path.string().size(), addr.sun_path);
        if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
        }
        if (::bind(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            throw std::runtime_error(std::format("Failed to bind UNIX socket to '{}'", path.string()));
        }
    }

    // Start listening for connections (server-specific)
    void listen(int backlog = 5) const {
        if (::listen(socket_fd_, backlog) == -1) {
            throw std::runtime_error("Failed to listen on UNIX socket");
        }
    }

    // Accept a new connection (server-specific)
    unix_socket accept() const {
        int client_fd = ::accept(socket_fd_, nullptr, nullptr);
        if (client_fd == -1) {
            throw std::runtime_error("Failed to accept connection on UNIX socket");
        }
        return unix_socket(client_fd);
    }

    // Connect to a server (client-specific)
    void connect(const std::filesystem::path& path) {
        sockaddr_un addr{};
        addr.sun_family = AF_UNIX;
        // Ensure the path fits within the sun_path limit
        if (path.string().size() >= sizeof(addr.sun_path)) {
            throw std::runtime_error("Socket path too long");
        }
        std::ranges::copy_n(path.string().begin(), path.string().size(), addr.sun_path);
        if (::connect(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
            throw std::runtime_error(std::format("Failed to connect to UNIX socket at '{}'", path.string()));
        }
    }

    // Send data through the socket
    void send(std::string_view message) const {
        if (::send(socket_fd_, message.data(), message.size(), 0) == -1) {
            throw std::runtime_error("Failed to send data through UNIX socket");
        }
    }

    // Receive data from the socket
    std::string receive(size_t buffer_size = 1024) const {
        std::string buffer(buffer_size, '\0');
        ssize_t bytes_received = ::recv(socket_fd_, buffer.data(), buffer_size, 0);
        if (bytes_received == -1) {
            throw std::runtime_error("Failed to receive data from UNIX socket");
        }
        buffer.resize(static_cast<size_t>(bytes_received)); // Trim the buffer to actual size
        return buffer;
    }

    // Close the socket
    void close() {
        if (socket_fd_ != -1) {
            ::close(socket_fd_);
            socket_fd_ = -1;
        }
    }

private:
    int socket_fd_; // File descriptor for the socket
};

} // namespace communication
```
<details><summary>correction</summary><p>
      <code>
      void bind(const std::filesystem::path& path) {
         sockaddr_un addr{};addr.sun_family = AF_UNIX;
       if (path.string().size() >= sizeof(addr.sun_path)) {
           throw std::runtime_error("Socket path too long");
       }
       // Copy the socket path into the address structure
       std::ranges::copy_n(path.string().begin(), path.string().size(), addr.sun_path);
       // Safely remove any existing socket file
       ::unlink(path.c_str()); // Atomic and safe for UNIX domain sockets
       // Bind the socket
       if (::bind(socket_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == -1) {
           throw std::runtime_error(std::format("Failed to bind UNIX socket to '{}'", path.string()));
       }
      }
      </code>
</p></details>

which allows to rewrite server:

```cpp
#include "unix_socket.h"
#include <iostream>
#include <format>

#define SOCKET_PATH "/tmp/unix_socket"

int main() {
    try {
        communication::unix_socket server_socket;
        server_socket.bind(SOCKET_PATH);
        server_socket.listen();
        std::println("Server listening on {}", SOCKET_PATH);
        communication::unix_socket client_socket = server_socket.accept();
        std::println("Client connected!");
        std::string message = client_socket.receive();
        std::println("Server received: {}", message);
        std::string response = "Hello from server!";
        client_socket.send(response);

    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```
and client
```cpp
#include "unix_socket.h"
#include <iostream>
#include <format>

#define SOCKET_PATH "/tmp/unix_socket"

int main() {
    try {
        communication::unix_socket client_socket;
        client_socket.connect(SOCKET_PATH);
        std::println("Connected to server at {}", SOCKET_PATH);
        std::string message = "Hello from client!";
        client_socket.send(message);
        std::string response = client_socket.receive();
        std::println("Client received: {}", response);
    } catch (const std::exception& e) {
        std::println("Error: {}", e.what());
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
```

You can compile example using
```bash
g++ -std=c++23 -o server server.cpp && g++ -std=c++23 -o client client.cpp
```
 and then 

```bash
./server & sleep 1 && ./client
```
Why `sleep 1`? What might happen if we do not sleep?
