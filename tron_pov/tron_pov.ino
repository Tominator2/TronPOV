/*

  This sketch reads data from an array and outputs it to 10 LEDs
  for use as a Persistence of Vision (POV) display.
  
  You can find construction details and more information about
  the display here:
  
  http://www.tomtilley.net/projects/tron-pov/
  
  Digital pins are used for the 10 LEDs and for the pattern change/select switch.
  
  Digital Pins:
  3  - input  - pattern select switch
  4  - output - LED 10
  5  - output - LED 9
  6  - output - LED 8
  7  - output - LED 7
  8  - output - LED 6
  9  - output - LED 5
  10 - output - LED 4
  11 - output - LED 3
  12 - output - LED 2  
  13 - output - LED 1 

 Analog pins are used for communicating with an ADXL345 accelerometer using I2C.

 Analog Pins:
 3 - SDA on ADXL345 (Serial Data Line)
 4 - SCL on ADXL345 (Serial Clock Line)
  
 */

#include <Wire.h>:       // Library for I2C communication
#define DEVICE (0x53)    // ADXL345 device address
#define TO_READ (6)      // num of bytes to read each time (two bytes for each axis)

// Accelerometer variables
byte buff[TO_READ] ;    // 6 byte buffer for saving data read from the device
char str[512];          // string buffer to transform data before sending it to the serial port
int pos_thresh;         // positive acceleration threshold
int neg_thresh;         // negatiive acceleration threshold
boolean braking;        // decelleration flag

// POV variables
const int noPixels  = 10;   // the number of LEDs in our display
const int LEDpinmap[noPixels] = {4,5,6,7,8,9,10,11,12,13};  // maps the LEDs to the corresponding o/p pins
const int switchPin = 3;    // the pin connected to the change pattern switch

// Names of the header files that contain the display patterns
#include "tron2logo.h"     // TRON Legacy logo
#include "legacy.h"        // LEGACY
#include "daftpunk.h"      // DAFT PUNK
#include "tronlogo.h"      // TRON - 1982
#include "i_heart_u.h"     // I <3 U
#include "pacman.h"        // PAC-MAN
#include "invaders_up.h"   // Space Invaders - up
//#include "invaders_down.h" // Space Invaders - down

int direction = 0;  // initial text direction -  0: L -> R, 1: R -> L
int pattern   = 0;  // which display pattern to use

// LED bitmask
word mask[] = {word(B10000,B00000), 
               word(B01000,B00000), 
               word(B00100,B00000), 
               word(B00010,B00000), 
               word(B00001,B00000), 
               word(B00000,B10000), 
               word(B00000,B01000), 
               word(B00000,B00100), 
               word(B00000,B00010), 
               word(B00000,B00001)};
                 
// Pointers to the display patterns                      
word* displayPatterns[] = {tron2logo, 
                           legacy, 
                           daftpunk, 
                           tronlogo, 
                           i_heart_u, 
                           pacman, 
                           invaders_up};
                        
// Becuase we will be referring to the pattern arrays via pointers we also need
// to keep track of how many rows they contain
int rows[] = {sizeof(tron2logo)/2, 
              sizeof(legacy)/2, 
              sizeof(daftpunk)/2, 
              sizeof(tronlogo)/2, 
              sizeof(i_heart_u)/2, 
              sizeof(pacman)/2, 
              sizeof(invaders_up)/2};


//
// The setup() method runs once, when the sketch starts
//
void setup() {    
  
  // initialize the digital pins for the LED output:
  for (int i=0; i < noPixels; i++)
  {
    pinMode(LEDpinmap[i], OUTPUT); 
  } 
  
   // initialize the digital pin for switch input: 
  pinMode(switchPin, INPUT);
  
  Wire.begin();  // join I2C bus (address optional for master)
  
  // Turn on the ADXL345 accelerometer
  writeTo(DEVICE, 0x2D, 0);      
  writeTo(DEVICE, 0x2D, 16);
  writeTo(DEVICE, 0x2D, 8);
      
  // Set the range of the accelerometer (see page 17 of the ADXL345 Datasheet)
  // DO, D1 set the range to +/-2, +/-4, +/-8, or +/-16g
  // D3 sets FULL_RES
  writeTo(DEVICE, 0x31, B1010); //8g FULL_RES
  
  // Set the detection thresholds
  // Note that these may need to be inverted when the unit is held 
  // switch-side up instead of USB-cable side.
  pos_thresh =  1500;
  neg_thresh = -1000;

  braking = false;
  
  //Serial.begin(9600); // debug (note that this uses pins 0 & 1)
    
}


//
// The loop() method runs over and over again,
// as long as the Arduino has power
//
void loop() {
  
  if (digitalRead(switchPin) == HIGH) {  // check the change pattern switch
    pattern++;
    pattern = pattern % (sizeof(displayPatterns)/2);
    digitalWrite(LEDpinmap[pattern], HIGH);
    delay(750);
    digitalWrite(LEDpinmap[pattern], LOW);
  }
  else {  // read the accelerometer and display the pattern
    int regAddress = 0x32;    //first axis-acceleration-data register on the ADXL345
    int y; // accelerometers y-axis
    
    readFrom(DEVICE, regAddress, TO_READ, buff); //read the acceleration data from the ADXL345
    
    // The axis reading comes in 10 bit resolution, i.e. 2 bytes (with the Least Significant Byte first!!)
    // thus we are combining both bytes in to a single int
    y = (((int)buff[3]) << 8) | buff[2];
    
    // Check the thresholds to see if we are decellerating
    if (y > pos_thresh || y < neg_thresh)
    {
      if (braking == false)
      {
        braking = true;
        //Serial.print("stopping\n");
        if (y > 0)
          direction = 1; // Moving R -> L
        else
          direction = 0; // Moving L -> R
      }
    }
    else if (braking == true) 
    {
        braking = false; 
        delay(5 + 60 - rows[pattern]); // where 5 is a base delay and we assume 60 rows is the maximum display length
        display(displayPatterns[pattern], direction, rows[pattern]);
    }
  }
  
}


//
// Print the data from all rows starting with the first row 
// if direction > 0 or the last if direction <= 0
//
void display(word thisPattern[], int dir, int noRows) {

  //Serial.print(str);
  if (dir > 0) { 
    for (int row = 0; row < noRows; row++) {
      printRow(thisPattern[row]);
    }
  }
  else {
    for (int row = noRows - 1; row >= 0; row--) {
      printRow(thisPattern[row]);
    }    
  }
  
} 
 
 
//
// Print a single row of data from the pattern as a vertical 
// column of pixels 
//  
void printRow(word column) {

  // step through each pixel (LED)
  for (int pixel = 0; pixel < noPixels; pixel++) {
    
    // check to see if this pixel is on or off    
    if ((column & mask[pixel]) > 0)
      digitalWrite(LEDpinmap[pixel], HIGH); // LED on
    else
      digitalWrite(LEDpinmap[pixel], LOW);  // LED off

  }
  
  // A delay of 1200uSec and a max width of 60 rows -> display width of approx. 72 mSec
  delayMicroseconds(1200); // delay between rows (= horizontal pixel width) 

}   


//
// Accelerometer writeTo/readFrom functions
//

//
// Writes val to address register on device
//
void writeTo(int device, byte address, byte val) {
  
   Wire.beginTransmission(device); //start transmission to device 
   Wire.write(address);            // send register address
   Wire.write(val);                // send value to write
   Wire.endTransmission();         //end transmission
   
}


//
// Reads num bytes starting from address register on device in to the buffer array
//
void readFrom(int device, byte address, int num, byte buff[]) {
  
  Wire.beginTransmission(device); //start transmission to device 
  Wire.write(address);            //sends address to read from
  Wire.endTransmission();         //end transmission
  
  Wire.beginTransmission(device); //start transmission to device
  Wire.requestFrom(device, num);  // request 6 bytes from device
  
  int i = 0;
  while(Wire.available())  //device may send less than requested (abnormal)
  { 
    buff[i] = Wire.read(); // receive a byte
    i++;
  }
  Wire.endTransmission();  //end transmission
  
}
