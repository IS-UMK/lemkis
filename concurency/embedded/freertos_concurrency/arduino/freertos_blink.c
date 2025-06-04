#include <avr/io.h>
#include <util/delay.h>
#include "Arduino_FreeRTOS.h"
#include "task.h"

// Opis: Podstawowy przykład równoległego wykonywania zadań z wykorzystaniem
// systemu operacyjnego czasu rzeczywistego. RTOS wykorzystuje planistę
// czasu rzeczywistego do dynamicznego przełączania wykonywanych zadań
// poprzez mechanizm wywłaszczania oraz odpowiednie polityki kolejkowania.

// Do zarządzania zadaniami i pilnowaniem czasowości wykorzystuje timery
// sprzętowe wysokiej rozdzielczości.

// Ponadto ułatwia zarządzanie zadaniami poprzez separację każdego zadania
// w bardzo wyraźny sposób. Każde zadanie może być traktowane jako swój własny
// oddzielny program/wątek, której to separacji nie ma w klasycznym sensie w
// rozwiązaniach bare-metal.

// Task function that blinks LED on PB3 (Arduino pin 11) every 500ms
void vBlinkTask1(void *pvParameters) {
    // Configure PB3 (pin 11) as output
    // DDRB = Data Direction Register B, bit 3 = pin 11
    DDRB |= (1 << DDB3);
    
    // Task loop - must be infinite
    while(true) {
        // Turn LED on - set PB3 high
        PORTB ^= (1 << PB3);

        // Wait 500ms using FreeRTOS delay
        vTaskDelay(pdMS_TO_TICKS(500));
        
        // Turn LED off - set PB3 low
        PORTB ^= (1 << PB3);

        // Wait 500ms using FreeRTOS delay
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Task function that blinks LED on PB0 (Arduino pin 8) every 100ms
void vBlinkTask2(void *pvParameters) {
    // Configure PB0 (pin 8) as output
    // DDRB = Data Direction Register B, bit 0 = pin 8
    DDRB |= (1 << DDB3);
    
    // Task loop - must be infinite
    while(true) {
        // Turn LED on - set PB0 high
        PORTB ^= (1 << PB0);

        // Wait 500ms using FreeRTOS delay
        vTaskDelay(pdMS_TO_TICKS(200));
        
        // Turn LED off - set PB0 low
        PORTB ^= (1 << PB0);

        // Wait 500ms using FreeRTOS delay
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void setup() {

    // Configure all GPIO pins as outputs and set them low
    DDRB = 0xFF;  // Port B (pins 8-13)
    DDRC = 0xFF;  // Port C (pins A0-A5)
    DDRD = 0xFF;  // Port D (pins 0-7)
    
    PORTB = 0x00; // Set all Port B pins low
    PORTC = 0x00; // Set all Port C pins low
    PORTD = 0x00; // Set all Port D pins low

    // Create the blink task
    xTaskCreate(
        vBlinkTask1,         // Task function
        "Blink1",            // Task name
        128,                 // Stack size in words
        NULL,                // Task parameters
        1,                   // Task priority
        NULL                 // Task handle
    );

    xTaskCreate(
        vBlinkTask2,         // Task function
        "Blink2",            // Task name
        128,                 // Stack size in words
        NULL,                // Task parameters
        1,                   // Task priority
        NULL                 // Task handle
    );
    
    // Start the FreeRTOS scheduler
    vTaskStartScheduler();

}

// Unreachable - scheduler is blocking
void loop() {

}
