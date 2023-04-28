#include <Wire.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <Servo.h>

// pins
const byte buttonPin = 2;
const byte servoPin = 3;
const byte buzzerPin = 3;

// BMP280 altimeter
Adafruit_BMP280 bmp;
const int bmpAddress = 0x76;
const float seaPressure = 1013.25;
const int measureDelay = 50;

// OLED display
Adafruit_SSD1306 display(128, 64, &Wire, -1);
const int oledAddress = 0x3C;

// servo
//Servo servo;
//const int servoHomePos = 0;
//const int servoTriggerPos = 120;

// variables
float height;
float duration;

void setup() {
  // setup io
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);
  bmp.begin(bmpAddress);
  display.begin(SSD1306_SWITCHCAPVCC, oledAddress);
  //servo.attach(servoPin);

  //servo.write(servoHomePos); // set servo to home position
  
  // debug menu
  bool debug = !digitalRead(buttonPin);
  if (debug) {
    displayText("Entering\ndebug menu");
    delay(2000);
    return; // go to loop
  }

  // set ground altitude
  displayText("Setting\nground\naltitude");
  float ground = 0;
  unsigned long start = millis();
  while(millis()-start < 5000) {
    float altitude = bmp.readAltitude(seaPressure);
    if(altitude > ground) {
      ground = altitude;
    }
    delay(measureDelay);
  }
  
  displayText("Ready for launch!");
  delay(2000);
  display.clearDisplay();
  display.display();

  // wait for launch
  while (true) {
    
    // detect launch if rocket is 1m higher than ground
    float altitude = bmp.readAltitude(seaPressure);
    if(altitude - ground > 1) {
      break;
    }
    delay(measureDelay);
  }
  
  const unsigned long launchTime = millis(); // save launch time

  float highest = ground;
  
  // wait for apogee
  while (true) {
    
    float altitude = bmp.readAltitude(seaPressure);
    if(altitude > highest) {
      highest = altitude;
    } else if (altitude - highest < -1) {
      break;
    }
    delay(measureDelay);
  }

  const unsigned long apogeeTime = millis(); // save apogee time
  deployParachute();

  // flight statistics
  height = highest - ground;
  height = roundFloat(height, 2);
  
  duration = float((apogeeTime - launchTime)) / 1000;
  duration = roundFloat(duration, 2);
  
  
  String stat = "";
  
  // show flight stats on OLED (height, duration, avg speed)
  while(true) {
    waitForButton(false);
    waitForButton(true);
    
    if(stat == "" || stat == "Duration") {
      stat = "Height";
      displayText(stat + "\n" + height + " m");
    } else {
      stat = "Duration";
      displayText(stat + "\n" + duration + " s");
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

void displayText(String text) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(text);
  display.display();
}

void deployParachute() {
  // trigger servo
  //servo.write(servoTriggerPos);
  //delay(500);
  //servo.write(servoHomePos);
  tone(buzzerPin, 800, 500);
}

void waitForButton(bool trigger) {
  while(true) {
    if(!digitalRead(buttonPin) == trigger) {
      return;
    }
    delay(50);
  }
}

float roundFloat(float number, int decimalPlaces) {
  float factor = pow(10, decimalPlaces);
  return round(number * factor) / factor;
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
