#include <Boards.h>
#include <Firmata.h>

#include <SoftwareSerial.h>

SoftwareSerial mySerial(8,14);

int redPin = 9;   // Red LED,   connected to digital pin 9
int grnPin = 10;  // Green LED, connected to digital pin 10
int bluPin = 11;  // Blue LED,  connected to digital pin 11

int relayPin = 12;

const int nCodes = 3;

byte codes[nCodes][8] = {{0x00, 0x00, 0x00, 0x00, 0x00, 0, 255, 0},
                         {0x00, 0x00, 0x00, 0x00, 0x00, 0, 255, 0},
                         {0x00, 0x00, 0x00, 0x00, 0x00, 0, 255, 0}};

byte color_err[3] = { 255, 0, 0 };
byte color_blank[3] = { 0, 0, 0 };

void setup()
{
  pinMode(redPin, OUTPUT);   // sets the pins as output
  pinMode(grnPin, OUTPUT);   
  pinMode(bluPin, OUTPUT); 
  pinMode(relayPin, OUTPUT);
  
  Serial.begin(19200);
  while (!Serial) {
    ;
  }
  Serial.write("Lets do this");
  mySerial.begin(9600);
}

void loop () {
  byte i = 0;
  byte val = 0;
  byte code[6];
  byte checksum = 0;
  byte bytesread = 0;
  byte tempbyte = 0;
  
  if(mySerial.available() > 0) {
    if((val = mySerial.read()) == 2) {                  // check for header 
      bytesread = 0; 
      while (bytesread < 12) {                        // read 10 digit code + 2 digit checksum
        if( mySerial.available() > 0) { 
          val = mySerial.read();
          if((val == 0x0D)||(val == 0x0A)||(val == 0x03)||(val == 0x02)) { // if header or stop bytes before the 10 digit reading 
            break;                                    // stop reading
          }

          // Do Ascii/Hex conversion:
          if ((val >= '0') && (val <= '9')) {
            val = val - '0';
          } else if ((val >= 'A') && (val <= 'F')) {
            val = 10 + val - 'A';
          }

          // Every two hex-digits, add byte to code:
          if (bytesread & 1 == 1) {
            // make some space for this hex-digit by
            // shifting the previous hex-digit with 4 bits to the left:
            code[bytesread >> 1] = (val | (tempbyte << 4));

            if (bytesread >> 1 != 5) {                // If we're at the checksum byte,
              checksum ^= code[bytesread >> 1];       // Calculate the checksum... (XOR)
            };
          } else {
            tempbyte = val;                           // Store the first hex digit first...
          };

          bytesread++;                                // ready to read next digit
        } 
      } 

      // Output to Serial:

      if (bytesread == 12) {                          // if 12 digit read is complete
        Serial.print("5-byte code: ");

        Serial.println();

        Serial.print("Checksum: ");
        Serial.print(code[5], HEX);
        Serial.println(code[5] == checksum ? " -- passed." : " -- error.");
        Serial.println();
        if (check_code(code)) {
          Serial.println("Its adam");
        }
      }

      bytesread = 0;
    }
  }
}

boolean check_code(byte in_code[8]) {
  boolean correct = true;
  byte color[3];
  int i = 0;
  int c = 0;

  for (c=0; c<nCodes; c++) { 
    correct = true;
    for (i=0; i<5; i++) {
      if (in_code[i] < 16) Serial.print("0");
      Serial.print(in_code[i], HEX);
      Serial.print(" ");
      if (codes[c][i] != in_code[i])
      {
        correct = false;
      }
    }  
    if (correct){
      Serial.println("Color:");
      for (i=5; i<8; i++) {
        color[i-5] = codes[c][i];
        Serial.println(color[i-5]);
      }
      set_led(color);
      fire_relay();
      flash_led(color);
      set_led(color_blank);
      return true;
    }
  }

  flash_led(color_err);
}

void set_led(byte color[3]) {
  int R = color[0];
  int G = color[1];
  int B = color[2];
  
  analogWrite(redPin, R);   // Write current values to LED pins
  analogWrite(grnPin, G);      
  analogWrite(bluPin, B);
}

void flash_led(byte color[3]) {
  
  int R = color[0];
  int G = color[1];
  int B = color[2];
  
  for (int i = 0; i < 10; i++) {
    
    analogWrite(redPin, R);   // Write current values to LED pins
    analogWrite(grnPin, G);      
    analogWrite(bluPin, B);
  
    delay(50);
  
    analogWrite(redPin, 0);   // Write current values to LED pins
    analogWrite(grnPin, 0);      
    analogWrite(bluPin, 0);    
    
    delay(50);
  }
}

void fire_relay() {
  digitalWrite(relayPin, HIGH);
  delay(500);
  digitalWrite(relayPin, LOW);
}
