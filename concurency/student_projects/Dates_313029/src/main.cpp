#include <print>
#include <vector>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include "../include/cafe/Cafe.hpp"
#include "../include/cafe/Process.hpp"

void redirect_output_to_console() {
    // Przekierowanie stdout do konsoli
    int fd = open("/dev/tty", O_WRONLY);
    if (fd != -1) {
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}

void cleanup_ipc_resources() {
    // Czyszczenie zasobów IPC
    sem_unlink("/cafe_mutex");
    sem_unlink("/cafe_cv");
    shm_unlink("/cafe_shm");
    std::println("[MAIN] IPC resources cleaned up");
}

int main() {
    constexpr int num_pairs = 3;
    constexpr unsigned simulation_duration_seconds = 3;
    
    // Wyczyść stare zasoby IPC 
    cleanup_ipc_resources();
    
    try {
        Cafe cafe;
        std::vector<pid_t> child_pids;

        std::println("[MAIN] Starting simulation with {} pairs", num_pairs);

        // Tworzenie procesów potomnych
        // i - ID pary
        // j - ID osoby w parze
        for (int i = 0; i < num_pairs; ++i) {
            for (int j = 0; j < 2; ++j) {

                pid_t pid = fork();

                if (pid == 0) { // Child process

                    redirect_output_to_console();

                    try {
                        //Każde dziecko tworzy swój obiekt Process i woła run
                        Process p(i, j, cafe);
                        std::println("[CHILD {}-{}] Started (PID: {})", i, j, getpid());
                        p.run();
                    } catch (const std::exception& e) {
                        std::println("[CHILD {}-{}] ERROR: {}", i, j, e.what());
                    }

                    _exit(0);

                }
                else if (pid > 0) {
                    
                    //Rodzic zapamiętuje każde dziecko
                    child_pids.push_back(pid);
                    std::println("[MAIN] Created process {}-{} (PID: {})", i, j, pid);
                    // Krótka przerwa między tworzeniem procesów
                    usleep(10000); // 10ms

                }
                else {
                    std::println("[MAIN] Fork failed!");
                    // Zakończ wszystkie dotychczas utworzone procesy
                    for (pid_t existing_pid : child_pids) {
                        kill(existing_pid, SIGTERM);
                        waitpid(existing_pid, nullptr, 0);
                    }
                    cleanup_ipc_resources();
                    return 1;
                }
            }
        }

        std::println("[MAIN] All processes created. Waiting for {} seconds...", simulation_duration_seconds);
        //Rodzic - proces główny śpi
        // W tym czasie dzieci randkują (ja wiem jak to brzmi)
        sleep(simulation_duration_seconds);

        std::println("[MAIN] Sending termination signals...");
        // Wyślij sygnały zakończenia do dzieci 
        for (pid_t pid : child_pids) {
            kill(pid, SIGTERM);
        }

        // Poczekaj na zakończenie wszystkich procesów
        for (pid_t pid : child_pids) {
            int status;
            if (waitpid(pid, &status, 0) > 0) {
                std::println("[MAIN] Process {} terminated with status {}", pid, status);
            }
        }

        std::println("[MAIN] All processes terminated. Cleaning up...");
        cleanup_ipc_resources();
        std::println("[MAIN] Simulation finished successfully");
        return 0;

    } catch (const std::exception& e) {
        std::println("[MAIN] ERROR: {}", e.what());
        
        // Próba zakończenia wszystkich procesów w przypadku błędu
        for (pid_t pid : std::vector<pid_t>()) {
            kill(pid, SIGKILL);
            waitpid(pid, nullptr, 0);
        }
        
        cleanup_ipc_resources();
        return 1;
    }
}