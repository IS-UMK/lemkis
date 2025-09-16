#pragma once
#include <cstdint>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <optional>

// Struktura reprezentująca żądanie randki
struct DateRequest {
    //Identyfikator pary
    int pair_id;

    //Identyfikator procesu w parze
    int process_id;
    
    //Konstruktory
    DateRequest() : pair_id(-1), process_id(-1) {}
    DateRequest(int p_id, int pr_id) : pair_id(p_id), process_id(pr_id) {}
};

constexpr int MAX_QUEUE_SIZE = 10;  

struct SharedData {
    //Kolejka oczekujących na randkę
    DateRequest waiting_queue[MAX_QUEUE_SIZE];

    //Aktualna wielkość kolejki
    int queue_size;
    
    //Czy obecnie trwa randka
    bool current_date_active;

    //ID pary, która ma randkę
    int current_date_pair_id;

    //ile procesów biedrze udział w randce
    int processes_on_date;
    
    //do sprawdzenia czy pamięć została zainicjalizowana
    uint32_t initialized;
};

class Cafe {
public:
    //Konstruktory
    Cafe();
    ~Cafe();
    
    //Ządanie randki
    bool request_date(int pair_id, int process_id);

    //Zakończenie randki
    void complete_date(int process_id);

    //Czyszczenie zasobów IPC
    static void cleanup_ipc_resources();

private:
    //Nazwy zasobów IPC
    static const char* SHM_NAME;
    static const char* MUTEX_NAME;
    static const char* CV_NAME;
    
    //Semafor binarny do synchronizacji dostępu
    sem_t* mutex_;

    //Semafor do powiadamiania (condition variable)
    sem_t* cv_;

    //Deskryptor pamięci współdzielonej 
    int shm_fd_;

    //Wskaźnik do danych współdzielonych
    SharedData* shared_data_;
};