// Prosty przykład równoległego wykonywania zadań
// Odliczanie czasu w pętli wraz z instrukcjami warunkowymi
// Wersja bare-metal

// Opis: Odliczanie czasu z wykorzystaniem sprzętowego licznika.
// Sprawdzanie czy "nadszedł" czas na wykonanie zadania.

// Niestety, w najlepszym przypadku powoduje nieregularności czasowe.
// W najgorszym przypadku wykonywania wielu długotrwałych operacji,
// nie pozwala na jakiekolwiek przełączanie, a więc i spełnienie wymagań
// czasowych.

#include <avr/io.h>

int main(void) {
    DDRB = 0xFF;
    PORTB = 0x00;
    
    unsigned long lastTogglePin1 = 0;
    unsigned long lastTogglePin2 = 0;
    
    while (1) {
        unsigned long currentTime = millis();
        
        // Check pin 11 (500ms interval)
        if (currentTime - lastTogglePin1 >= 500) {
            PORTB ^= (1 << PB3);
            lastTogglePin1 = currentTime;
        }
        
        // Check pin 8 (200ms interval)
        if (currentTime - lastTogglePin2 >= 200) {
            PORTB ^= (1 << PB0);
            lastTogglePin2 = currentTime;
        }
    }
    
    return 0;
}
