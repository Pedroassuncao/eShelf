// Based on DumpInfo Example from MFRC522 library

#include <SPI.h>
#include <MFRC522.h>
#include "libraries/wemosd1mini_pins.h"

#define RST_PIN D0
#define SDA_PIN D8

MFRC522 rc522(SDA_PIN, RST_PIN);

void setup(){
    Serial.begin(115200);

    while (!Serial){};

    SPI.begin();

    rc522.PCD_Init();
    rc522.PCD_DumpVersionToSerial();

    Serial.println(F("Scan a tag..."));
}

void loop(){
    if(!rc522.PICC_IsNewCardPresent()){
        return;
    }

    if(!rc522.PICC_ReadCardSerial()){
        return;
    }

    rc522.PICC_DumpToSerial(&(rc522.uid));
}