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
  radio.setAutoAck(1);                    // Ensure autoACK is enabled
  radio.setRetries(15,15);                // Max delay between retries & number of retries
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);
  
  radio.startListening();                 // Start listening
  radio.printDetails();                   // Dump the configuration of the rf unit for debugging

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
    
    delay(2000);
}

//  delay(dht.getMinimumSamplingPeriod());

