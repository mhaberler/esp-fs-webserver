#include <esp-fs-webserver.h>  // https://github.com/cotestatnt/esp-fs-webserver

#include <FS.h>
#include <LittleFS.h>
#define FILESYSTEM LittleFS

FSWebServer myWebServer(FILESYSTEM, 80);

// In order to set SSID and password open the /setup webserver page
// const char* ssid;
// const char* password;
bool apMode = false;


////////////////////////////////  Filesystem  /////////////////////////////////////////
void startFilesystem() {
    // FILESYSTEM INIT
    if ( !FILESYSTEM.begin()) {
        Serial.println("ERROR on mounting filesystem. It will be formmatted!");
        FILESYSTEM.format();
        ESP.restart();
    }
    myWebServer.printFileList(LittleFS, Serial, "/", 2);
}

/*
* Getting FS info (total and free bytes) is strictly related to
* filesystem library used (LittleFS, FFat, SPIFFS etc etc) and ESP framework
* ESP8266 FS implementation has methods for total and used bytes (only label is missing)
*/
#ifdef ESP32
void getFsInfo(fsInfo_t* fsInfo) {
    fsInfo->fsName = "LittleFS";
    fsInfo->totalBytes = LittleFS.totalBytes();
    fsInfo->usedBytes = LittleFS.usedBytes();
}
#else
void getFsInfo(fsInfo_t* fsInfo) {
    fsInfo->fsName = "LittleFS";
}
#endif

static int ledval;

////////////////////////////  HTTP Request Handlers  ////////////////////////////////////
void handleLed() {
    // http://xxx.xxx.xxx.xxx/led?val=1
    if(myWebServer.hasArg("val")) {
        int value = myWebServer.arg("val").toInt();
#ifdef RGB_LED
        if (value) {
          // digitalWrite(PIN_NEOPIXEL, HIGH); 
            neopixelWrite(PIN_NEOPIXEL,RGB_BRIGHTNESS,0,0); // Red
        } else {
            // digitalWrite(PIN_NEOPIXEL, LOW);    // Turn the RGB LED off

            neopixelWrite(PIN_NEOPIXEL,0,0,0); // Off / black
        }
        ledval = value;
#else
        digitalWrite(ledPin, value);
#endif
    }

    String reply = "LED is now ";
#ifdef RGB_BUILTIN
    reply += ledval ? "OFF" : "ON";
#else
    reply += digitalRead(ledPin) ? "OFF" : "ON";
#endif
    myWebServer.send(200, "text/plain", reply);
}


void setup() {

    Serial.begin(115200);

    // FILESYSTEM INIT
    startFilesystem();

    // Try to connect to stored SSID, start AP if fails after timeout
    myWebServer.setAP(AP_SSID, AP_PASSWORD);

    IPAddress myIP = myWebServer.startWiFi(15000);
    Serial.println("\n");

    // Add custom page handlers to webserver
    myWebServer.on("/led", HTTP_GET, handleLed);

    // set /setup and /edit page authentication
    // myWebServer.setAuthentication("admin", "admin");

    // Enable ACE FS file web editor and add FS info callback function
    myWebServer.enableFsCodeEditor(getFsInfo);

    // Start webserver
    myWebServer.begin();
    Serial.print(F("ESP Web Server started on IP Address: "));
    Serial.println(myIP);
    Serial.println(F("Open /setup page to configure optional parameters"));
    Serial.println(F("Open /edit page to view and edit files"));

#ifdef RGB_LED
    neopixelWrite(PIN_NEOPIXEL, 0, RGB_BRIGHTNESS,0); // green
#endif
}


void loop() {
    myWebServer.run();
}