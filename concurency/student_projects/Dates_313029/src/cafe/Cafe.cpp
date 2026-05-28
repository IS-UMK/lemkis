#include "../../include/cafe/Cafe.hpp"
#include <stdexcept>
#include <cstring>
#include <print>

// Inicjalizacja statycznych nazw IPC
const char* Cafe::SHM_NAME = "/cafe_shm";
const char* Cafe::MUTEX_NAME = "/cafe_mutex";
const char* Cafe::CV_NAME = "/cafe_cv";

#include <sys/stat.h>
#include <cstring>

Cafe::Cafe() {
    // Inicjalizacja semaforów z odpowiednimi uprawnieniami

    // Semafor do wzajmenego wyklucznia - wartość pocz. 1
    // O_CREAT – stwórz semafor, jeśli jeszcze nie istnieje.
    // O_RDWR – otwórz go do czytania i pisania.
    // 0666 - uprawnienia unixowe
    // 1 = oznacza, że jeden proces może wejść, potem reszta musi czekać.
    mutex_ = sem_open("/cafe_mutex", O_CREAT | O_RDWR, 0666, 1);
    if (mutex_ == SEM_FAILED) {
        throw std::runtime_error("Mutex init failed: " + std::string(strerror(errno)));
    }

    // Semafor dla powiadamiania o zakończeniu randek - wartość pocz. 0
    cv_ = sem_open("/cafe_cv", O_CREAT | O_RDWR, 0666, 0);
    if (cv_ == SEM_FAILED) {
        throw std::runtime_error("CV init failed: " + std::string(strerror(errno)));
    }

    // Pamięć współdzielona
    shm_fd_ = shm_open("/cafe_shm", O_CREAT | O_RDWR, 0666);
    if (shm_fd_ == -1) {
        throw std::runtime_error("SHM open failed: " + std::string(strerror(errno)));
    }
    
    //Ustalenie rozmiaru pamięci współdzielonej 
    ftruncate(shm_fd_, sizeof(SharedData));
    
    //Mapowanie, aby móć korzystać z pamięci współdzielonej jak z struktury
    shared_data_ = static_cast<SharedData*>(
        mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd_, 0)
    );
    if (shared_data_ == MAP_FAILED) {
        throw std::runtime_error("MMAP failed: " + std::string(strerror(errno)));
    }

    // Inicjalizacja danych 
    sem_wait(mutex_);
    if (shared_data_->initialized != 0x12345678) {  
        memset(shared_data_, 0, sizeof(SharedData));
        shared_data_->queue_size = 0;
        shared_data_->current_date_active = false;
        shared_data_->current_date_pair_id = -1;
        shared_data_->processes_on_date = 0;
        shared_data_->initialized = 0x12345678;
        std::println("[DEBUG] SharedData initialized by process {}", getpid());
    }
    sem_post(mutex_);
}

Cafe::~Cafe() {
    sem_close(mutex_);
    sem_close(cv_);
    munmap(shared_data_, sizeof(SharedData));
    close(shm_fd_);
}

bool Cafe::request_date(int pair_id, int process_id) {
    // Blokowanie mutexa
    sem_wait(mutex_);
    
    std::println("[DEBUG] Process {}-{} looking for partner, queue size: {}", 
                pair_id, process_id, shared_data_->queue_size);
    
    // Sprawdź czy już trwa randka dla tej pary
    if (shared_data_->current_date_active && 
        shared_data_->current_date_pair_id == pair_id) {
        std::println("[DEBUG] Date already in progress for pair {}", pair_id);
        sem_post(mutex_);
        return true; // Dołącz do trwającej randki
    }
    
    // Sprawdź czy trwa JAKAKOLWIEK randka
    if (shared_data_->current_date_active) {
        std::println("[DEBUG] Table occupied by pair {}, adding {}-{} to queue", 
                    shared_data_->current_date_pair_id, pair_id, process_id);
        
        // Sprawdź czy już jesteśmy w kolejce (unikaj duplikatów)
        bool already_in_queue = false;
        for (int i = 0; i < shared_data_->queue_size; i++) {
            if (shared_data_->waiting_queue[i].pair_id == pair_id && 
                shared_data_->waiting_queue[i].process_id == process_id) {
                already_in_queue = true;
                break;
            }
        }
        
        if (!already_in_queue && shared_data_->queue_size < MAX_QUEUE_SIZE) {
            // Dodaj siebie do kolejki oczekujących
            shared_data_->waiting_queue[shared_data_->queue_size] = {pair_id, process_id};
            shared_data_->queue_size++;
            std::println("[DEBUG] Added {}-{} to queue, new size: {}", 
                        pair_id, process_id, shared_data_->queue_size);
        }
        
        sem_post(mutex_);
        return false; // Nie można rozpocząć randki - stolik zajęty
    }
    
    // Stolik jest wolny - sprawdź czy jest dostępny partner w kolejce
    bool partner_found = false;
    int partner_index = -1;
    
    for (int i = 0; i < shared_data_->queue_size; i++) {
        if (shared_data_->waiting_queue[i].pair_id == pair_id && 
            shared_data_->waiting_queue[i].process_id != process_id) {
            partner_found = true;
            partner_index = i;
            std::println("[DEBUG] MATCH! Found partner: {}-{} with {}-{}", 
                        pair_id, process_id, 
                        shared_data_->waiting_queue[i].pair_id, 
                        shared_data_->waiting_queue[i].process_id);
            break;
        }
    }

    if (partner_found) {
        // Usuń partnera z kolejki
        for (int i = partner_index; i < shared_data_->queue_size - 1; i++) {
            shared_data_->waiting_queue[i] = shared_data_->waiting_queue[i + 1];
        }
        shared_data_->queue_size--;
        
        // Rozpocznij randkę
        shared_data_->current_date_active = true;
        shared_data_->current_date_pair_id = pair_id;
        shared_data_->processes_on_date = 2;
        std::println("[DEBUG] Starting date for pair {}", pair_id);
        sem_post(mutex_);
        return true;
    } else {
        // Sprawdź czy już jesteśmy w kolejce 
        bool already_in_queue = false;
        for (int i = 0; i < shared_data_->queue_size; i++) {
            if (shared_data_->waiting_queue[i].pair_id == pair_id && 
                shared_data_->waiting_queue[i].process_id == process_id) {
                already_in_queue = true;
                break;
            }
        }
        
        if (!already_in_queue && shared_data_->queue_size < MAX_QUEUE_SIZE) {
            // Dodaj siebie do kolejki oczekujących
            shared_data_->waiting_queue[shared_data_->queue_size] = {pair_id, process_id};
            shared_data_->queue_size++;
            std::println("[DEBUG] Added {}-{} to queue, new size: {}", 
                        pair_id, process_id, shared_data_->queue_size);
        }
        
        sem_post(mutex_);
        return false;
    }
}

void Cafe::complete_date(int process_id) {
    sem_wait(mutex_);
    
    std::println("[DEBUG] Process {} completing date, processes_on_date: {}", 
                process_id, shared_data_->processes_on_date);
    
    if (shared_data_->processes_on_date > 0) {
        //Zmniejszenie licznika uczestników randki
        shared_data_->processes_on_date--;
        
        //Jeśli ostatnia osoba z pary opuściła randkę to koniec randki i wolny stolik
        if (shared_data_->processes_on_date == 0) {
            std::println("[DEBUG] Date completed, table is now free");
            shared_data_->current_date_active = false;
            shared_data_->current_date_pair_id = -1;
        }
    }
    
    sem_post(mutex_);
}

// Do czyszczenia zasobów IPC
void Cafe::cleanup_ipc_resources() {
    sem_unlink("/cafe_mutex");
    sem_unlink("/cafe_cv");
    shm_unlink("/cafe_shm");
}