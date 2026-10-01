#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

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

// Most I2C LCDs use 0x27
// If your LCD doesn't work, try 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);


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

  // LCD
  lcd.init();
  lcd.backlight();

  // Startup screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("AIR QUALITY");
  lcd.setCursor(0, 1);
  lcd.print("MONITOR");

  delay(2000);

  lcd.clear();
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


  // Check DHT22

  if (isnan(temperature) || isnan(humidity)) {

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("DHT22 ERROR");

    lcd.setCursor(0, 1);
    lcd.print("Check sensor");

    delay(2000);

    return;
  }


  // -------------------------
  // CONVERT SENSOR VALUES
  // -------------------------

  /*
     These are relative pollution
     scores, NOT official AQI values.

     0   = low sensor reading
     100 = high sensor reading
  */

  int coScore = map(mq7, 0, 1023, 0, 100);

  int airScore = map(mq135, 0, 1023, 0, 100);

  int gasScore = map(mq5, 0, 1023, 0, 100);


  // Keep values between 0 and 100

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

    // Green ON
    digitalWrite(GREEN_LED, HIGH);

    digitalWrite(YELLOW_LED, LOW);

    digitalWrite(RED_LED, LOW);

    // Buzzer OFF
    noTone(BUZZER);
  }


  else if (airQualityScore <= 60) {

    status = "MODERATE";

    // Yellow ON
    digitalWrite(GREEN_LED, LOW);

    digitalWrite(YELLOW_LED, HIGH);

    digitalWrite(RED_LED, LOW);

    // Buzzer OFF
    noTone(BUZZER);
  }


  else {

    status = "POOR";

    // Red ON
    digitalWrite(GREEN_LED, LOW);

    digitalWrite(YELLOW_LED, LOW);

    digitalWrite(RED_LED, HIGH);

    // Warning buzzer
    tone(BUZZER, 1000);
  }


  // ==================================================
  // LCD SCREEN 1
  // ==================================================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("AIR: ");

  lcd.print(status);


  lcd.setCursor(0, 1);

  lcd.print("SCORE: ");

  lcd.print(airQualityScore);


  delay(2500);


  // ==================================================
  // LCD SCREEN 2
  // ==================================================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("TEMP:");

  lcd.print(temperature, 1);

  lcd.print((char)223);

  lcd.print("C");


  lcd.setCursor(0, 1);

  lcd.print("HUM:");

  lcd.print(humidity, 0);

  lcd.print("%");


  delay(2500);


  // ==================================================
  // LCD SCREEN 3
  // ==================================================

  lcd.clear();

  lcd.setCursor(0, 0);

  lcd.print("CO:");

  lcd.print(coScore);


  lcd.setCursor(9, 0);

  lcd.print("GAS:");

  lcd.print(gasScore);


  lcd.setCursor(0, 1);

  lcd.print("MQ135:");

  lcd.print(airScore);


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
