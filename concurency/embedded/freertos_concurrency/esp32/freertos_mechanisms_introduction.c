#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/timers.h"
#include "esp_random.h"

// Opis: Przykład wprowadza mechanizmy współbieżnego wykonywania zadań
// z wykorzystaniem systemu operacyjnego czasu rzeczywistego FreeRTOS.
// Prezentuje też mechanizmy synchronizacji oraz komunikacji między
// zadaniami.


// =============================================================================
// Przykład 1: Tworzenie zadań i demonstracja wywłaszczania.
// =============================================================================

// Definicja zadania z wysokim priorytetem
// Zadanie wywłaszczy zadanie z niskim priorytetem
void vHighPriorityTask(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(10);
    
    // Główna pętla zadania
    while(true) {
        printf("High Priority Task Running\n");
        
        // Opóźnienie
        vTaskDelay(xDelay);
    }
}

// Definicja zadania z niskim priorytetem
// Zadanie zostanie wywłaszczone przez zadanie z wyższym priorytetem
void vLowPriorityTask(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(100);
    
    // Główna pętla zadania
    while(true) {
        printf("Low Priority Task Running\n");
        
        // Opóźnienie
        vTaskDelay(xDelay);
    }
}

// Utworzenie zadań z przykładu
void create_preemption_example(void) {
    xTaskCreate(vHighPriorityTask, "HighTask", 2048, NULL, 3, NULL);
    xTaskCreate(vLowPriorityTask, "LowTask", 2048, NULL, 1, NULL);
}

// =============================================================================
// Przykład 2: Komunikacja poprzez kolejkę.
// =============================================================================

// Utworzenie uchwytu do kolejki
QueueHandle_t xQueue;

// Struktura danych z sensora temperatury
// Zawiera ID sensora, wartość pomiaru temperatury i moment czasu pomiaru
typedef struct {
    int sensor_id;
    float temperature;
    uint32_t timestamp;
} SensorData_t;

// Zadanie symulujące zbieranie danych pomiarowych z sensora
void vSensorTask(void *pvParameters) {
    // Utworzenie zmiennej z danymi pomiarowymi
    SensorData_t data;
    // Pobranie identyfikatora sensora z obszaru pamięci zawierającego parametry wywołania
    int sensor_id = (int)(intptr_t)pvParameters;
    
    while(true) {
        // Symulacja dokonania pomiaru
        data.sensor_id = sensor_id;
        data.temperature = 20.0 + (esp_random() % 1000) / 100.0f;  // 20-30°C
        data.timestamp = xTaskGetTickCount();
        
        // Zapis danych pomiarowych do kolejki
        // Domyślnie blokujące dopóki kolejka się nie zwolni lub nie minie czas
        if(xQueueSend(xQueue, &data, pdMS_TO_TICKS(100)) == pdPASS) {
            printf("Sensor %d: Sent temp %.1f°C\n", sensor_id, data.temperature);
        } else {
            printf("Sensor %d: Queue full!\n", sensor_id);
        }
        
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// Zadanie symulujące przetwarzanie danych
void vDataProcessorTask(void *pvParameters) {
    SensorData_t received_data;
    
    while(true) {
        // Oczekiwanie na dane z kolejki
        // Domyślnie blokuje bez ograniczeń czasowych
        if(xQueueReceive(xQueue, &received_data, portMAX_DELAY) == pdPASS) {
            printf("Processor: Received from sensor %d: %.1f°C at tick %lu\n",
                   received_data.sensor_id, 
                   received_data.temperature,
                   received_data.timestamp);
            
            // Przetworzenie danych
            if(received_data.temperature > 25.0) {
                printf("Warning: High temperature detected!\n");
            }
        }
    }
}

// Utworzenie przykładu z kolejką
void create_queue_example(void) {
    // Utworzenie kolejki na 10 danych z sensorów
    xQueue = xQueueCreate(10, sizeof(SensorData_t));
    
    if(xQueue != NULL) {
        // Utworzenie zadań związanych z sensorami
        xTaskCreate(vSensorTask, "Sensor1", 2048, (void*)(intptr_t)1, 2, NULL);
        xTaskCreate(vSensorTask, "Sensor2", 2048, (void*)(intptr_t)2, 2, NULL);
        xTaskCreate(vDataProcessorTask, "Processor", 2048, NULL, 3, NULL);
    }
}

// =============================================================================
// Przykład 3: Użycie semaforów.
// =============================================================================

// Utworzenie uchwytów do semafora i mutexa
SemaphoreHandle_t xBinarySemaphore;
SemaphoreHandle_t xMutex;
volatile int shared_resource = 0;

// Zadanie symulujące przerwanie
void vInterruptSimulator(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(3000);
    
    while(true) {
        vTaskDelay(xDelay);
        
        // Symulacja przerwania i przekazanie semafora
        printf("Interrupt occurred!\n");
        xSemaphoreGive(xBinarySemaphore);
    }
}

// Zadanie obsługujące zdarzenia
void vEventHandlerTask(void *pvParameters) {
    while(true) {
        // Oczekiwanie na otrzymanie semafora (zajście przerwania)
        if(xSemaphoreTake(xBinarySemaphore, portMAX_DELAY) == pdTRUE) {
            printf("Event Handler: Processing interrupt event\n");
            
            // Symulacja przetwarzania zdarzenia
            vTaskDelay(pdMS_TO_TICKS(500));
            printf("Event Handler: Event processed\n");
        }
    }
}

// Zadanie używające współdzielonego zasobu
void vSharedResourceTask(void *pvParameters) {
    int task_id = (int)(intptr_t)pvParameters;
    
    while(true) {
        // Pobranie mutexa żeby uzyskać dostęp do zasobu
        if(xSemaphoreTake(xMutex, pdMS_TO_TICKS(1000)) == pdTRUE) {
            printf("Task %d: Accessing shared resource (value: %d)\n", task_id, shared_resource);
            
            // Sekcja krytyczna - modyfikacja zasobu
            int temp = shared_resource;
            vTaskDelay(pdMS_TO_TICKS(100)); // Symulacja pracy
            shared_resource = temp + 1;
            
            printf("Task %d: Updated shared resource to %d\n", task_id, shared_resource);
            
            // Zwolnienie mutexa
            xSemaphoreGive(xMutex);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

// Utworzenie przykładu z semaforami
void create_semaphore_example(void) {
    // Utworzenie semafora binarnego i mutexa
    xBinarySemaphore = xSemaphoreCreateBinary();
    xMutex = xSemaphoreCreateMutex();
    
    if(xBinarySemaphore != NULL && xMutex != NULL) {
        xTaskCreate(vInterruptSimulator, "InterruptSimulator", 2048, NULL, 1, NULL);
        xTaskCreate(vEventHandlerTask, "EventHandler", 2048, NULL, 2, NULL);
        xTaskCreate(vSharedResourceTask, "ResourceTask1", 2048, (void*)(intptr_t)1, 2, NULL);
        xTaskCreate(vSharedResourceTask, "ResourceTask2", 2048, (void*)(intptr_t)2, 2, NULL);
    }
}

// =============================================================================
// Przykład 4: Mechanizm powiadamiania.
// =============================================================================

// Uchwyt do zadania które będzie powiadamiane
TaskHandle_t xTaskToNotify = NULL;

// Zadanie wysyłające powiadomienia
void vNotifyingTask(void *pvParameters) {
    const TickType_t xDelay = pdMS_TO_TICKS(2000);
    uint32_t notification_value = 0;
    
    while(true) {
        vTaskDelay(xDelay);
        
        notification_value++;
        // Wysłanie powiadomienia z wartością
        if(xTaskToNotify != NULL) {
            xTaskNotify(xTaskToNotify, notification_value, eSetValueWithOverwrite);
            printf("Notifying Task: Sent notification with value %lu\n", notification_value);
        }
    }
}

// Zadanie odbierające powiadomienia
void vNotifiedTask(void *pvParameters) {
    uint32_t notification_value;
    
    while(true) {
        // Oczekiwanie na powiadomienie
        if(xTaskNotifyWait(0, 0, &notification_value, portMAX_DELAY) == pdTRUE) {
            printf("Notified Task: Received notification with value %lu\n", notification_value);
            
            // Przetwarzanie powiadomienia
            vTaskDelay(pdMS_TO_TICKS(500));
        }
    }
}

// Utworzenie przykładu z powiadomieniami zadań
void create_notification_example(void) {
    // Utworzenie zadania odbierającego powiadomienia
    xTaskCreate(vNotifiedTask, "NotifiedTask", 2048, NULL, 2, &xTaskToNotify);
    
    // Utworzenie zadania wysyłającego powiadomienia
    xTaskCreate(vNotifyingTask, "NotifyingTask", 2048, NULL, 2, NULL);
}

// =============================================================================
// Przykład 5: Praca wielordzeniowa.
// =============================================================================

SemaphoreHandle_t print_mutex;

void safe_print(const char *message) {
    if (xSemaphoreTake(print_mutex, portMAX_DELAY)) {
        printf("%s\n", message);
        xSemaphoreGive(print_mutex);
    }
}

void vCore0Task(void *pvParameters) {
    while (1) {
        safe_print("Hello from Core 0");
        vTaskDelay(pdMS_TO_TICKS(0));
    }
}

void vCore1Task(void *pvParameters) {
    while (1) {
        safe_print("Hello from Core 1");
        vTaskDelay(pdMS_TO_TICKS(0));
    }
}

void create_multicore_example() {
    print_mutex = xSemaphoreCreateMutex();
    if (print_mutex == NULL) {
        printf("Failed to create mutex\n");
        return;
    }

    xTaskCreatePinnedToCore(vCore0Task, "Core0Task", 2048, NULL, 1, NULL, 0); // Core 0
    xTaskCreatePinnedToCore(vCore1Task, "Core1Task", 2048, NULL, 1, NULL, 1); // Core 1
}

// =============================================================================
// Główna aplikacja
// =============================================================================

// function, name, stack size, parameters, priority, handle
void app_main(void) {
    printf("FreeRTOS Examples Starting...\n");
    
    // Choose which example to run (uncomment one):
    
    //create_preemption_example();
    //create_queue_example();
    //create_semaphore_example();
    //create_notification_example();
    create_multicore_example();
    
    // FreeRTOS scheduler is automatically started in app_main
}

