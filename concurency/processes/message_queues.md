### **Message Queues in C**
Message queues are a mechanism for **Inter-Process Communication (IPC)** that allow multiple processes to exchange messages in a coordinated manner. They are particularly useful for asynchronous communication, where processes do not need to wait for each other to send or receive data.

---

### **How Message Queues Work**
1. **Concept**:
   - A message queue is a data structure that stores messages.
   - Messages are sent to the queue by one process and retrieved by another.
   - Each message is treated as an independent entity with an optional priority.

2. **Key Characteristics**:
   - **Asynchronous**: Sending and receiving processes don’t need to synchronize directly.
   - **Prioritization**: Messages with higher priority can be delivered before lower-priority ones.

3. **POSIX Message Queues**:
   - These are standardized in the POSIX API and are implemented in most modern Unix-like operating systems, including Linux.
   - POSIX message queues are identified by a **name** (e.g., `/myqueue`).

---

### **POSIX Message Queues API**

To use POSIX message queues in C, you include the following header:
```c
#include <mqueue.h>
```

Here are the main functions used to work with message queues:

1. **Create or Open a Message Queue**
   ```c
   mqd_t mq_open(const char *name, int oflag, mode_t mode, struct mq_attr *attr);
   ```
   - `name`: Name of the queue (e.g., `/myqueue`).
   - `oflag`: Flags such as `O_CREAT` (create queue) or `O_RDONLY`/`O_WRONLY`.
   - `mode`: Permissions (e.g., `0666` for read/write access).
   - `attr`: Attributes like max message size and queue depth (can be NULL for defaults).
   - Returns a message queue descriptor (`mqd_t`).

   Example:
   ```c
   mqd_t mq = mq_open("/myqueue", O_CREAT | O_RDWR, 0666, NULL);
   ```

2. **Send a Message**
   ```c
   int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len, unsigned int msg_prio);
   ```
   - `mqdes`: Message queue descriptor returned by `mq_open`.
   - `msg_ptr`: Pointer to the message to send.
   - `msg_len`: Length of the message.
   - `msg_prio`: Priority (higher value = higher priority).

   Example:
   ```c
   mq_send(mq, "Hello", 5, 1);
   ```

3. **Receive a Message**
   ```c
   ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len, unsigned int *msg_prio);
   ```
   - `mqdes`: Message queue descriptor.
   - `msg_ptr`: Buffer to store the received message.
   - `msg_len`: Size of the buffer.
   - `msg_prio`: Optional pointer to store the priority of the received message.

   Example:
   ```c
   char buffer[128];
   mq_receive(mq, buffer, 128, NULL);
   printf("Received: %s\n", buffer);
   ```

4. **Close the Message Queue**
   ```c
   int mq_close(mqd_t mqdes);
   ```
   - Closes the message queue descriptor.

   Example:
   ```c
   mq_close(mq);
   ```

5. **Remove a Message Queue**
   ```c
   int mq_unlink(const char *name);
   ```
   - Deletes the queue from the system (useful for cleanup).

   Example:
   ```c
   mq_unlink("/myqueue");
   ```

6. **Set and Get Message Queue Attributes**
   - Set attributes:
     ```c
     int mq_setattr(mqd_t mqdes, const struct mq_attr *newattr, struct mq_attr *oldattr);
     ```
   - Get attributes:
     ```c
     int mq_getattr(mqd_t mqdes, struct mq_attr *attr);
     ```

---

### **Example Code for POSIX Message Queues**

#### **Server (Sender)**
```c name=server.c
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    mqd_t mq;
    struct mq_attr attr;

    // Set message queue attributes
    attr.mq_flags = 0;         // Blocking mode
    attr.mq_maxmsg = 10;       // Maximum number of messages
    attr.mq_msgsize = 128;     // Maximum message size
    attr.mq_curmsgs = 0;       // Number of messages currently in queue

    // Create message queue
    mq = mq_open("/myqueue", O_CREAT | O_WRONLY, 0666, &attr);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    // Send a message
    const char *message = "Hello from server";
    if (mq_send(mq, message, strlen(message) + 1, 1) == -1) {
        perror("mq_send");
        exit(1);
    }

    printf("Message sent: %s\n", message);

    // Close message queue
    mq_close(mq);

    return 0;
}
```

#### **Client (Receiver)**
```c name=client.c
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    mqd_t mq;
    char buffer[128];
    struct mq_attr attr;

    // Open message queue
    mq = mq_open("/myqueue", O_RDONLY);
    if (mq == (mqd_t)-1) {
        perror("mq_open");
        exit(1);
    }

    // Receive a message
    if (mq_receive(mq, buffer, 128, NULL) == -1) {
        perror("mq_receive");
        exit(1);
    }

    printf("Message received: %s\n", buffer);

    // Close message queue
    mq_close(mq);

    return 0;
}
```

---

# Cpp wrapper

```cpp
#pragma once

#include <mqueue.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#include <string>
#include <string_view>
#include <expected>
#include <format>
#include <span>
#include <chrono>
#include <concepts>
#include <memory>
#include <system_error>
#include <optional>
#include <filesystem>
#include <vector>

namespace ipc {

/**
 * @brief Error codes specific to message queue operations
 */
enum class message_queue_error {
    open_failed,
    send_failed,
    receive_failed,
    attributes_failed,
    notification_failed,
    unlink_failed,
    timeout
};

/**
 * @brief Implementation of the error_category for message_queue_error
 */
class message_queue_error_category : public std::error_category {
public:
    [[nodiscard]] const char* name() const noexcept override {
        return "message_queue";
    }
    
    [[nodiscard]] std::string message(int ev) const override {
        switch (static_cast<message_queue_error>(ev)) {
            case message_queue_error::open_failed:
                return "Failed to open message queue";
            case message_queue_error::send_failed:
                return "Failed to send message";
            case message_queue_error::receive_failed:
                return "Failed to receive message";
            case message_queue_error::attributes_failed:
                return "Failed to get or set queue attributes";
            case message_queue_error::notification_failed:
                return "Failed to set up notification";
            case message_queue_error::unlink_failed:
                return "Failed to unlink message queue";
            case message_queue_error::timeout:
                return "Message queue operation timed out";
            default:
                return "Unknown message queue error";
        }
    }

    [[nodiscard]] static const message_queue_error_category& instance() noexcept {
        static message_queue_error_category instance;
        return instance;
    }
};

/**
 * @brief Convert message_queue_error to std::error_code
 */
[[nodiscard]] inline std::error_code make_error_code(message_queue_error e) noexcept {
    return {static_cast<int>(e), message_queue_error_category::instance()};
}

/**
 * @brief Attributes for a message queue
 */
struct message_queue_attributes {
    long flags{0};
    long max_messages{10};
    long message_size{8192};
    long current_messages{0};
    
    [[nodiscard]] constexpr mq_attr to_native() const noexcept {
        mq_attr attr{};
        attr.mq_flags = flags;
        attr.mq_maxmsg = max_messages;
        attr.mq_msgsize = message_size;
        attr.mq_curmsgs = current_messages;
        return attr;
    }
    
    [[nodiscard]] static constexpr message_queue_attributes from_native(const mq_attr& attr) noexcept {
        return {
            .flags = attr.mq_flags,
            .max_messages = attr.mq_maxmsg,
            .message_size = attr.mq_msgsize,
            .current_messages = attr.mq_curmsgs
        };
    }
};

/**
 * @brief Result type for operations that can fail
 */
template<typename T>
using result = std::expected<T, std::error_code>;

/**
 * @brief A RAII wrapper around POSIX message queues
 */
class message_queue {
public:
    /**
     * @brief Open modes for the message queue
     */
    enum class mode {
        read_only = O_RDONLY,
        write_only = O_WRONLY,
        read_write = O_RDWR
    };
    
    /**
     * @brief Custom deleter for the message queue descriptor
     */
    struct mq_descriptor_deleter {
        void operator()(mqd_t* mqdes) const noexcept {
            if (mqdes && *mqdes != static_cast<mqd_t>(-1)) {
                mq_close(*mqdes);
            }
            delete mqdes;
        }
    };
    
    using mq_descriptor_handle = std::unique_ptr<mqd_t, mq_descriptor_deleter>;

    /**
     * @brief Open an existing message queue
     * 
     * @param name Name of the message queue
     * @param mode Access mode
     * @return result containing the message queue or an error
     */
    [[nodiscard]] static result<message_queue> open(
        std::string_view name, 
        mode mode = mode::read_write) 
    {
        return open_impl(name, static_cast<int>(mode), false);
    }
    
    /**
     * @brief Create a new message queue or open if it exists
     * 
     * @param name Name of the message queue
     * @param mode Access mode
     * @param permissions File permissions for the queue
     * @param attributes Message queue attributes
     * @return result containing the message queue or an error
     */
    [[nodiscard]] static result<message_queue> create(
        std::string_view name,
        mode mode = mode::read_write,
        mode_t permissions = 0666,
        const std::optional<message_queue_attributes>& attributes = std::nullopt)
    {
        int flags = static_cast<int>(mode) | O_CREAT;
        
        if (attributes) {
            mq_attr attr = attributes->to_native();
            auto mqdes = std::make_unique<mqd_t>(
                mq_open(validate_name(name).c_str(), flags, permissions, &attr));
            
            if (*mqdes == static_cast<mqd_t>(-1)) {
                return std::unexpected(make_error_code(message_queue_error::open_failed));
            }
            
            return message_queue(mq_descriptor_handle(std::move(mqdes)), std::string(name));
        } else {
            return open_impl(name, flags, true, permissions);
        }
    }

    /**
     * @brief Remove a message queue from the system
     * 
     * @param name Name of the message queue
     * @return result indicating success or failure
     */
    [[nodiscard]] static result<void> unlink(std::string_view name) {
        if (mq_unlink(validate_name(name).c_str()) == -1) {
            return std::unexpected(make_error_code(message_queue_error::unlink_failed));
        }
        return {};
    }
    
    /**
     * @brief Send a message to the queue
     * 
     * @param data Message data
     * @param priority Message priority
     * @return result indicating success or failure
     */
    template<typename T> requires std::is_trivially_copyable_v<T>
    [[nodiscard]] result<void> send(const T& data, unsigned int priority = 0) const {
        return send(std::as_bytes(std::span{&data, 1}), priority);
    }
    
    /**
     * @brief Send a message consisting of a span of bytes to the queue
     * 
     * @param data Message data as a span of bytes
     * @param priority Message priority
     * @return result indicating success or failure
     */
    [[nodiscard]] result<void> send(std::span<const std::byte> data, unsigned int priority = 0) const {
        if (!mqdes_ || *mqdes_ == static_cast<mqd_t>(-1)) {
            return std::unexpected(make_error_code(message_queue_error::send_failed));
        }
        
        if (mq_send(*mqdes_, reinterpret_cast<const char*>(data.data()),
                    data.size_bytes(), priority) == -1) {
            return std::unexpected(make_error_code(message_queue_error::send_failed));
        }
        
        return {};
    }
    
    /**
     * @brief Send a message with a timeout
     * 
     * @param data Message data as a span of bytes
     * @param timeout Maximum time to wait
     * @param priority Message priority
     * @return result indicating success or failure
     */
    template<typename Rep, typename Period>
    [[nodiscard]] result<void> send(std::span<const std::byte> data, 
                                  const std::chrono::duration<Rep, Period>& timeout, 
                                  unsigned int priority = 0) const {
        if (!mqdes_ || *mqdes_ == static_cast<mqd_t>(-1)) {
            return std::unexpected(make_error_code(message_queue_error::send_failed));
        }
        
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - seconds);
        
        timespec ts{
            .tv_sec = static_cast<time_t>(seconds.count()),
            .tv_nsec = static_cast<long>(nanoseconds.count())
        };
        
        if (mq_timedsend(*mqdes_, reinterpret_cast<const char*>(data.data()),
                        data.size_bytes(), priority, &ts) == -1) {
            if (errno == ETIMEDOUT) {
                return std::unexpected(make_error_code(message_queue_error::timeout));
            }
            return std::unexpected(make_error_code(message_queue_error::send_failed));
        }
        
        return {};
    }
    
    /**
     * @brief Receive a message and interpret it as type T
     * 
     * @tparam T Type to interpret the message as
     * @param priority Optional pointer to store message priority
     * @return result containing the received message or an error
     */
    template<typename T> requires std::is_trivially_copyable_v<T>
    [[nodiscard]] result<T> receive(unsigned int* priority = nullptr) const {
        T result{};
        auto bytes = std::as_writable_bytes(std::span{&result, 1});
        auto receive_result = receive_bytes(bytes, priority);
        
        if (!receive_result) {
            return std::unexpected(receive_result.error());
        }
        
        return result;
    }
    
    /**
     * @brief Receive a message as raw bytes
     * 
     * @param output Buffer to store the message
     * @param priority Optional pointer to store message priority
     * @return result containing the number of bytes received or an error
     */
    [[nodiscard]] result<size_t> receive_bytes(std::span<std::byte> output, 
                                             unsigned int* priority = nullptr) const {
        if (!mqdes_ || *mqdes_ == static_cast<mqd_t>(-1)) {
            return std::unexpected(make_error_code(message_queue_error::receive_failed));
        }
        
        ssize_t result = mq_receive(*mqdes_, reinterpret_cast<char*>(output.data()),
                                   output.size_bytes(), priority);
        
        if (result == -1) {
            return std::unexpected(make_error_code(message_queue_error::receive_failed));
        }
        
        return static_cast<size_t>(result);
    }
    
    /**
     * @brief Receive a message with a timeout
     * 
     * @tparam T Type to interpret the message as
     * @param timeout Maximum time to wait
     * @param priority Optional pointer to store message priority
     * @return result containing the received message or an error
     */
    template<typename T, typename Rep, typename Period> requires std::is_trivially_copyable_v<T>
    [[nodiscard]] result<T> receive(const std::chrono::duration<Rep, Period>& timeout,
                                 unsigned int* priority = nullptr) const {
        T result{};
        auto bytes = std::as_writable_bytes(std::span{&result, 1});
        auto receive_result = receive_bytes(bytes, timeout, priority);
        
        if (!receive_result) {
            return std::unexpected(receive_result.error());
        }
        
        return result;
    }
    
    /**
     * @brief Receive raw bytes with a timeout
     * 
     * @param output Buffer to store the message
     * @param timeout Maximum time to wait
     * @param priority Optional pointer to store message priority
     * @return result containing the number of bytes received or an error
     */
    template<typename Rep, typename Period>
    [[nodiscard]] result<size_t> receive_bytes(std::span<std::byte> output,
                                             const std::chrono::duration<Rep, Period>& timeout,
                                             unsigned int* priority = nullptr) const {
        if (!mqdes_ || *mqdes_ == static_cast<mqd_t>(-1)) {
            return std::unexpected(make_error_code(message_queue_error::receive_failed));
        }
        
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(timeout);
        auto nanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(timeout - seconds);
        
        timespec ts{
            .tv_sec = static_cast<time_t>(seconds.count()),
            .tv_nsec = static_cast<long>(nanoseconds.count())
        };
        
        ssize_t result = mq_timedreceive(*mqdes_, reinterpret_cast<char*>(output.data()),
                                        output.size_bytes(), priority, &ts);
        
        if (result == -1) {
            if (errno == ETIMEDOUT) {
                return std::unexpected(make_error_code(message_queue_error::timeout));
            }
            return std::unexpected(make_error_code(message_queue_error::receive_failed));
        }
        
        return static_cast<size_t>(result);
    }
    
    /**
     * @brief Get the current attributes of the message queue
     * 
     * @return result containing the attributes or an error
     */
    [[nodiscard]] result<message_queue_attributes> get_attributes() const {
        if (!mqdes_ || *mqdes_ == static_cast<mqd_t>(-1)) {
            return std::unexpected(make_error_code(message_queue_error::attributes_failed));
        }
        
        mq_attr attr{};
        if (mq_getattr(*mqdes_, &attr) == -1) {
            return std::unexpected(make_error_code(message_queue_error::attributes_failed));
        }
        
        return message_queue_attributes::from_native(attr);
    }
    
    /**
     * @brief Set the attributes of the message queue
     * 
     * @param attributes New attributes for the queue
     * @return result containing the old attributes or an error
     */
    [[nodiscard]] result<message_queue_attributes> set_attributes(const message_queue_attributes& attributes) {
        if (!mqdes_ || *mqdes_ == static_cast<mqd_t>(-1)) {
            return std::unexpected(make_error_code(message_queue_error::attributes_failed));
        }
        
        mq_attr new_attr = attributes.to_native();
        mq_attr old_attr{};
        
        if (mq_setattr(*mqdes_, &new_attr, &old_attr) == -1) {
            return std::unexpected(make_error_code(message_queue_error::attributes_failed));
        }
        
        return message_queue_attributes::from_native(old_attr);
    }
    
    /**
     * @brief Get the name of the message queue
     * 
     * @return The name of the message queue
     */
    [[nodiscard]] std::string_view name() const noexcept {
        return name_;
    }
    
    // Movable but not copyable
    message_queue(message_queue&& other) noexcept = default;
    message_queue& operator=(message_queue&& other) noexcept = default;
    
    message_queue(const message_queue&) = delete;
    message_queue& operator=(const message_queue&) = delete;
    
    ~message_queue() = default;

private:
    mq_descriptor_handle mqdes_;
    std::string name_;
    
    message_queue(mq_descriptor_handle mqdes, std::string name)
        : mqdes_(std::move(mqdes)), name_(std::move(name)) {}
    
    [[nodiscard]] static std::string validate_name(std::string_view name) {
        // Message queue names must start with a '/'
        if (name.empty() || name[0] != '/') {
            
```

and `server.cpp`:

```cpp

```
and `client.cpp`

```cpp
```
