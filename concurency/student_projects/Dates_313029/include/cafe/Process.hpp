#pragma once
#include <csignal>
#include <unistd.h>
#include <atomic>
#include <functional>

class Cafe;

class Process {
public:
    Process(int pair_id, int process_id, Cafe& cafe);
    ~Process();

    // Usunięte metody do zarządzania wątkami
    // void start(); 
    // void stop();

    // Główna funkcja wykonawcza (uruchamiana w procesie potomnym)
    void run();

    // Handler sygnału
    static void signal_handler(int sig);

private:
    int pair_id_;
    int process_id_;
    Cafe& cafe_;
    
    // Flaga atomowa do kontrolowania wykonania
    static std::atomic<bool> should_run_;
    
    // Statyczna instancja do obsługi sygnałów
    static Process* current_instance_;
};