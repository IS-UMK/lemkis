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
