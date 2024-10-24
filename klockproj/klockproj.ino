/*
* Name: Advanced Clock
* Author: Karim Nagy
* Date: 2024-10-10
* Description: This project uses a ds3231 to measure time and displays the time and temperature to an 1306 oled display
* Further, it measures temperature  with an analog temperature module and displays a mapped value to a 9g-servo-motor
* Updates the Neopixel RGB ring depending on the current time
*/

// Include Libraries

#include <RTClib.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include "U8glib.h"
#include <Servo.h>

// Init constants

const int tempPin = A5;

// Init global variables

#define NUMPIXELS 24  // constant for number of pixels in the Neopixel RGB ring  
#define PIN 6  // attach the Neopixel RGB ring to pin 6
float temp = 0; 
char t[32]; // character array for storing strings

// construct objects

RTC_DS3231 rtc; // declares an object "rtc" of the RTC_DS3231 class 
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);    // Display which does not send AC
Servo myservo; // declares an object "myservo" of the servo class
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, 6, NEO_GRB + NEO_KHZ800); 
// Initializes 'pixels' as an Adafruit_NeoPixel object with:
// - NUMPIXELS: Number of LEDs in the NeoPixel strip/ring.
// - 6: Pin number where the NeoPixel data line is connected.
// - NEO_GRB + NEO_KHZ800: Specifies color order (Green, Red, Blue) and signal frequency (800 kHz).

  void setup() {

  // init communication
  Serial.begin(9600);
  Wire.begin();

  // Init Hardware
  rtc.begin(); // initializes the RTC (real time clock)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // sets the RTC to the current date and time
  u8g.setFont(u8g_font_unifont); // sets the font of the text on the OLED display to "u8g_font_unifont"
  myservo.attach(9); // attaches the servo to pin 9
  pixels.begin(); // initializes the Neopixel RGB ring
  pixels.setBrightness(25); // sets brightness of the LEDs to 25 (out of 255)
  pixels.show(); // sends current pixel color settings to the Neopixel ring to update its display

  
}

  void loop() {

  DateTime now = rtc.now(); // gets the current time from the RTC
 
  Serial.println("The time is " + getTime()); // prints time in serial monitor
  delay(100);
  oledWrite(getTime());
  servoWrite(getTemp());
  Serial.println(getTemp()); // prints temperature in serial monitor
  ledringDayProgress(now);

  // other functions are already explained below, here I only call on them 

}


/*
*This function reads time from an ds3231 module and package the time as a String
*Parameters: Void
*Returns: time in hh:mm:ss as String
*/

  String getTime(){
  DateTime now = rtc.now();
  return (String(now.hour()) +":"+ String(now.minute()) +":"+ String(now.second()));

}

/*
* This function reads an analog pin connected to an analog temprature sensor and calculates the corresponding temp
*Parameters: Void
*Returns: temprature as float
*/
  float getTemp(){
  float R1 = 10000;  // value of R1 on board
  float logR2, R2, T;
  float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;  //steinhart-hart coeficients for thermistor

  int Vo = analogRead(tempPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);  //calculate resistance on thermistor
  logR2 = log(R2);
  temp = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));  // temperature in Kelvin
  temp = temp - 308.15;                                           //convert Kelvin to Celcius

  return temp;

}

/*
* This function takes a string and draws it to an oled display (added printing temperature)
*Parameters: - text: String to write to display
*Returns: void
*/
  void oledWrite(String text){
  u8g.firstPage();

  do {

  u8g.setPrintPos(80, 36);  //sets the cursor position to (80, 36) on the OLED
  u8g.print(temp);  //prints the temperature at the set position

  u8g.drawStr(0, 36, text.c_str()); 


  } while( u8g.nextPage() );
}



/*
*Takes a temprature value and maps it to corresponding degree on a servo
*Parameters: - value: temprature
*Returns: void
*/
  void servoWrite(float value){
  myservo.write(map(value, 20, 30, 0, 179));  //maps the "value" (temperature) from the range 20-30 to the servo's angle range 0-179 and sets the servo position accordingly
}
  
/*
* This function updates the NeoPixel ring to visually show how much of the day has passed
* Parameters: - time: current time (hours, minutes, seconds)
* Returns: void
*/
  void ledringDayProgress(DateTime now) {
  const int totalSeconds = 60;  // total number of seconds in a minute
  int currentSeconds = now.second();  // seconds since start of minute
  
  // calculate how many LEDs should be on based on how much of the day has passed by mapping the values of the ints currentSeconds and totalSecondsInDay to the amount of pixels (NUMPIXELS)
  int numLEDsOn = map(currentSeconds, 0, totalSeconds, 0, NUMPIXELS);

  // turn off all pixels first
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0)); // turn off all LEDs
  }

  // turn on LEDs based on day progress
  for (int i = 0; i < numLEDsOn; i++) {
    pixels.setPixelColor(i, pixels.Color(204, 0, 204)); // violet color to show progress (can change the value of pixels.Color depending on what color you prefer)
  }

  pixels.show();  // updates the NeoPixel ring to display the current colors
}
