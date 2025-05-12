#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <queue>
#include <cstring>
#include <cstdlib>
#include <ctime>

const int N = 5;

using UnitType = int;  // Definicja typu jednostki
using unit = int;

UnitType type(const unit& u) {
     return u;
}

// Struktura reprezentująca jednostkę produkowaną przez Workerów
struct Unit {
    UnitType type;     // Typ jednostki (np. 0,1,2,...)
    int producer_id;   // ID Workera, który ją stworzył
};

// Funkcja konwertująca typ jednostki na string (np. "0", "1", ...)
std::string unitTypeToString(UnitType type) {
    return std::to_string(type);
}

// Funkcja odczytu jednostki z potoku (blokowo lub nieblokowo, zależnie od ustawienia deskryptora)
bool read_unit(int fd, Unit &u) {
    ssize_t bytes_read = read(fd, &u, sizeof(Unit));
    return bytes_read == sizeof(Unit);
}

// Funkcja zapisu jednostki do potoku
bool write_unit(int fd, const Unit &u) {
    ssize_t bytes_written = write(fd, &u, sizeof(Unit));
    return bytes_written == sizeof(Unit);
}


int main() {
    // Tworzenie trzech rodzajów potoków (pipes) dla każdego Workera:
    int wt_helper[N][2];  // Worker -> Helper
    int ht_worker[N][2];  // Helper -> Worker
    int ht_helper[N][2];  // Helper -> następny Helper

    // Tworzymy potoki i ustawiamy tryb nieblokujący dla odczytu
    for (int i = 0; i < N; ++i) {
        pipe(wt_helper[i]);
        pipe(ht_worker[i]);
        pipe(ht_helper[i]);
    }

    // Tworzenie procesów pomocników (Helperów)
    for (int i = 0; i < N; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            int id = i;
            int next = (i + 1) % N;  // ID następnego Helpera
            std::map<UnitType, std::queue<Unit>> buffer;  // Bufor przechowujący jednostki
            Unit buffer1;


            // Każdy Helper zamyka potoki nieprzypisane do siebie
            for (int j = 0; j < N; ++j) {
                if (j != id) {
                    close(wt_helper[j][0]); close(wt_helper[j][1]);
                    close(ht_worker[j][0]); close(ht_worker[j][1]);
                    if(j != next){
                        close(ht_helper[j][0]); close(ht_helper[j][1]);
                    }
                        
                }

            }

            // Zamknięcie niepotrzebnych końców potoków (zostają tylko przydatne)
            close(wt_helper[id][1]);  // Helper tylko odczytuje dane z Workera
            close(ht_worker[id][0]);  // Helper tylko pisze do Workera
            close(ht_helper[id][1]);  // Helper tylko odczytuje od poprzedniego Helpera
            close(ht_helper[next][0]);  // Helper tylko pisze do następnego Helpera

            while (true) {
                Unit u;

                // Odbieramy jednostki od Workera
                if (read_unit(wt_helper[id][0], u)) {
                    std::cout << "[Helper " << id << "] Received unit of type " << unitTypeToString(u.type)
                              << " from its worker " << std::endl << std::flush;
                    //buffer[u.type].push(u);  // Dodajemy jednostkę do bufora
                    buffer1 = u;
                }

                while(buffer1.type != id){
                    write_unit(ht_helper[next][1], buffer1);
                    std::cout << "[Helper " << id << "] Passed unit of type " << unitTypeToString(buffer1.type)
                                          << " to Helper " << next << std::endl << std::flush;
                    read_unit(ht_helper[id][0], u);
                    buffer1 = u;
                    std::cout << "[Helper " << id << "] Received unit of type " << unitTypeToString(buffer1.type)
                                          << " produced by Worker " << buffer1.producer_id << std::endl << std::flush;
                }

                write_unit(ht_worker[id][1], buffer1);
                std::cout << "[Helper " << id << "] Sent unit of type " << unitTypeToString(buffer1.type)
                                          << " to Worker " << id << std::endl << std::flush;
            }
            exit(0);
        }
    }

    // Tworzenie Workerów
    for (int i = 0; i < N; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            int id = i;
            UnitType expected = id;  // Oczekiwany typ jednostki dla każdego Workera

            // Zamykanie niepotrzebnych potoków
            for (int j = 0; j < N; ++j) {
                if (j != id) {
                    close(wt_helper[j][0]); close(wt_helper[j][1]);
                    close(ht_worker[j][0]); close(ht_worker[j][1]);
                }
            }

            close(wt_helper[id][0]);  // Worker pisze do Helpera
            close(ht_worker[id][1]);  // Worker czyta od Helpera
            srand(time(nullptr)+getpid());  // Inicjalizacja generatora liczb losowych

            while (true) {
                //sleep(2);  //sleep żeby spowolnić produkcję jednostek
                Unit u{static_cast<UnitType>(rand() % N), id};
                std::cout << "[Worker " << id << "] Produced unit of type: " << unitTypeToString(u.type) << std::endl << std::flush;
                // Wysyłamy jednostkę do odpowiedniego Helpera
                write_unit(wt_helper[id][1], u);
                std::cout << "[Worker " << id << "] Sent unit to Helper " << id << std::endl << std::flush;

                // Odczytujemy jednostkę z potoku, jeśli jest dostępna
                Unit rcv;
                if (read_unit(ht_worker[id][0], rcv)) {
                    std::cout << "[Worker " << id << "] Received and consumed unit of type: " << unitTypeToString(rcv.type) << " produced by Worker "<<rcv.producer_id<< std::endl << std::flush;
                }
            }
            exit(0);
        }
    }
    // Główna funkcja czeka na zakończenie wszystkich procesów
    for (int i = 0; i < 2 * N; ++i)
        wait(nullptr);

    return 0;
}
