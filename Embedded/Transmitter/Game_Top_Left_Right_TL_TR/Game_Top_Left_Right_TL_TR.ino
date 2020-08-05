

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
        HEADER FILES
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include<avr/io.h>
#include<avr/interrupt.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
       GLOBAL VARIABLES
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
   Various codes to execute after a gesture is detected
*/
uint32_t code_ON = 0b10110010010011011111001000001101;
uint32_t code_OFF = 0b10110010010011010110001010011101;

uint32_t code = 0;



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
          ISRs
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
   This routine will toggle the IR LED pin ON/OFF
*/
ISR(TIMER0_COMPA_vect) {
  PORTD ^= (1 << 2);
}

/*
   This routuine will stop the current job being executed when transmitting the IR signal.
*/
ISR(TIMER1_COMPA_vect) {
  TCCR1B = 0x00;
  TCCR0B = 0x00;
  PORTD = 0x00;
}



/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      TRANSMITTER FUNCTIONS
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
  Function to send Start pulse.
  Pulse is 9ms long burst of 38kHz IR light.
*/
void start_pulse() {
  OCR1A = 18000;
  TCCR1B = (1 << CS11);
  TCCR0B = (1 << CS00);
  TCNT1 = 0;
  TCNT0 = 0;
  PORTD = 0b00000100;
  while (TCCR1B & (1 << CS11));
}

/*
   Function to cause delay.
   Pulse is retained low for 4.5ms.
*/
void start_delay() {
  PORTD = 0x00;
  OCR1A = 9000;
  TCCR1B = (1 << CS11);
  TCNT1 = 0;
  while (TCCR1B & (1 << CS11));
}

/*
   Function to send 562.5us burst of IR signal showing start of a bit.
*/
void code_pulse() {
  OCR1A = 9000;
  TCCR1B = (1 << CS10);
  TCCR0B = (1 << CS00);
  TCNT1 = 0;
  TCNT0 = 0;
  PORTD = 0b00000100;
  while (TCCR1B & (1 << CS10));
}

/*
   Function to retain signal line low as per the time passed in time_tic.
   If bit is 1, it is retained low for 1687.5us
   If bit is 0, it is retained low for 562.5us
*/
void delay_bit(int time_tic) {
  PORTD = 0x00;
  OCR1A = time_tic;
  TCCR1B = (1 << CS10);
  TCNT1 = 0;
  while (TCCR1B & (1 << CS10));
}



/*
   Function to send code according to the arguement passed.
*/
void send_code(int code_value) {
  if (code_value == 1) {
    code = code_ON;
  }
  else if (code_value == 0) {
    code = code_OFF;
  }

  start_pulse();
  start_delay();
  for (int i = 0; i < 32; ++i) {
    code_pulse();
    if (code & 0x80000000) {
      delay_bit(27000);
    }
    else {
      delay_bit(9000);
    }
    code <<= 1;
  }
  code_pulse();
  code = 0;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      MAIN FUNCTION
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main() {


  DDRB = 0xFF;                                               //PORTB OUTPUT



  TCCR0A = (1 << WGM01);                                     //Timer0 CTC mode
  TCCR1A = (1 << WGM12);                                     //Timer1 CTC mode
  //Enable interrupts on compare match vect A for Timer 1 and 0
  TIMSK0 = (1 << OCIE0A);
  TIMSK1 = (1 << OCIE1A);


  OCR0A = 210;

  sei();                                                     //Enable global interrupts.

  DDRD = 0b00000100;                                         //IR Led is connceted to PIN 2(PORT D).

  PORTD = 0x00;                                              //LED Off

  Serial.begin(9600);
  while (1) {
    Serial.println("Off");
    send_code(0);
    _delay_ms(5000);
    Serial.println("On");
    send_code(1);
    _delay_ms(5000);


  }
  PORTD = 0x00;
  return 0;
}
