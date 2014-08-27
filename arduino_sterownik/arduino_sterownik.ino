#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "stdio.h"

int relay = 8;

// Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10
//RF24 radio(7, 8);
RF24 radio(9,10);

//byte addresses[][6] = {"1Node", "2Node"};
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup() {
  Serial.begin(57600);
  pinMode(relay, OUTPUT);
  digitalWrite(relay, HIGH);
  
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("*** PRESS 'T' 2 to begin transmitting to the other node\n\r");

  // Setup and configure rf radio
  radio.begin();                          // Start up the radio

  radio.setDataRate(RF24_250KBPS);
/*
  
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15, 15);               // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);


 // radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
*/

    radio.setRetries( 15, 15);
    radio.setChannel(0x4c);
    radio.setPALevel(RF24_PA_MAX);
    radio.setPALevel(RF24_PA_MAX);

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    radio.startListening();
    radio.printDetails();

  printf("setup done \n\r");
}

void loop() {

    radio.stopListening();                                    // First, stop listening so we can talk.


    printf("Now sending \n\r");

    unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
    
    const int n = snprintf(NULL, 0, "%lu", time);
    char buf[n+1];
    int c = snprintf(buf, n+1, "%lu", time);

    if (!radio.write( &buf, sizeof(buf) )) {
      printf("failed.\n\r");
    }
    printf("2\n\r");
    radio.startListening();                                    // Now, continue listening

    unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
    boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not

    while ( ! radio.available() ) {
      //printf("..\n\r");      // While nothing is received
      if (micros() - started_waiting_at > 500000 ) {           // If waited longer than 200ms, indicate timeout and exit while loop
        timeout = true;
        break;
      }
    }

    if ( timeout ) {                                            // Describe the results
      printf("Failed, response timed out.\n\r");
    } else {
      unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Sent %lu, Got response %lu, round-trip delay: %lu microseconds\n\r", time, got_time, micros() - got_time);
    }

    // Try again 1s later
    delay(1000);
 
}
