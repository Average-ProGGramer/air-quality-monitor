#include <DHT.h>

// ==========================================
// AIR QUALITY MONITOR - ARDUINO UNO
// MQ2 + MQ7 + MQ135 + DHT22
// ==========================================

// ---------- DHT22 ----------
#define DHT_PIN 2
#define DHT_TYPE DHT22

// ---------- MQ GAS SENSORS ----------
#define MQ7_PIN   A0
#define MQ135_PIN A1
#define MQ2_PIN   A2

// ---------- LEDs ----------
#define GREEN_LED  3
#define YELLOW_LED 4
#define RED_LED    5

// ---------- BUZZER ----------
#define BUZZER 6

DHT dht(DHT_PIN, DHT_TYPE);


// ==========================================
// SETUP
// ==========================================

void setup() {

  Serial.begin(9600);

  dht.begin();

  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(YELLOW_LED, LOW);
  digitalWrite(RED_LED, LOW);

  noTone(BUZZER);

  // Give DHT22 some time to start
  delay(2000);
}


// ==========================================
// MAIN LOOP
// ==========================================

void loop() {

  // ----------------------------------------
  // READ MQ SENSORS
  // ----------------------------------------

  int mq7Raw   = analogRead(MQ7_PIN);
  int mq135Raw = analogRead(MQ135_PIN);
  int mq2Raw   = analogRead(MQ2_PIN);


  // ----------------------------------------
  // READ DHT22
  // ----------------------------------------

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();


  // Check whether DHT22 is working
  if (isnan(temperature) || isnan(humidity)) {

    Serial.println("{\"error\":\"DHT22_ERROR\"}");

    delay(2000);

    return;
  }


  // ----------------------------------------
  // CONVERT MQ READINGS TO 0-100 SCORE
  // ----------------------------------------

  int coScore =
      map(mq7Raw, 0, 1023, 0, 100);

  int airScore =
      map(mq135Raw, 0, 1023, 0, 100);

  int gasScore =
      map(mq2Raw, 0, 1023, 0, 100);


  coScore =
      constrain(coScore, 0, 100);

  airScore =
      constrain(airScore, 0, 100);

  gasScore =
      constrain(gasScore, 0, 100);


  // ----------------------------------------
  // CALCULATE OVERALL AIR QUALITY SCORE
  // ----------------------------------------
  //
  // MQ135 = 50%
  // MQ7   = 30%
  // MQ2   = 20%
  //

  int airQualityScore =
      (airScore * 50 +
       coScore * 30 +
       gasScore * 20) / 100;


  // ----------------------------------------
  // DETERMINE AIR QUALITY STATUS
  // ----------------------------------------

  String status;


  // ===== GOOD =====

  if (airQualityScore <= 30) {

    status = "GOOD";

    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
  }


  // ===== MODERATE =====

  else if (airQualityScore <= 60) {

    status = "MODERATE";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, HIGH);
    digitalWrite(RED_LED, LOW);

    noTone(BUZZER);
  }


  // ===== POOR =====

  else {

    status = "POOR";

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(YELLOW_LED, LOW);
    digitalWrite(RED_LED, HIGH);

    // Warning alarm
    tone(BUZZER, 1000);
  }


  // ==========================================
  // SEND DATA TO WEBSITE AS JSON
  // ==========================================

  Serial.print("{");


  // Temperature
  Serial.print("\"temperature\":");
  Serial.print(temperature, 1);


  // Humidity
  Serial.print(",\"humidity\":");
  Serial.print(humidity, 1);


  // Raw MQ7
  Serial.print(",\"mq7\":");
  Serial.print(mq7Raw);


  // Raw MQ135
  Serial.print(",\"mq135\":");
  Serial.print(mq135Raw);


  // Raw MQ2
  Serial.print(",\"mq2\":");
  Serial.print(mq2Raw);


  // CO score
  Serial.print(",\"coScore\":");
  Serial.print(coScore);


  // Air pollution score
  Serial.print(",\"airScore\":");
  Serial.print(airScore);


  // Gas / smoke score
  Serial.print(",\"gasScore\":");
  Serial.print(gasScore);


  // Overall score
  Serial.print(",\"airQuality\":");
  Serial.print(airQualityScore);


  // Status
  Serial.print(",\"status\":\"");
  Serial.print(status);
  Serial.print("\"");


  Serial.println("}");


  // Update every second
  delay(1000);
}
