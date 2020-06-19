#ifndef api_helper_h
#define api_helper_h

#ifndef API_HELPER_DEBBUG_SERIAL
    #define API_HELPER_DEBBUG_SERIAL false // send debbug messages to serial
#endif

#ifndef API_HELPER_DEBBUG_DISPLAY
    #define API_HELPER_DEBBUG_DISPLAY false // send debbug messages to display
#endif

#include <WString.h>
#include <ArduinoJson.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include "api_config.h"

#define STATUS_OK 0
#define STATUS_FAIL 1

/**
 * @brief Helps handling the API connection and its responses.
 */
class ApiConnectorHelper{
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

    /**
     * @brief Gets the last response from the API.
     * @return API response.
     */
    String getApiResponse(){
        return http_response;
    }

protected:
    HardwareSerial *serial;
    Adafruit_SSD1306 *display;

    WiFiClient *api_client;
    HTTPClient *http;

    #if API_HELPER_DEBBUG_SERIAL
        void debbug_log_serial(String msg){
            serial->printf(msg.c_str());
        }
    #endif

    #if API_HELPER_DEBBUG_DISPLAY
        void debbug_log_display(String msg){
            display->print(msg.c_str());
            display->display();
        }
    #endif
    
    void debbug_log(String msg){
        #if API_HELPER_DEBBUG_SERIAL
            debbug_log_serial(msg);
        #endif

        #if API_HELPER_DEBBUG_DISPLAY
            debbug_log_display(msg);
        #endif
    }

    void debbug_log(String msg_serial, String msg_display){
        #if API_HELPER_DEBBUG_SERIAL
            debbug_log_serial(msg_serial);
        #endif

        #if API_HELPER_DEBBUG_DISPLAY
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

    /**
     * @brief Gets the data from the API by executing the http request and sets its response.
     */
    void getApiData(){
        // sends http get request to the api
        int httpCode = http->GET();

        // these codes come from inside the library
        // check the documentation for more details
        // negative values represent error so lets check for that too
        if(httpCode > 0){
            if(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY){
                http_response = http->getString();
                
                setStatus(STATUS_OK);
                debbug_log("ApiConnectorHelper(getApiData):" + http_response);
            }
        }else{
            setStatus(STATUS_FAIL);
            debbug_log("ApiConnectorHelper(getApiData): Error: %s" + http->errorToString(httpCode));
        }

        http->end();
    }

    /**
     * @brief Initializes the object and stars the API connection.
     * @param The arguments of the http request (in this case it's just the product id).
     */
    void init(String args){
        api_client = new WiFiClient;
        http = new HTTPClient;

        if(
            http->begin(
                *api_client,
                String(api_protocol) + "://" + 
                String(api_uri) + ":" +
                String(api_port) +
                String(api_path) +
                args
            )
        ){
            getApiData();
        }else{
            debbug_log("ApiConnectorHelper(getApiData): Error: Can't connect to api...");
        }

        // raw http request would look like this
        // const char request[] =
        //     "GET / HTTP/1.1\r\n"
        //     "Host: " api_uri "\r\n"
        //     "User_Agent: eshelf\r\n"
        //     "Accept: */*\r\n"
        //     "Connection: close\r\n"
        //     "\r\n";
    }

public:
    ApiConnectorHelper(String args, HardwareSerial *serial = nullptr, Adafruit_SSD1306 *display = nullptr){
        if(serial){
            this->serial = serial;
        }

        if(display){
            this->display = display;
        }

        init(args);
    }
};

#endif
