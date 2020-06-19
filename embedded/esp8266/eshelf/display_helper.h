#ifndef display_helper_h
#define display_helper_h

#ifndef DISPLAY_HANDLER_HELPER_DEBBUG_SERIAL
    #define DISPLAY_HANDLER_HELPER_DEBBUG_SERIAL false // Send debbug messages to serial
#endif

#ifndef DISPLAY_HANDLER_HELPER_DEBBUG_DISPLAY
    #define DISPLAY_HANDLER_HELPER_DEBBUG_DISPLAY false // Send debbug messages to display
#endif

#include <WString.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "api_config.h"

#define STATUS_OK 0
#define STATUS_FAIL 1

/**
 * @brief Helps handling the display, so we can
 * extrapolate to other display models if needed.
 */
class DisplayHandlerHelper{
public:
    typedef unsigned short int status;

protected:
    status status_code;
    String http_response;

public:
    /**
     * @brief Gets the status.
     * @return Status code.
     */
    status getStatus(){
        return status_code;
    }

    /**
     * @brief Gets the status as a String.
     * @return Status code String.
     */
    const String getStatusStr() const{
        switch(status_code){
            case STATUS_OK:
                return String("Ok");
            case STATUS_FAIL:
                return String("Fail");
            default:
                return String("Unknown");
        }
    }

protected:
    Adafruit_SSD1306 *display;
    HardwareSerial *serial;

    WiFiClient *api_client;
    HTTPClient *http;

    #if DISPLAY_HANDLER_HELPER_DEBBUG_SERIAL
        void debbug_log_serial(String msg){
            serial->printf(msg.c_str());
        }
    #endif

    #if DISPLAY_HANDLER_HELPER_DEBBUG_DISPLAY
        void debbug_log_display(String msg){
            display->print(msg.c_str());
            display->display();
        }
    #endif
    
    void debbug_log(String msg){
        #if DISPLAY_HANDLER_HELPER_DEBBUG_SERIAL
            debbug_log_serial(msg);
        #endif

        #if DISPLAY_HANDLER_HELPER_DEBBUG_DISPLAY
            debbug_log_display(msg);
        #endif
    }

    void debbug_log(String msg_serial, String msg_display){
        #if DISPLAY_HANDLER_HELPER_DEBBUG_SERIAL
            debbug_log_serial(msg_serial);
        #endif

        #if DISPLAY_HANDLER_HELPER_DEBBUG_DISPLAY
            debbug_log_display(msg_display);
        #endif
    }

    /**
     * @brief Sets the status.
     * @param Status code.
     */
    void setStatus(status status_code){
        this->status_code = status_code;
    }

    void init(){
    }

public:
    DisplayHandlerHelper(Adafruit_SSD1306 *display, HardwareSerial *serial = nullptr){
        this->display = display;

        if(serial){
            this->serial = serial;
        }

        init();
    }
};

#endif
