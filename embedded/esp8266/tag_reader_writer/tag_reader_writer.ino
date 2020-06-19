#define DUMP_ONLY true
#define DEBBUG false

#include "libraries/wemosd1mini_pins.h"
#include "wiring.h"

#include "libraries/myMFRC522.h"
#include "libraries/ntag213.h"

myMFRC522 rc522(SDA_PIN, RST_PIN);
myMFRC522::StatusCode status;

const uint8_t NTAG_START_PAGE = NTAG_START_PAGE;
byte buffer[NTAG_WRITABLE_SPACE];
char data[NTAG_WRITABLE_SPACE];

void initRFIDReader(){
    #if DEBBUG
        Serial.printf("\nStarting RFID Reader...");
    #endif

    rc522.PCD_Init();
}

void handleRFIDReader(){
    #if !DUMP_ONLY
        for(size_t i = 0; i < NTAG_WRITABLE_SPACE / 4; i++){
            #if DEBBUG
                Serial.printf("\nWritting to page: %d", NTAG_START_PAGE + i);
            #endif

            status = (MFRC522::StatusCode)rc522.MIFARE_Ultralight_Write(NTAG_START_PAGE + i, &buffer[i * 4], 4);
            if(status != MFRC522::STATUS_OK){
                #if DEBBUG
                    Serial.printf("\nWriting to tag failed :(");
                #endif

                return;
            }
        }

        Serial.printf("\n\nTag got successfully written");
    #endif

    {
        Serial.println("\n------------------------------Dumping_Tag_Contents------------------------------\n");

        Serial.println("Hex Dump:");
        rc522.PICC_DumpMifareUltralightToSerial();

        Serial.println("\nText Dump:");
        rc522.PICC_DumpMifareUltralightTextToSerial();

        rc522.PICC_DumpMifareUltralightTextToDisplay(&display);

        Serial.println("\n--------------------------------------Done--------------------------------------\n");
    }

    rc522.PICC_HaltA();
}

bool tagRFIDAvailable(){
    return rc522.PICC_IsNewCardPresent() && rc522.PICC_ReadCardSerial();
}

void setup(){
    SPI.begin();

    #if DEBBUG && !DUMP_ONLY
        Serial.begin(115200);
        while(!Serial){};
    #endif

    initRFIDReader();

    #if !DUMP_ONLY
        Serial.printf("\nEnter the data to be written on the tag (%d bytes max)...", NTAG_WRITABLE_SPACE);

        while (!Serial.available()){};

        size_t read_data_length = Serial.readBytes(data, NTAG_WRITABLE_SPACE);

        Serial.printf("\nThis will write the following data (%d bytes):\n\t%s\n\n", read_data_length, data);

        // Padding with zeros
        for(size_t i = read_data_length; i < NTAG_WRITABLE_SPACE; i++){
            data[i] = 0;
        }

        memcpy(buffer, data, NTAG_WRITABLE_SPACE);
    #endif
}

void loop(){
    if(tagRFIDAvailable()){
        handleRFIDReader();
    }
}
