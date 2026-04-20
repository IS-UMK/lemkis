#pragma once
#include <atomic>
#include <csignal>


class utility {
  public:
    static bool shutdown_requested;
    static void signal_handler(int /*signum*/) { shutdown_requested = true; }
    static void setup_signal_handler() { signal(SIGINT, signal_handler); }
};
