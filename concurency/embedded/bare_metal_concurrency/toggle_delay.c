// Najprostszy przykład wykonywania zadań w czasie
// Okresowe uśpienie programu
// Wersja bare-metal

// Opis: Najprostsze wykonywanie zadania z zadanym okresem czasu wykonania
// Nie pozwala na jakąkolwiek równoległość. Blokuje system w uśpieniu.

#include <avr/io.h>
#include <util/delay.h>

int main(void) {
    DDRB = 0xFF;
    PORTB = 0x00;
    
    while (1) {
        // Toggle pin 11 (PB3)
        PORTB ^= (1 << PB3);
        // Wait 500ms
        _delay_ms(500);
    }
    
    return 0;
}
