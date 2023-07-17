// Konfigurasi koneksi Wifi
#ifdef ESP32
  #include <WiFi.h>
#else
  #include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h> 

// Replace with your network credentials
const char* ssid = "nadhifathalla";
const char* password = "xmez7243"; 

String chatId = "1177352978";

// Initialize Telegram BOT
#define BOTtoken "5834939217:AAHGt6cgJHiiCtffDF4Yjjf0tDuoBJfmfrA"
 
#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
 
// Checks for new messages every 2 second.
int botRequestDelay = 2000;
unsigned long lastTimeBotRan;

// Sensor DHT11
#include "DHT.h"
#define DHTPIN 32
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
bool humidityDetectMonitor = false;
bool temperatureDetectMonitor = false;

// Sensor MQ-2
#include <MQ2.h>
#define MQSensor 33
bool smokeDetectMonitor = false;

MQ2 mq2(MQSensor);

// Water Level Sensor
#define waterLevelPowerPin 17
#define waterLevelSignalPin 36
#define threshold 0
int waterValue = 0;
bool waterLevelDetectMonitor = false;

//Aktuator
#define LEDBlue 19
#define LEDGreen 18
#define LEDRed 15
#define LEDYellow 16
#define buzzer 23

// LCD
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 16, 2);

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != chatId){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    // println the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
  
    // Enable Temperature Monitor
    if (text == "/EnableTemperatureAlert") {
      temperatureDetectMonitor = true;
      bot.sendMessage(chat_id, "Monitoring Suhu Telah Dinyalakan", "");
      Serial.println("Monitoring Suhu Telah Dinyalakan");
    }

    // Enable Humidity Monitor
    if (text == "/EnableHumidityAlert") {
      humidityDetectMonitor = true;
      bot.sendMessage(chat_id, "Monitoring Kelembaban Telah Dinyalakan", "");
      Serial.println("Monitoring Kelembaban Telah Dinyalakan");
    }
      
    // Enable Smoke Monitor
    if (text == "/EnableSmokeAlert") {
      smokeDetectMonitor = true;
      bot.sendMessage(chat_id, "Monitoring Kadar Asap Telah Dinyalakan", "");
      Serial.println("Monitoring Kadar Asap Telah Dinyalakan");
    }

    // Enable Water Level Monitor
    if (text == "/EnableWaterLevelAlert") {
      waterLevelDetectMonitor = true;
      bot.sendMessage(chat_id, "Monitoring Ketersediaan Air Minum Telah Dinyalakan", "");
      Serial.println("Monitoring Ketersediaan Air Minum Telah Dinyalakan");
    }

    // Disable Temperature Monitor
    if (text == "/DisableTemperatureAlert") {
      temperatureDetectMonitor = false;
      bot.sendMessage(chat_id, "Monitoring Suhu Telah Dimatikan", "");
      Serial.println("Monitoring Suhu Telah Dimatikan");
    }

    // Disable Humidity Monitor
    if (text == "/DisableHumidityAlert") {
      humidityDetectMonitor = false;
      bot.sendMessage(chat_id, "Monitoring Kelembaban Telah Dimatikan", "");
      Serial.println("Monitoring Kelembaban Telah Dimatikan");
    }
      
    // Disable Smoke Monitor
    if (text == "/DisableSmokeAlert") {
      smokeDetectMonitor = false;
      bot.sendMessage(chat_id, "Monitoring Kadar Asap Telah Dimatikan", "");
      Serial.println("Monitoring Kadar Asap Telah Dimatikan");
    }

    // Disable Water Level Monitor
    if (text == "/DisableWaterLevelAlert") {
      waterLevelDetectMonitor = false;
      bot.sendMessage(chat_id, "Monitoring Ketersediaan Air Minum Telah Dimatikan", "");
      Serial.println("Monitoring Ketersediaan Air Minum Telah Dimatikan");
    }

    if (text == "/start"){
      String welcome = "Selamat Datang didalam Sistem Monitoring Kondisi Kandang Anak Ayam Broiler, " + from_name + ".\n\n";
      welcome += "Gunakan Beberapa Perintah dibawah ini Untuk Memonitor Kondisi Kandang.\n\n";
      welcome += "/EnableTemperatureAlert : Menyalakan Notifikasi Hasil Monitoring Suhu\n";
      welcome += "/DisableTemperatureAlert : Mematikan Notifikasi Hasil Monitoring Suhu\n\n";
      welcome += "/EnableHumidityAlert : Menyalakan Notifikasi Hasil Monitoring Kelembaban\n";
      welcome += "/DisableHumidityAlert : Mematikan Notifikasi Hasil Monitoring Kelembaban\n\n";
      welcome += "/EnableSmokeAlert : Menyalakan Notifikasi Hasil Monitoring Kadar Asap\n";
      welcome += "/DisableSmokeAlert : Mematikan Notifikasi Hasil Monitoring Kadar Asap\n\n";
      welcome += "/EnableWaterLevelAlert : Menyalakan Notifikasi Hasil Monitoring Ketersediaan Air Minum\n";
      welcome += "/DisableWaterLevelAlert : Mematikan Notifikasi Hasil Monitoring Ketersediaan Air Minum\n\n";
      bot.sendMessage(chatId, welcome, "Markdown");
    }
  }
}

void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);
  dht.begin();
  mq2.begin();

  pinMode(LEDBlue, OUTPUT);
  pinMode(LEDGreen, OUTPUT);
  pinMode(LEDRed, OUTPUT);
  pinMode(LEDYellow, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(waterLevelPowerPin, OUTPUT);
  digitalWrite(waterLevelPowerPin, LOW);

  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org"); // get UTC time via NTP
    client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif
 
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // println ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  // put your main code here, to run repeatedly:
  // Membaca nilai suhu dan kelembaban
  int temperatureValue = dht.readTemperature();
  int humidityValue = dht.readHumidity();
  // int temperatureValue = 20;
  // int humidityValue = 78;
  // float smoke = 1.20;

  // Membaca nilai ketinggian air
  digitalWrite(waterLevelPowerPin, HIGH);
  delay(10);  
  waterValue = analogRead(waterLevelSignalPin);
  digitalWrite(waterLevelPowerPin, LOW);

  if (isnan(temperatureValue) || isnan(humidityValue)){
    Serial.println(F("Sensor Gagal Terbaca!"));
    return;
  }
  Serial.print(F("Suhu: "));
  Serial.print(temperatureValue);
  Serial.println(F("°C"));
  Serial.print(F("Kelembaban: "));
  Serial.println(humidityValue);
  delay(1000);
  lcd.setCursor(0,0);
  lcd.print("T:");
  lcd.print(temperatureValue);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("H:");
  lcd.print(humidityValue);

  lcd.setCursor(5,0);
  lcd.print("|");
  lcd.setCursor(5,1);
  lcd.print("|");
  
  // Menampilkan kadar asap
  Serial.print("Kadar Asap: ");
  Serial.print(mq2.readSmoke());
  Serial.println(" PPM");
  delay(1000);
  lcd.setCursor(6,1);
  lcd.print("A:");
  lcd.print(mq2.readSmoke());
  lcd.print(" PPM");

  // Menampilkan nilai ketinggian air
  Serial.print("Nilai Ketinggian Air: ");
  Serial.println(waterValue);
  delay(1000);
  lcd.setCursor(6,0);
  lcd.print("W:");
  lcd.print(waterValue);

  // Menyalakan sistem monitor pada telegram berdasarkan nilai suhu
  if(temperatureValue < 24 && temperatureDetectMonitor){
    bot.sendMessage(chatId, "Alert - Nilai Suhu Kandang Terlalu Rendah, Segera Hangatkan Kandang!", "");
    Serial.println("Nilai Suhu Kandang Tidak Ideal!");
    digitalWrite(LEDBlue, HIGH);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    tone(buzzer, 1500);
    delay(1000);
  }
  else if(temperatureValue > 25 && temperatureDetectMonitor){
    bot.sendMessage(chatId, "Alert - Nilai Suhu Kandang Terlalu Tinggi, Segera Dinginkan Kandang!", "");
    Serial.println("Nilai Suhu Kandang Tidak Ideal!");
    digitalWrite(LEDBlue, HIGH);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    tone(buzzer, 1500);
    delay(1000);
  }
  else if(temperatureValue >= 24 && temperatureValue <= 25){
    Serial.println("Nilai Suhu Kandang Ideal");
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    noTone(buzzer);
    delay(1000);
  }

  // Menyalakan sistem monitor pada telegram berdasarkan nilai kelembaban
  if(humidityValue < 60 && humidityDetectMonitor){
    bot.sendMessage(chatId, "Alert - Nilai Kelembaban Kandang Terlalu Rendah, Segera Dinginkan Kandang!", "");
    Serial.println("Nilai Kelembaban Kandang Tidak Ideal!");
    digitalWrite(LEDGreen, HIGH);
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    tone(buzzer, 1000);
    delay(1000);
  }
  else if(humidityValue > 70 && humidityDetectMonitor){
    bot.sendMessage(chatId, "Alert - Nilai Kelembaban Kandang Terlalu Tinggi, Segera Hangatkan Kandang!", "");
    Serial.println("Nilai Kelembaban Kandang Tidak Ideal!");
    digitalWrite(LEDGreen, HIGH);
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    tone(buzzer, 1000);
    delay(1000);
  }
  else if(humidityValue >= 60 && humidityValue <= 70){
    Serial.println("Nilai Kelembaban Kandang Ideal");
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    noTone(buzzer);
    delay(1000);
  }

  // Menyalakan sistem monitor pada telegram berdasarkan kadar asap
  if(mq2.readSmoke() > 1 && smokeDetectMonitor){
    bot.sendMessage(chatId, "Alert - Terdapat Kadar Asap yang Tidak Ideal, Segera Bersihkan Udara Kandang!", "");
    Serial.println("Terdapat Kadar Asap yang Tidak Ideal!");
    digitalWrite(LEDRed, HIGH);
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDYellow, LOW);
    tone(buzzer, 1500);
    delay(1000);
  }
  else if(mq2.readSmoke() <= 1){
    Serial.println("Kadar Asap Ideal");
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    noTone(buzzer);
    delay(1000);
  }

  // Menyalakan sistem monitor pada telegram berdasarkan ketersediaan air minum
  if(waterValue == threshold && waterLevelDetectMonitor){
    bot.sendMessage(chatId, "Alert - Air Minum Habis, Segera Isi Air Minum!", "");
    Serial.println("Air Minum Habis!");
    digitalWrite(LEDYellow, HIGH);
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    tone(buzzer, 500);
    delay(1000);
  }
  else if(waterValue > threshold){
    Serial.println("Air Minum Tersedia");
    digitalWrite(LEDBlue, LOW);
    digitalWrite(LEDGreen, LOW);
    digitalWrite(LEDRed, LOW);
    digitalWrite(LEDYellow, LOW);
    noTone(buzzer);
    delay(1000);
  } 

  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}