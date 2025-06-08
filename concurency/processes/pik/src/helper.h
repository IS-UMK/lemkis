#ifndef HELPER_H
#define HELPER_H

#include <optional>

#include "unit.h"

class Helper {
  public:
    Helper(int id);
    void run();

  private:
    int id;
    int next_id;
    std::optional<int> unit_type;
    struct Buffer {
        message unit;
        bool busy = false;
    };
    Buffer buffer;

    void close_unused_pipes_ends() const;
    void processLoop();

    void receiveFromWorker();
    void handleReceivedMessage(message u);
    void receiveFromHelper();
    void pass_unit_to_next_helper();

    // Logging
    void logSendToWorker(message u) const;
    void logReceiveFromWorker(message u) const;
    void logReceiveFromHelper(message u) const;
    void logSendToHelper(message u) const;
};

#endif  // HELPER_H
