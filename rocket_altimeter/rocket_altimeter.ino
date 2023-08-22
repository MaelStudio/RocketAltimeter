#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

// pins
const byte buttonPin = 2;
const byte servoPin = 3;

// BMP280 altimeter
Adafruit_BMP280 bmp;
const int bmpAddress = 0x76;
const float seaPressure = 1013.25;
const int measureDelay = 50;

// OLED display
Adafruit_SSD1306 display(128, 64, &Wire, -1);
const int oledAddress = 0x3C;

// servo
Servo servo;
const int servoHomePos = 0;
const int servoTriggerPos = 180;

void setup() {
  
  // setup io
  pinMode(buttonPin, INPUT_PULLUP);
  bmp.begin(bmpAddress);
  display.begin(SSD1306_SWITCHCAPVCC, oledAddress);
  servo.attach(servoPin);

  servo.write(servoHomePos); // set servo to home position
  
  // debug menu
  bool debug = !digitalRead(buttonPin);
  if (debug) {
    unsigned long start = millis();
    while(millis()-start < 1000) {
      displayText("Entering\ndebug menu", false);
      displayProgressBar(float(millis()-start)/1000, 2, 36, 124, 10);
      display.display();
    }
    return; // go to loop
  }

  // set ground altitude
  float ground = 0;
  unsigned long start = millis();
  while(millis()-start < 5000) {
    float altitude = bmp.readAltitude(seaPressure);
    if(altitude > ground) {
      ground = altitude;
    }
    displayText("Setting\nground\naltitude", false);
    displayProgressBar(float(millis()-start)/5000, 2, 52, 124, 10);
    display.display();
    delay(measureDelay);
  }
  
  displayText("Ready for launch!", true);
  delay(2000);
  display.clearDisplay();
  display.display();

  // wait for launch
  while (true) {
    
    // detect launch if rocket is 1m higher than ground
    float altitude = bmp.readAltitude(seaPressure);
    if(altitude - ground > 2) {
      break;
    }
  }
  
  const unsigned long launchTime = millis(); // save launch time

  float highest = ground;
  
  // wait for apogee
  while (true) {
    
    float altitude = bmp.readAltitude(seaPressure);
    if(altitude > highest) {
      highest = altitude;
    } else if (altitude - highest < -2) {
      break;
    }
    delay(measureDelay);
  }

  const unsigned long apogeeTime = millis(); // save apogee time
  deployParachute();

  // flight statistics

  float height = highest - ground; // get height
  
  char heightStr[6];
  dtostrf(height, 6, 2, heightStr); // convert height to text
  char heightDisplay[20];
  strcpy(heightDisplay, "Altitude:\n\n");
  strcat(heightDisplay, heightStr);
  strcat(heightDisplay, " m");
  
  float duration = float((apogeeTime - launchTime)) / 1000; // get duration

  char durationStr[6];
  dtostrf(duration, 6, 2, durationStr); // convert height to text // convert duration to text
  char durationDisplay[20];
  strcpy(durationDisplay, "Duration:\n\n");
  strcat(durationDisplay, durationStr);
  strcat(durationDisplay, " s");
  
  bool stat = 0;
  
  // show flight stats on OLED (height, duration, avg speed)
  while(true) {
    waitForButton(false);
    waitForButton(true);

    if(!stat) {
      displayText(heightDisplay, true);
      stat = !stat;
    } else {
      displayText(durationDisplay, true);
      stat = !stat;
    }
  }
}

void loop() {
  // debug mode
  displayDebug();

  // test servo function
  if(!digitalRead(buttonPin)) {
    deployParachute();
  }

  delay(50);
}

void displayText(char * text, bool send) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(text);
  if(send){
    display.display();
  }
}

void displayProgressBar(float progress, int x, int y, int w, int h) {
  display.drawRect(x, y, w, h, SSD1306_WHITE);
  display.fillRect(x+3, y+3, ceil(w*progress)-6, h-6, SSD1306_WHITE);
}

void deployParachute() {
  // trigger servo
  servo.write(servoTriggerPos);
  delay(500);
  servo.write(servoHomePos);
}

void waitForButton(bool trigger) {
  while(true) {
    if(!digitalRead(buttonPin) == trigger) {
      return;
    }
    delay(50);
  }
}

void displayDebug() {
  float altitude = bmp.readAltitude(seaPressure);
  float temperature = bmp.readTemperature();

  // show altitude and temperature on OLED display
  display.clearDisplay(); // Clear display buffer
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println("Temperature:");
  display.setTextSize(2);
  display.print(temperature);
  display.println(" C");
  display.setCursor(0, 32);
  display.setTextSize(1);
  display.println("Altitude:");
  display.setTextSize(2);
  display.print(altitude);
  display.println(" m");
  display.display(); // Show buffer on display
}