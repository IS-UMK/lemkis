#include <thread>
#include <chrono>
#include <print>
#include <mutex>
#include <condition_variable>
#include <semaphore>
#include <atomic>
#include <random>
#include <esp_pthread.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Opis: Przykład pokazuje możliwość posługiwania się mechanizmami współbieżności
// systemu operacyjnego czasu rzeczywistego FreeRTOS używając interfejsów w języku C++.

// Możliwość wykorzystywania kodu i mechanizmów biblioteki standardowej języka C++ nie
// jest oczywista w systemach wbudowanych. Zwłaszcza biorąc pod uwagę różnorodną specyfikę
// sprzętu.

// Systemy wbudowane nie są konstrukcjami porównywalnymi ze zwykłymi komputerami
// na wiele sposobów. Jednocześnie nie wspierają dokładnie tych samych mechanizmów, jakie
// zostały pierwotnie przewidziane i wykorzystane w "dużych" komputerach. Tak więc zwykle
// nie można oczekiwać od nich porównywalnego sposobu programowania.

// Żeby rozszerzyć możliości wykorzystania sprzętu oraz efektywność programistów, producenci
// często czynią wysiłek, ażeby udostępnić znane programistom, wydajne, złożone mechanizmy
// pozwalające budować zaawansowane systemy w sposób zorganizowany i elastyczny.

// Tak też jest w przypadku niniejszego przykładu, a więc mikrokontrolera ESP32, którego
// cały mechanizm pracy jest fundamentalnie oparty o FreeRTOS. Firma Espressif obudowała
// w swoim SDK funkcje FreeRTOS'a oraz funkcje sprzętowe mikrokontrolera w odpowiednie
// interfejsy, zapewniając przy tym wsparcie dla jednego z najważniejszych języków
// programowania na systemy wbudowane, tj.: języka C++, nawet zapewniając wsparcie
// dla jego najnowszych standardów (obecnie C++23).

// Umożliwia to korzystanie z mechanizmów biblioteki standardowej C++, biblioteki szablonów,
// mechanizmów obiektowości, oraz mechanizmów programowania współbieżnego biblioteki standardowej
// w sposób bardzo zbliżony do klasycznego. Wrappery języka C++ obejmują mechanizm obsługi
// FreeRTOS'owych tasków interfejsem zarówno POSIX'owym, jak i biblioteki standardowej C++, a
// także prymitywów synchronizacyjnych i mechanizmów komunikacji. Poniższe przykłady je prezentują.

// Helper function to create pthread configuration
esp_pthread_cfg_t create_task_config(const char* name, int prio, int stack_size) {
    esp_pthread_cfg_t cfg = esp_pthread_get_default_config();
    cfg.thread_name = name;
    cfg.prio = prio;
    cfg.stack_size = stack_size;
    return cfg;
}

// =============================================================================
// Example 1: Task creation and preemption demonstration
// =============================================================================

void create_preemption_example() {
    std::print("Starting preemption example...\n");

    // High-priority thread (priority 3, 10ms delay)
    auto high_cfg = create_task_config("HighTask", 3, 2048);
    esp_pthread_set_cfg(&high_cfg);
    std::jthread high_thread([](std::stop_token st) {
        while (!st.stop_requested()) {
            std::print("High Priority Task Running\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    // Low-priority thread (priority 1, 100ms delay)
    auto low_cfg = create_task_config("LowTask", 1, 2048);
    esp_pthread_set_cfg(&low_cfg);
    std::jthread low_thread([](std::stop_token st) {
        while (!st.stop_requested()) {
            std::print("Low Priority Task Running\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    high_thread.detach();
    low_thread.detach();
}

// =============================================================================
// Example 2: Using semaphores
// =============================================================================

// Global synchronization objects
std::binary_semaphore interrupt_semaphore(0);
std::mutex shared_resource_mutex;
std::atomic<int> shared_resource{0};

void create_semaphore_example() {
    std::print("Starting semaphore example...\n");

    // Interrupt simulator thread
    auto int_cfg = create_task_config("IntSim", 1, 4096);
    esp_pthread_set_cfg(&int_cfg);
    std::jthread interrupt_simulator([](std::stop_token st) {
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
            std::print("Interrupt occurred!\n");
            interrupt_semaphore.release();
        }
    });

    // Event handler thread
    auto event_cfg = create_task_config("EventHandler", 2, 4096);
    esp_pthread_set_cfg(&event_cfg);
    std::jthread event_handler([](std::stop_token st) {
        while (!st.stop_requested()) {
            interrupt_semaphore.acquire();
            std::print("Event Handler: Processing interrupt event\n");
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            std::print("Event Handler: Event processed\n");
        }
    });

    // Shared resource access threads
    // Resource thread 1
    auto res1_cfg = create_task_config("Resource1", 2, 4096);
    esp_pthread_set_cfg(&res1_cfg);
    std::jthread resource_thread1([](std::stop_token st) {
        const int task_id = 1;
        while (!st.stop_requested()) {
            {
                std::lock_guard<std::mutex> lock(shared_resource_mutex);
                int current_value = shared_resource.load();
                std::print("Task {}: Accessing shared resource (value: {})\n", task_id, current_value);
                
                // Critical section - simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                shared_resource.store(current_value + 1);
                
                std::print("Task {}: Updated shared resource to {}\n", task_id, shared_resource.load());
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        }
    });

    // Resource thread 2
    auto res2_cfg = create_task_config("Resource2", 2, 4096);
    esp_pthread_set_cfg(&res2_cfg);
    std::jthread resource_thread2([](std::stop_token st) {
        const int task_id = 2;
        while (!st.stop_requested()) {
            {
                std::lock_guard<std::mutex> lock(shared_resource_mutex);
                int current_value = shared_resource.load();
                std::print("Task {}: Accessing shared resource (value: {})\n", task_id, current_value);
                
                // Critical section - simulate work
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                shared_resource.store(current_value + 1);
                
                std::print("Task {}: Updated shared resource to {}\n", task_id, shared_resource.load());
            }
            
            std::this_thread::sleep_for(std::chrono::milliseconds(1500));
        }
    });
    
    resource_thread1.detach();
    resource_thread2.detach();
    
    interrupt_simulator.detach();
    event_handler.detach();
}

// =============================================================================
// Example 4: Notification mechanism using condition variables
// =============================================================================

class TaskNotification {
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<uint32_t> notification_value{0};
    std::atomic<bool> has_notification{false};

public:
    void notify(uint32_t value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            notification_value.store(value);
            has_notification.store(true);
        }
        cv.notify_one();
    }
    
    uint32_t wait_for_notification() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return has_notification.load(); });
        
        uint32_t value = notification_value.load();
        has_notification.store(false);
        return value;
    }
};

void create_notification_example() {
    std::print("Starting notification example...\n");
    
    static TaskNotification notification;

    // Notifying thread
    auto notify_cfg = create_task_config("NotifyingTask", 2, 4096);
    esp_pthread_set_cfg(&notify_cfg);
    std::jthread notifying_thread([](std::stop_token st) {
        uint32_t counter = 0;
        while (!st.stop_requested()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            ++counter;
            notification.notify(counter);
            std::print("Notifying Task: Sent notification with value {}\n", counter);
        }
    });

    // Notified thread
    auto notified_cfg = create_task_config("NotifiedTask", 2, 4096);
    esp_pthread_set_cfg(&notified_cfg);
    std::jthread notified_thread([](std::stop_token st) {
        while (!st.stop_requested()) {
            uint32_t value = notification.wait_for_notification();
            std::print("Notified Task: Received notification with value {}\n", value);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });
    
    notifying_thread.detach();
    notified_thread.detach();
}

// =============================================================================
// Main application
// =============================================================================

extern "C" void app_main(void) {
    std::print("C++ FreeRTOS Examples Starting...\n");
    
    // Choose which example to run (uncomment one):
    
    //create_preemption_example();
    //create_semaphore_example();
    create_notification_example();
    
    // Keep main thread alive
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
