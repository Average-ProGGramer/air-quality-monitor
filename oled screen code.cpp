#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// =========================
// OLED SETTINGS
// =========================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// =========================
// PIN DEFINITIONS
// =========================

// DHT22
#define DHT_PIN 2
#define DHT_TYPE DHT22

// MQ Sensors
#define MQ7_PIN A0
#define MQ135_PIN A1
#define MQ5_PIN A2

// LEDs
#define GREEN_LED 3
#define YELLOW_LED 4
#define RED_LED 5

// Buzzer
#define BUZZER 6


// =========================
// OBJECTS
// =========================

DHT dht(DHT_PIN, DHT_TYPE);


// =========================
// SETUP
// =========================

void setup() {

  Serial.begin(9600);

  // DHT
  dht.begin();

  // LEDs
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Buzzer
  pinMode(BUZZER, OUTPUT);

  // OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {

    Serial.println("OLED ERROR");

    while (1);
  }

  // Startup screen
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(5, 10);
  display.println("AIR");

  display.setCursor(5, 35);
  display.println("QUALITY");

  display.display();

  delay(2000);

  display.clearDisplay();
  display.display();
}


// =========================
// MAIN LOOP
// =========================

void loop() {

  // -------------------------
  // READ MQ SENSORS
  // -------------------------

  int mq7 = analogRead(MQ7_PIN);
  int mq135 = analogRead(MQ135_PIN);
  int mq5 = analogRead(MQ5_PIN);


  // -------------------------
  // READ DHT22
  // -------------------------

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();


  // -------------------------
  // CHECK DHT22
  // -------------------------

  if (isnan(temperature) || isnan(humidity)) {

    display.clearDisplay();

    display.setTextColor(SSD1306_WHITE);

    display.setTextSize(2);

    display.setCursor(5, 10);
    display.println("DHT22");

    display.setCursor(5, 35);
    display.println("ERROR");

    display.display();

    delay(2000);

    return;
  }


  // -------------------------
  // CONVERT SENSOR VALUES
  // -------------------------

  /*
     These are relative pollution
     scores, NOT official AQI values.
  */

  int coScore = map(mq7, 0, 1023, 0, 100);

  int airScore = map(mq135, 0, 1023, 0, 100);

  int gasScore = map(mq5, 0, 1023, 0, 100);


  coScore = constrain(coScore, 0, 100);
  airScore = constrain(airScore, 0, 100);
  gasScore = constrain(gasScore, 0, 100);


  // -------------------------
  // CALCULATE AIR SCORE
  // -------------------------

  /*
     MQ135 = 50%
     MQ7   = 30%
     MQ5   = 20%
  */

  int airQualityScore =
      (airScore * 50 +
       coScore * 30 +
       gasScore * 20) / 100;


  // -------------------------
  // DETERMINE STATUS
  // -------------------------

  String status;


  if (airQualityScore <= 30) {

    status = "GOOD";

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
  }


  else if (airQualityScore <= 60) {

    status = "MODERATE";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
  }


  else {

    status = "POOR";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    tone(BUZZER, 1000);
  }


  // ==================================================
  // OLED SCREEN 1
  // AIR QUALITY
  // ==================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("AIR QUALITY");

  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(0, 20);
  display.print(status);

  display.setCursor(0, 45);
  display.print("SCORE: ");

  display.print(airQualityScore);

  display.display();

  delay(2500);


  // ==================================================
  // OLED SCREEN 2
  // TEMPERATURE & HUMIDITY
  // ==================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("ENVIRONMENT");

  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

  display.setTextSize(2);

  display.setCursor(0, 20);
  display.print("TEMP ");

  display.print(temperature, 1);

  display.print((char)247);
  display.print("C");

  display.setCursor(0, 45);
  display.print("HUM ");

  display.print(humidity, 0);

  display.print("%");

  display.display();

  delay(2500);


  // ==================================================
  // OLED SCREEN 3
  // GAS SENSOR VALUES
  // ==================================================

  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);

  display.setCursor(0, 0);
  display.println("SENSOR VALUES");

  display.drawLine(0, 12, 127, 12, SSD1306_WHITE);

  display.setCursor(0, 20);
  display.print("CO (MQ7): ");

  display.println(coScore);

  display.setCursor(0, 34);
  display.print("MQ135:    ");

  display.println(airScore);

  display.setCursor(0, 48);
  display.print("GAS (MQ5): ");

  display.println(gasScore);

  display.display();

  delay(2500);


  // ==================================================
  // SERIAL MONITOR
  // ==================================================

  Serial.println();
  Serial.println("========================");

  Serial.print("MQ-7 CO Raw: ");
  Serial.println(mq7);

  Serial.print("MQ-135 Raw: ");
  Serial.println(mq135);

  Serial.print("MQ-5 Raw: ");
  Serial.println(mq5);

  Serial.print("CO Score: ");
  Serial.println(coScore);

  Serial.print("MQ135 Score: ");
  Serial.println(airScore);

  Serial.print("Gas Score: ");
  Serial.println(gasScore);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Air Quality Score: ");
  Serial.println(airQualityScore);

  Serial.print("Status: ");
  Serial.println(status);

  Serial.println("========================");

  delay(1000);
}
