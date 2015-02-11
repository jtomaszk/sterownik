/*
 * Copyright (C) 2014 jt
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "DHT.h"
#include "string_helper.h"
#include "sprotocol.h"

DHT dht;

// Hardware configuration
RF24 radio(8,7);

byte addresses[][6] = {"1Node","2Node"};


// Set up roles to simplify testing 
boolean role;                                    // The main role variable, holds the current role identifier
boolean role_ping_out = 1, role_pong_back = 0;   // The two different roles.

void setup() {
  Serial.begin(9600);
  printf_begin();
  
  dht.setup(2);
  
  // Setup and configure rf radio
  radio.begin();                          // Start up the radio

radio.enableDynamicPayloads();
//  radio.setDataRate(RF24_250KBPS);
  radio.setAutoAck(1);     
/*
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  */
  radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging


    radio.setRetries( 15, 15);
   // radio.setPayloadSize(8);
    
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
    
    
    radio.setChannel(70);
    radio.setPALevel(RF24_PA_MAX);
    radio.setCRCLength(RF24_CRC_8);

  Serial.println(dht.getStatusString());
  
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);
}

void write(char msg[]) {
  radio.stopListening();
  
  printf("Now sending \n\r");


  unsigned long time = micros();                             // Take the time, and send it.  This will block until complete
  if (!radio.write( msg, 255 )){  printf("failed.\n\r");  }
//  if (!radio.write( &time, sizeof(unsigned long) )){  printf("failed.\n\r");  }    
  radio.startListening();                                    // Now, continue listening
  
  unsigned long started_waiting_at = micros();               // Set up a timeout period, get the current microseconds
  boolean timeout = false;                                   // Set up a variable to indicate if a response was received or not
  
  while ( ! radio.available() ){                             // While nothing is received
    if (micros() - started_waiting_at > 200000 ){            // If waited longer than 200ms, indicate timeout and exit while loop
        timeout = true;
        break;
    }      
  }
      
  if (timeout) {                                             // Describe the results
      printf("Failed, response timed out.\n\r");
  } else{
      unsigned long got_time;                                 // Grab the response, compare, and send to debugging spew
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Sent %lu, Got response %lu, round-trip delay: %lu microseconds\n\r",time,got_time,micros()-got_time);
  }
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
    }
}

void readTemp(char out[]) {
  sprintf(out, "H=");
  fmtDouble(dht.getHumidity(), 4, out+2, 9);
  sprintf(out+9, ";T=");  
  fmtDouble(dht.getTemperature(), 4, out+12, 10);
  sprintf(out+19, ";\n");  
}

void loop(void){
    char msg[255] = "";
    
    delay(dht.getMinimumSamplingPeriod());
    readTemp(msg);
    Serial.print(msg);
    write(msg);

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

    delay(2000);
}

//  delay(dht.getMinimumSamplingPeriod());
