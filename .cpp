#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22

#define MQ7_PIN A0
#define MQ135_PIN A1
#define MQ5_PIN A2

DHT dht(DHTPIN, DHTTYPE);

// Most 16x2 I2C LCDs use 0x27.
// If yours doesn't work, try 0x3F.
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(9600);

  dht.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("AIR QUALITY");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  delay(3000);
  lcd.clear();
}

void loop() {

  // Read MQ sensors
  int mq7 = analogRead(MQ7_PIN);
  int mq135 = analogRead(MQ135_PIN);
  int mq5 = analogRead(MQ5_PIN);

  // Read DHT22
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check DHT22
  if (isnan(temperature) || isnan(humidity)) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DHT22 ERROR");
    delay(2000);
    return;
  }

  /*
     Convert sensor readings into 0-100 pollution values.

     These limits are PROJECT VALUES and need calibration
     for meaningful real-world measurements.
  */

  int coScore = map(mq7, 0, 1023, 0, 100);
  int airScore = map(mq135, 0, 1023, 0, 100);
  int gasScore = map(mq5, 0, 1023, 0, 100);

  coScore = constrain(coScore, 0, 100);
  airScore = constrain(airScore, 0, 100);
  gasScore = constrain(gasScore, 0, 100);

  // Weighted overall air-quality score
  int overallScore =
      (airScore * 50 +
       coScore * 30 +
       gasScore * 20) / 100;

  String status;

  if (overallScore <= 30) {
    status = "GOOD";
  }
  else if (overallScore <= 60) {
    status = "MODERATE";
  }
  else {
    status = "POOR";
  }

  // ---------- LCD PAGE 1 ----------
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Air:");
  lcd.print(status);

  lcd.setCursor(0, 1);
  lcd.print("Score:");
  lcd.print(overallScore);

  delay(2500);

  // ---------- LCD PAGE 2 ----------
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("T:");
  lcd.print(temperature, 1);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor(9, 0);
  lcd.print("H:");
  lcd.print(humidity, 0);
  lcd.print("%");

  lcd.setCursor(0, 1);
  lcd.print("CO:");
  lcd.print(coScore);

  lcd.setCursor(9, 1);
  lcd.print("G:");
  lcd.print(gasScore);

  delay(2500);

  // ---------- SERIAL MONITOR ----------
  Serial.println("----------------------");

  Serial.print("MQ7 CO: ");
  Serial.println(mq7);

  Serial.print("MQ135 Air: ");
  Serial.println(mq135);

  Serial.print("MQ5 Gas: ");
  Serial.println(mq5);

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" C");

  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Air Quality Score: ");
  Serial.println(overallScore);

  Serial.print("Status: ");
  Serial.println(status);

  delay(1000);
}
