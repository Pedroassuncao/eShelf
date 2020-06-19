// TODO reconnect to wifi if it goes down for some reason
// TODO cache some of the last api responses for some time

#define VERSION "0.0.9" // This is just for testing purposes

/*
    Notice that turning on this debbuging features will
    dramatically slow down the execution of the sketch.
    Please turn them off for production.
*/
#define ESHELF_DEBBUG_SERIAL false
#define ESHELF_DEBBUG_DISPLAY false
#define JSON_HELPER_DEBBUG_SERIAL false
#define JSON_HELPER_DEBBUG_DISPLAY false
#define API_HELPER_DEBBUG_SERIAL false
#define API_HELPER_DEBBUG_DISPLAY false

#include "libraries/wemosd1mini_pins.h"
#include "wiring.h"

#include "libraries/myMFRC522.h"
#include "libraries/ntag213.h"

#include "libraries/ssd1306.h"
#include <Adafruit_SSD1306.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServerSecure.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include "certificates/certificates.h"

#include "wifi_config.h"
#include "mdns_config.h"
#include "ota_config.h"
#include "json_helper.h"
#include "api_helper.h"

ESP8266WebServerSecure httpServer(443);
ESP8266HTTPUpdateServer httpUpdater;

myMFRC522 rc522(SDA_PIN, RST_PIN);
myMFRC522::StatusCode status;

Adafruit_SSD1306 display(DISPLAY_WIDTH, DISPLAY_HIGHT, &Wire, DISPLAY_RESET);

// the last byte is for the string terminator char '\0'
char buffer_ntag[NTAG_STORAGE_SPACE + 1];

TagParserHelper *ntag = nullptr;
ApiConnectorHelper *api = nullptr;
JsonParserHelper *api_json = nullptr;

void debbug_log_serial(String msg){
    #if ESHELF_DEBBUG_SERIAL
        Serial.printf(msg.c_str());
    #endif
}

void debbug_log_display(String msg){
    #if ESHELF_DEBBUG_DISPLAY
        display.print(msg.c_str());
        display.display();
    #endif
}

void debbug_log(String msg){
    #if ESHELF_DEBBUG_SERIAL
        debbug_log_serial(msg);
    #endif

    #if ESHELF_DEBBUG_DISPLAY
        debbug_log_display(msg);
    #endif
}

void debbug_log(String msg_serial, String msg_display){
    #if ESHELF_DEBBUG_SERIAL
        debbug_log_serial(msg_serial);
    #endif

    #if ESHELF_DEBBUG_DISPLAY
        debbug_log_display(msg_display);
    #endif
}

/**
 * @brief Initializes the wifi connection to the configured access point.
 */
void initWifi(){
    debbug_log("\neshelf: Starting Wifi...");

    WiFi.mode(WIFI_STA);

    debbug_log("\neshelf: Connecting to " + String(station_ssid) + " Wifi network...");

    WiFi.begin(station_ssid, station_password);

    while(WiFi.waitForConnectResult() != WL_CONNECTED){
        debbug_log("\neshelf: Failed to connect to " + String(station_ssid) + " Wifi network, retrying...");

        WiFi.begin(station_ssid, station_password);
    }
}

/**
 * @brief Initializes multicast DNS protocol so we can refer to the device by its DNS name.
 */
void initMDNS(){
    debbug_log("\neshelf: Starting MDNS...");

    MDNS.begin(mdns_host);
    MDNS.addService("https", "tcp", 443);
}

/**
 * @brief Initializes the https secured Over The Air firmware updater.
 */
void initOTAUpdateServer(){
    debbug_log("\neshelf: Starting OTA Update Server...");
    
    httpServer.setServerKeyAndCert_P(rsakey, sizeof(rsakey), x509, sizeof(x509));
    httpUpdater.setup(&httpServer, update_path, update_username, update_password);
    httpServer.begin();

    debbug_log(
        "\neshelf: HTTPSUpdateServer running at https://" +
        String(update_username) + ":" + String(update_password) +
        "@" + String(mdns_host) + ".local" + String(update_path)
    );
}

/**
 * @brief Initializes the display so its ready to write to.
 */
void initDisplay(){
    #if ESHELF_DEBBUG
        Serial.printf("\neshelf: Starting Display...");
    #endif

    display.begin(SSD1306_SWITCHCAPVCC, DISPLAY_ADDR);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.display();
}

/**
 * @brief Initializes the Radio-frequency identification reader so its ready to read from the tags.
 */
void initRFIDReader(){
    debbug_log("\neshelf: Starting RFID Reader...");

    rc522.PCD_Init();
}

/**
 * @brief Handles the Radio-frequency identification reader and, as soon a tag is detected reads its
 * contents and writes the correspondent informations to the screen.
 */
void handleRFIDReader(){
    if(tagRFIDAvailable()){
        #if ESHELF_DEBBUG_SERIAL
            Serial.println("\n------------------------------Dumping_Tag_Contents------------------------------\n");

            Serial.println("Hex Dump:");
            rc522.PICC_DumpMifareUltralightToSerial();

            Serial.println("\nText Dump:");
            rc522.PICC_DumpMifareUltralightTextToSerial();

            // rc522.PICC_DumpMifareUltralightTextToDisplay(&display);

            Serial.println("\n--------------------------------------Done--------------------------------------\n");
        #endif

        if(rc522.DumpNTag(buffer_ntag, sizeof(buffer_ntag))){
            debbug_log_serial("\nbuffer_ntag = " + String(buffer_ntag));

            String json = String(buffer_ntag);
            ntag = new TagParserHelper(json, &Serial, &display);

            String ntag_status_str = ntag->getStatusStr();
            if(ntag_status_str == "Ok"){
                debbug_log_serial("\neshelf: Tag read successfully");
            }else{
                // TODO write some error message to the screen (with an error code maybe, see the manual)
                debbug_log_serial("\neshelf: Error reading the tag info: " + ntag_status_str);

                delete ntag;
                return;
            }

            debbug_log("\neshelf: doc[id] = " + String(ntag->id));

            api = new ApiConnectorHelper(ntag->id, &Serial, &display);

            String api_status_str = api->getStatusStr();
            if(api_status_str == "Ok"){
                debbug_log_serial("\neshelf: Api connected successfully");
            }else{
                // TODO write some error message to the screen (with an error code maybe, see the manual)
                debbug_log_serial("\neshelf: Error connecting to api: " + api_status_str);

                delete ntag;
                delete api;
                return;
            }

            debbug_log("\neshelf: api.getApiResponse() = " + api->getApiResponse());

            String api_raw_json = api->getApiResponse();
            debbug_log_serial("\neshelf: api_raw_json: " + api_raw_json);

            DynamicJsonDocument api_doc(2048);
            DeserializationError error = deserializeJson(api_doc, api_raw_json);

            if(error){
                // TODO write some error message to the screen (with an error code maybe, see the manual)
                debbug_log_serial("\neshelf api_raw_json: Error: " + String(error.c_str()));

                delete ntag;
                delete api;
                return;
            }else{
                debbug_log_serial("\neshelf api_raw_json: Deserialization successful");
            }

            JsonObject api_json_obj = api_doc.as<JsonObject>();
            
            if(!api_json_obj.isNull()){
                writeDisplayProductInfo(api_json_obj);
            }else{
                Serial.println("\neshelf: Error: json_obj is NULL");
            }

            delete ntag;
            delete api;
        }

        rc522.PICC_HaltA();
    }
}

/**
 * @brief Keeps the multicast DNS working.
 */
void handleMDNS(){
    MDNS.update();
}

/**
 * @brief Keeps the Over The Air firmware updater working.
 */
void handleOTAUpdateServer(){
    httpServer.handleClient();
}

/**
 * @brief Checks if a new tag is available for reading.
 * @return True if there is a new tag, False otherwise.
 */
bool tagRFIDAvailable(){
    return rc522.PICC_IsNewCardPresent() && rc522.PICC_ReadCardSerial();
}

/**
 * @brief Writes the statup message to the screen.
 */
void writeDisplayStartupMessage(){
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(2);
    display.println("Lay an\nitem on\nthe shelf\narea...");
    display.display();
}

/**
 * @brief Writes the formated product information to the screen.
 */
void writeDisplayProductInfo(JsonObject prod_info){
    // for(JsonPair pair : prod_info){
    //     // String key = String(pair.key().c_str());
    //     String value = String(pair.value().as<char*>());

    //     Serial.println(/* key + String(": ") + */ value);
    //     display.println(/* key + String(": ") + */ value);
    // }

    display.setCursor(0, 0);
    display.clearDisplay();
    display.setTextWrap(true);
    display.setTextSize(1);

    display.printf(
        "%s\n%s-%s\n%s",
        prod_info["item"].as<char*>(),
        prod_info["brand"].as<char*>(),
        prod_info["model"].as<char*>(),
        prod_info["description"].as<char*>()
    );

    display.printf("\n\n");

    display.setTextSize(2);

    display.printf(
        "%s %s",
        prod_info["price"].as<char*>(),
        prod_info["currency"].as<char*>()
    );

    display.display();

    // display.startscrollleft(0x00, 0x0F);
    // display.stopscroll();
}

/**
 * @brief This function runs once and sets up the sketch.
 */
void setup(){
    #if ESHELF_DEBBUG_SERIAL
        Serial.begin(115200);
        while(!Serial){};
    #endif

    debbug_log_serial("\neshelf: Booting...\n");

    SPI.begin();

    initDisplay();

    display.setTextSize(2);
    display.print("Booting...");
    display.display();

    debbug_log("\neshelf: Version: " + String(VERSION));

    initWifi();
    initMDNS();
    initOTAUpdateServer();
    initRFIDReader();

    debbug_log_serial("\neshelf: Ready...\n");

    writeDisplayStartupMessage();
}

/**
 * @brief This function runs in a infinite loop and keeps the sketch running.
 */
void loop(){
    handleMDNS();
    handleOTAUpdateServer();
    handleRFIDReader();
}
