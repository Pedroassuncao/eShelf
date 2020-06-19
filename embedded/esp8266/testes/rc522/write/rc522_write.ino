#include <SPI.h>
#include "libraries/wemosd1mini_pins.h"
#include "libraries/myMFRC522.h"

#define RST_PIN D0
#define SDA_PIN D8

#define DUMP_ONLY false
#define DEBBUG false

#define NTAG_START_PAGE 5 // WARNING: this value must be greater than 4 so you don't brick the tag
#define NTAG_STOP_PAGE 39 // WARNING: this value must be less than 40 so you don't brick the tag
#define NTAG_WRITABLE_SPACE (NTAG_STOP_PAGE - NTAG_START_PAGE + 1) * 4 // Available writable space on the tag

myMFRC522 rc522(SDA_PIN, RST_PIN);
myMFRC522::StatusCode status;

const uint8_t NTAG_START_PAGE = NTAG_START_PAGE;
byte buffer[NTAG_WRITABLE_SPACE];
char data[NTAG_WRITABLE_SPACE];

void setup(){
    SPI.begin();
    Serial.begin(115200);
    while(!Serial){};

    rc522.PCD_Init();
    rc522.PCD_DumpVersionToSerial();

    if(!DUMP_ONLY){
        Serial.printf("\nEnter de data to be written on the tag (%d bytes max)...", NTAG_WRITABLE_SPACE);

        while(!Serial.available()){};

        size_t read_data_length = Serial.readBytes(data, NTAG_WRITABLE_SPACE);

        Serial.printf("\nThis will write the following data (%d bytes):\n\t%s\n\n", read_data_length, data);

        // Padding with zeros
        for (size_t i = read_data_length; i < NTAG_WRITABLE_SPACE; i++){
            data[i] = 0;
        }

        memcpy(buffer, data, NTAG_WRITABLE_SPACE);
    }
}

void loop(){
    if(!rc522.PICC_IsNewCardPresent()){
        return;
    }

    if(!rc522.PICC_ReadCardSerial()){
        return;
    }

    if(!DUMP_ONLY){
        for(size_t i = 0; i < NTAG_WRITABLE_SPACE / 4; i++){
            if(DEBBUG){
                Serial.printf("\nWritting to page: %d", NTAG_START_PAGE + i);
            }

            status = (MFRC522::StatusCode)rc522.MIFARE_Ultralight_Write(NTAG_START_PAGE + i, &buffer[i * 4], 4);
            if(status != MFRC522::STATUS_OK){
                Serial.println("Write failed :(");

                return;
            }
        }
        
        Serial.println("\nTag got successfully written");
    }

    Serial.println("\n------------------------------Dumping_Tag_Contents------------------------------\n");
    
    Serial.println("Hex Dump:");
    rc522.PICC_DumpMifareUltralightToSerial();

    Serial.println("\nText Dump:");
    rc522.PICC_DumpMifareUltralightTextToSerial();
    
    Serial.println("\n--------------------------------------Done--------------------------------------\n");
    
    rc522.PICC_HaltA();
}
