// Note: this was tested and it's currently working for NTAG213 tags

#ifndef myMFRC522_h
#define myMFRC522_h

#include <MFRC522.h>
#include <Adafruit_SSD1306.h>
#include "ntag213.h"

class myMFRC522: public MFRC522{
    // read up to this page
    #define END_PAGE 47

public:
    myMFRC522(byte chipSelectPin, byte resetPowerDownPin): MFRC522(){}

    /**
     * @brief Slightly modified from the MFRC522 lib in order to redefine the END_PAGE value.
     */
    void PICC_DumpMifareUltralightToSerial(){
        MFRC522::StatusCode status;
        byte byteCount;
        byte buffer[18];
        byte i;
        
        Serial.println(F("Page  0  1  2  3"));
        for(byte page = 0; page < END_PAGE; page +=4){
            byteCount = sizeof(buffer);
            status = MIFARE_Read(page, buffer, &byteCount);
            if (status != STATUS_OK) {
                Serial.print(F("MIFARE_Read() failed: "));
                Serial.println(GetStatusCodeName(status));
                break;
            }

            for (byte offset = 0; offset < 4; offset++) {
                i = page + offset;

                if(i < 10){
                    Serial.print(F("  "));
                }else{
                    Serial.print(F(" "));
                }
                
                Serial.print(i);
                Serial.print(F("  "));
                
                for (byte index = 0; index < 4; index++) {
                    i = 4 * offset + index;

                    if(buffer[i] < 0x10){
                        Serial.print(F(" 0"));
                    }else{
                        Serial.print(F(" "));
                    }

                    Serial.print(buffer[i], HEX);
                }
                Serial.println();
            }
        }
    }

    /**
     * @brief Dumps the contents in text mode to Serial.
     * @Note Non text chars will be displayed as spaces.
     */
    void PICC_DumpMifareUltralightTextToSerial(){
        MFRC522::StatusCode status;
        byte byteCount;
        byte buffer[18];
        byte i;
        
        for (byte page = 0; page < END_PAGE; page +=4){
            byteCount = sizeof(buffer);
            status = MIFARE_Read(page, buffer, &byteCount);

            if(status != STATUS_OK){
                Serial.print(F("MIFARE_Read() failed: "));
                Serial.println(GetStatusCodeName(status));
                break;
            }

            for(size_t i = 0; i < sizeof(buffer) - 2; i++){
                if(this->isPrintable(buffer[i])){
                    // prints the actual character
                    Serial.print((char)buffer[i]);
                }else{
                    // prints a space instead
                    Serial.print(" ");
                }
            }
        }
    }

    /**
     * @brief Dumps the contents in text mode to the display.
     * @Note Non text chars will be displayed as spaces.
     */
    void PICC_DumpMifareUltralightTextToDisplay(Adafruit_SSD1306 *display){
        MFRC522::StatusCode status;
        byte byteCount;
        byte buffer[18];
        byte i;
        
        display->clearDisplay();
        display->setCursor(0, 0);
        
        for (byte page = 0; page < END_PAGE; page +=4){
            byteCount = sizeof(buffer);
            status = MIFARE_Read(page, buffer, &byteCount);

            if(status != STATUS_OK){
                Serial.print(F("MIFARE_Read() failed: "));
                Serial.println(GetStatusCodeName(status));
                break;
            }
            
            for(size_t i = 0; i < sizeof(buffer) - 2; i++){
                if(this->isPrintable(buffer[i])){
                    // prints the actual character
                    display->print((char)buffer[i]);
                }else{
                    // prints a space instead
                    display->print(" ");
                }
            }
        }
        
        display->display();
    }

    /**
     * @brief Dumps the NTAG to a buffer.
     * @param Buffer to hold the NTAG information.
     * @param Size of the buffer.
     * @return True if the operation was successful, False otherwise.
     */
    bool DumpNTag(char *buffer_ntag, unsigned int length){
        MFRC522::StatusCode status;
        byte byteCount;
        byte buffer_page[18];
        
        // memset(buffer_ntag, ' ', length);

        // copying from ntag to buffer_ntag
        for(byte page = NTAG_START_PAGE; page < NTAG_STOP_PAGE; page +=4){
            byteCount = sizeof(buffer_page);
            status = MIFARE_Read(page, buffer_page, &byteCount);

            // if read fails don't read any further, it's all the info or nothing
            if(status != STATUS_OK){
                return false;
            }

            // the relevant info from the tag at NTAG_START_PAGE, but buffer_ntag starts at addr 0
            memcpy(buffer_ntag + (page - NTAG_START_PAGE) * 4, buffer_page, sizeof(buffer_page) - 2);
        }

        // substituting premature end of string with a space character
        for(size_t i = 0; i < length; ++i){
            if(buffer_ntag[i] == '\0'){
                buffer_ntag[i] = ' ';
            }
        }

        // terminating the string with a null character (end of string)
        buffer_ntag[length - 1] = '\0';

        return true;
    }


    /**
     * @brief Checks if a byte corresponds to a utf-8/ascii printable character.
     * @return True if it's printable, False if not.
     */
    bool isPrintable(byte byte_char){
        return byte_char >= 0x20 && byte_char <= 0x7e;
    }
};

#endif
