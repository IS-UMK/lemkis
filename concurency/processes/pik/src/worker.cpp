#ifndef WORKER_H
#define WORKER_H

#include "unit.h"

class Worker {
  public:
    explicit Worker(int id);
    void run();

  private:
    int id;
    bool shouldSendSignal;
    message received;

    void close_unused_pipes() const;
    void processLoop();

    void sendUnitToHelper();
    void sendSignalToHelper();
    void receiveFromHelper();

    bool handleReceivedMessage(const message& u);

    void logUnitSent() const;
    void logSignalSent() const;
    void logMessageReceived(const message& u) const;
};

#endif
