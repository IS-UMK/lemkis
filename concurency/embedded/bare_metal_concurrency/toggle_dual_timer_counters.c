// Bardziej zaawansowany i "prawidłowy" przykład równoległego wykonywania zadań
// Inkrementowanie liczników za pomocą przerwania zegarowego + prosta maszyna stanów
// Wersja bare-metal na ATMega328p

// Opis: Wykorzystywanie przerwań czasowych jest jedną z najbardziej właściwych
// metod symulowania równoległości oraz dotrzymywania terminów czasowych.
// Niestety im więcej przerwań i im bardziej złożony jest system, tym bardziej
// zarządzanie zachowaniem systemu staje się skomplikowane.

#include <avr/io.h>
#include <avr/interrupt.h>

// Define timer period and pin toggle intervals
#define TIMER_PERIOD_MS     100
#define PIN8_PERIOD_MS      200
#define PIN11_PERIOD_MS     500

// Structure to hold pin control data
typedef struct {
    uint8_t pin_number;
    uint8_t current_counter;
    uint8_t counter_trigger_val;
} pin_control_t;

// Global pin control structures
volatile pin_control_t pin8_ctrl = {8, 0, PIN8_PERIOD_MS / TIMER_PERIOD_MS};
volatile pin_control_t pin11_ctrl = {11, 0, PIN11_PERIOD_MS / TIMER_PERIOD_MS};

// Timer1 Compare Match A interrupt service routine (100ms period)
ISR(TIMER1_COMPA_vect) {
    // Handle pin 8 (200ms interval, counter target = 2)
    pin8_ctrl.current_counter++;
    if (pin8_ctrl.current_counter >= pin8_ctrl.counter_trigger_val) {
        PORTB ^= (1 << PB0);
        pin8_ctrl.current_counter = 0;
    }
    
    // Handle pin 11 (500ms interval, counter target = 5)
    pin11_ctrl.current_counter++;
    if (pin11_ctrl.current_counter >= pin11_ctrl.counter_trigger_val) {
        PORTB ^= (1 << PB3);
        pin11_ctrl.current_counter = 0;
      
    }
}

int main(void) {
    DDRB = 0xFF;
    DDRC = 0xFF;
    DDRD = 0xFF;
    
    PORTB = 0x00;
    PORTC = 0x00;
    PORTD = 0x00;
    
    // Configure Timer1 for 100ms interval
    // Using CTC mode with OCR1A as TOP
    TCCR1A = 0;                    // Clear Timer1 control register A
    TCCR1B = (1 << WGM12);         // Set CTC mode (Clear Timer on Compare Match)
    
    // Set prescaler to 1024 and start timer
    // Timer frequency = 16MHz / 1024 = 15625 Hz
    // For 100ms: OCR1A = (15625 * 0.1) - 1 = 1562
    TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler 1024
    OCR1A = 1562;                  // Set compare value for 100ms
    
    // Enable Timer1 Compare Match A interrupt
    TIMSK1 = (1 << OCIE1A);
    
    // Enable global interrupts
    sei();
    
    // Main loop - empty, everything handled by interrupt
    while (1) {
        // Do nothing, interrupt handles the toggling with counters
        _delay_ms(10000);
    }
    
    return 0;
}
