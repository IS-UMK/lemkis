#include "../../include/cafe/Process.hpp"
#include "../../include/cafe/Cafe.hpp"
#include <iostream>
#include <csignal>
#include <random>
#include <chrono>
#include <print>

// Flaga - czy proces ma dalej działać
std::atomic<bool> Process::should_run_(true);

// Aby widzieć, który proces zatrzymać przez np. Ctrl C
Process* Process::current_instance_ = nullptr;

//Konstruktor 
Process::Process(int pair_id, int process_id, Cafe& cafe) 
    : pair_id_(pair_id), process_id_(process_id), cafe_(cafe) {
    
    // Konfiguracja handlera sygnałów
    if (current_instance_ == nullptr) {
        //Proces ustawiamy jako aktualny
        current_instance_ = this;
        struct sigaction sa;
        sa.sa_handler = signal_handler;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = 0;
        sigaction(SIGTERM, &sa, nullptr);
        sigaction(SIGINT, &sa, nullptr);  // Obsługa Ctrl+C
    }
}

//Destruktor
Process::~Process() {
    if (current_instance_ == this) {
        current_instance_ = nullptr;
    }
}

void Process::signal_handler(int sig) {
    //Jeśli przyjdzie sygnał zakończenia, to zmieniamy wartość flagi, co w konsekwencji zatrzymuje
    //główną pętlę działania procesu
    if (sig == SIGTERM || sig == SIGINT) {
        should_run_.store(false);
        std::println("[SIGNAL] Process received termination signal");
    }
}

void Process::run() {

    //Do generowania liczb losowych
    std::random_device rd;
    std::mt19937 gen(rd());

    //Losowy czas pracy i losowy czas randki
    std::uniform_int_distribution<> work_dist(100, 500);
    std::uniform_int_distribution<> date_dist(200, 800);

    std::println("[PID {}] Process {}-{} starting main loop", getpid(), pair_id_, process_id_);

    while (should_run_.load()) {

        // Symulacja pracy własnej
        // Wedłyg treści zadania:  Each process cyclically performs its own tasks
        int work_time = work_dist(gen) * 1000; // 100-500ms
        usleep(work_time);
        
        // Sprawdzanie czy nadal trzeba pracować - czy nie zostało przerwane działanie
        if (!should_run_.load()) {
            break;
        }

        // Próba umówienia randki
        bool date_started = false;
        try {
            std::println("[PID {}] Requesting date {}-{}", getpid(), pair_id_, process_id_);

            //Przez kawiarnię żądana jest randka
            date_started = cafe_.request_date(pair_id_, process_id_);
            
            if (!date_started) {
                std::println("[PID {}] Added to waiting queue {}-{}", getpid(), pair_id_, process_id_);
                // Czekaj krótko przed kolejną próbą
                usleep(50000); // 50ms
                continue;
            }
        } catch (const std::exception& e) {
            std::println("[PID {}] ERROR in request_date: {}", getpid(), e.what());
            break;
        }

        // Randka się rozpoczęła
        if (date_started) {
            std::println("[PID {}] DATE STARTED {}-{}", getpid(), pair_id_, process_id_);

            // Symulacja randki - każdy proces może skończyć o innym czasie
            int date_time = date_dist(gen) * 1000; // 200-800ms
            usleep(date_time);

            std::println("[PID {}] DATE FINISHED {}-{}", getpid(), pair_id_, process_id_);
            
            //Gdy koniec randki -> informuj kawiarnię 
            try {
                cafe_.complete_date(process_id_);
            } catch (const std::exception& e) {
                std::println("[PID {}] ERROR in complete_date: {}", getpid(), e.what());
                break;
            }

            // Sprawdź czy nadal powinniśmy działać po randce
            if (!should_run_.load()) {
                break;
            }
        }
    }
    
    std::println("[PID {}] Process {}-{} terminating gracefully", getpid(), pair_id_, process_id_);
}