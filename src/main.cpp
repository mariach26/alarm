#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>

#define SCREEN_WIDTH    128
#define SCREEN_HEIGHT   64
#define OLED_RESET      -1
#define SDA_PIN         21
#define SCK_PIN         22

#define BUZZER_PIN 18 
#define BUTTON_PIN 19
#define BUZZER_CHANNEL 0

Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT, &Wire ,OLED_RESET);

const char* ssid = "Mple";
const char* password = "1234567896";

const char* ntpServer ="pool.ntp.org";
const long gmtOffset_sec= 7200; //2 wres *3600sec
const int daylightOffset_sec = 3600;

//alarm settings
int alarmHour=13; //wra poy tha xtypaei (24hr)
int alarmMinute=34;
bool isAlarmActive = false; //an einai anoixto twra
bool alarmAcknowledged = false; //an pathsa na stamathsei

unsigned long lastScreenUpdate = 0; //extra gia roloi 

void setup() {
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP); // low otan patietai

    ledcSetup(BUZZER_CHANNEL, 1000, 8); // Κανάλι 0, Συχνότητα 1000Hz, Ανάλυση 8-bit
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    Wire.begin(SDA_PIN, SCK_PIN);
    Wire.setClock(100000); // xamhlh taxuthta na mhn exei paremboles

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Η οθόνη δεν βρέθηκε!");
        for(;;);
    }
  
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.print("Connecting WiFi...");
    display.display();

    // wifi settings gia stathero reuma
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); 
    WiFi.begin(ssid, password);
  
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

  //sygxronismos wras
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void loop(){
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
      return; //an den exei parei thn wra akoma, perimenei
    }
    if (timeinfo.tm_hour == alarmHour && timeinfo.tm_min == alarmMinute) {
        if (!alarmAcknowledged) {
        isAlarmActive = true; //xtypaei
        } 
    } else {
        alarmAcknowledged = false; //an perase to lepto, epanaferw gia thn epomenh mera
    }
    if (digitalRead(BUTTON_PIN) == LOW) {
        if (isAlarmActive) {
            isAlarmActive = false;
            alarmAcknowledged = true; // stamataei, den ksanaxtypaei sto idio lepto
            ledcWriteTone(BUZZER_CHANNEL, 0);       // stamataei hxo
        }
    }

    if (isAlarmActive) {
        if (millis() % 1000 < 500) {
            ledcWriteTone(BUZZER_CHANNEL, 1000); // 1000Hz Ήχος (αντί για tone)
        } else {
            ledcWriteTone(BUZZER_CHANNEL, 0);    // Σιωπή
        }
    } else {
        ledcWriteTone(BUZZER_CHANNEL, 0); // Σιγουριά ότι το buzzer είναι κλειστό
    }

    if (millis() - lastScreenUpdate >= 1000) {
        lastScreenUpdate = millis();


        display.clearDisplay();

        display.setTextSize(1);
        display.setCursor(52,0);
        display.print("TIME");

        display.setTextSize(2);
        display.setCursor(15, 25);
        display.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

        display.setTextSize(1);
        display.setCursor(20, 50);
        display.printf("Alarm: %02d:%02d", alarmHour, alarmMinute);

        display.display();

    }
}