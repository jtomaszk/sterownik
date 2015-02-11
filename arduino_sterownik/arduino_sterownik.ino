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
//  pinMode(relay, OUTPUT);
//  digitalWrite(relay, HIGH);
  
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("*** PRESS 'T' 2 to begin transmitting to the other node\n\r");

  // Setup and configure rf radio
  radio.begin();                          // Start up the radio
radio.enableDynamicPayloads();
//  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);     
/*
  
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15, 15);               // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);


 // radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging
*/

    radio.setRetries( 15, 15);
   // radio.setPayloadSize(8);
    
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    
    
    radio.setChannel(70);
    radio.setPALevel(RF24_PA_MAX);
    radio.setCRCLength(RF24_CRC_8);

    radio.startListening();
    radio.printDetails();

  printf("setup done \n\r");
}
uint8_t loopCounter=0;  
void loop() {
  unsigned long send_time = millis() ;
  
  bool timeout=0;
  while ( radio.available() ) {
     // Check for timeout and exit the while loop
      if ( millis() - send_time > 50000 ) {
          //Serial.println("Timeout!!!");
          timeout = 1;
      }
  }
  loopCounter ++;
    if (loopCounter > 6) {
      
radio.powerUp();
    radio.stopListening();                                    // First, stop listening so we can talk.

char outBuffer[320]="xxxxxxxxxxxx";

    unsigned long time = millis();
    printf("Now sending %lu...",time);
//    bool ok = radio.write( &time, sizeof(unsigned long) );
  bool ok = radio.write( outBuffer, strlen(outBuffer));
    
    radio.startListening();  
    
    if (!ok) {
      printf("failed.\n\r");
    } else {
      printf("ok...\n\r");
                                        // Now, continue listening
  
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
    }
    // Try again 1s later
    delay(1000);
 loopCounter = 0;
    }
}
