#ifndef json_helper_h
#define json_helper_h

#ifndef JSON_HELPER_DEBBUG_SERIAL
    #define JSON_HELPER_DEBBUG_SERIAL false // send debbug messages to serial
#endif

#ifndef JSON_HELPER_DEBBUG_DISPLAY
    #define JSON_HELPER_DEBBUG_DISPLAY false // send debbug messages to display
#endif

#include <WString.h>
#include <ArduinoJson.h>
#include "libraries/json_syntax.h"
#include "libraries/ntag213.h"

#define STATUS_OK 0
#define STATUS_FAIL 1
#define STATUS_JSON_INVALID 2
#define STATUS_JSON_TOO_SHORT 3


/**
 * @brief Helps handling the json data so we dont that to worry too much about that.
 */
class JsonParserHelper{
public:
    typedef unsigned short int status;

protected:
    status status_code;

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
            case STATUS_JSON_INVALID:
                return String("InvalidJson");
            case STATUS_JSON_TOO_SHORT:
                return String("JsonTooShort");
            default:
                return String("Unknown");
        }
    }

protected:
    HardwareSerial *serial;
    Adafruit_SSD1306 *display;

    StaticJsonDocument<NTAG_STORAGE_SPACE> doc;
    JsonObject json_obj;
    String json;

    #if JSON_HELPER_DEBBUG_SERIAL
        void debbug_log_serial(String msg){
            serial->printf(msg.c_str());
        }
    #endif

    #if JSON_HELPER_DEBBUG_DISPLAY
        void debbug_log_display(String msg){
            display->print(msg.c_str());
            display->display();
        }
    #endif
    
    void debbug_log(String msg){
        #if JSON_HELPER_DEBBUG_SERIAL
            debbug_log_serial(msg);
        #endif

        #if JSON_HELPER_DEBBUG_DISPLAY
            debbug_log_display(msg);
        #endif
    }

    void debbug_log(String msg_serial, String msg_display){
        #if JSON_HELPER_DEBBUG_SERIAL
            debbug_log_serial(msg_serial);
        #endif

        #if JSON_HELPER_DEBBUG_DISPLAY
            debbug_log_display(msg_display);
        #endif
    }

    // TODO move this to TagParserHelper
    /**
     * @brief 
     *     Removes prepended and appended extra bytes which do not encode Json data.
     *     Sets a clean version of the input Json String candidate as a String without the extra junk.
     * @Note
     *     This does not do any type of Json syntax validation.
     * @Warning
     *     if there are adicional Json tokens inside the first and last valid tokens
     *     they are ignored. Just the outermost tokens matter in this case.
     */
    void cleanDirtyJson(String json_dirty){
        // too small to be a valid json
        if(json_dirty.length() < JSON_MIN_VALID_LENGTH){
            setStatus(STATUS_JSON_TOO_SHORT);
            return;
        }

        unsigned int start_index = 0;
        unsigned int end_index = json_dirty.length() - 1;
        int i; // cannot be unsigned due to underflow that leads to crash

        for(i = 0; i < json_dirty.length(); ++i){
            debbug_log("\nJsonParserHelper(cleanDirtyJson): Start->End - Testing char[" + String(i) + "] = '" + String(json_dirty.charAt(i)) + "'");

            if(json_dirty.charAt(i) == JSON_START_TOKEN){
                start_index = i;
                break;
            }
        }

        for(i = json_dirty.length() - 1; i >= 0; --i){
            debbug_log("\nJsonParserHelper(cleanDirtyJson): End->Start - Testing char[" + String(i) + "] = '" + String(json_dirty.charAt(i)) + "'");

            if(json_dirty.charAt(i) == JSON_END_TOKEN){
                end_index = i;
                break;
            }
        }

        debbug_log(
            "\nJsonParserHelper(cleanDirtyJson):\n\
            Start - json_dirty[" + String(start_index) + "] = '" + String(json_dirty[start_index]) + "'\n\
            End   - json_dirty[" + String(end_index)   + "] = '" + String(json_dirty[end_index])   + "'\n"
        );

        // tokens don't match with the expected
        if(json_dirty[start_index] != JSON_START_TOKEN || json_dirty[end_index] != JSON_END_TOKEN){
            setStatus(STATUS_JSON_INVALID);
            return;
        }

        // no valid token pair found
        if(start_index >= end_index){
            setStatus(STATUS_JSON_INVALID);
            return;
        }

        // too small to be a valid json
        if(end_index - start_index + 1 < JSON_MIN_VALID_LENGTH){
            setStatus(STATUS_JSON_TOO_SHORT);
            return;
        }

        String json_clean = json_dirty.substring(start_index, end_index + 1);

        if(json_clean == ""){
            setStatus(STATUS_FAIL);
        }

        json = json_clean;

        setStatus(STATUS_OK);

        return;
    }

    /**
     * @brief Sets the status.
     * @param Status code.
     */
    void setStatus(status status_code){
        this->status_code = status_code;
    }

    /**
     * @brief Parses the json string to a json object we can work with.
     */
    void setJsonObject(){
        DeserializationError error = deserializeJson(doc, json);

        if(error){
            debbug_log(String("\nJsonParserHelper(setJsonObject): Error: ") + String(error.c_str()));
        }else{
            debbug_log("\nJsonParserHelper(setJsonObject): Deserialization successful");

            json_obj = doc.as<JsonObject>();
        }
    }

    /**
     * @brief Initializes the object and cleans the raw json.
     * @param Raw json that can have junky bytes.
     */
    void init(String json_dirty){
        setStatus(STATUS_OK);

        cleanDirtyJson(json_dirty);

        if(getStatus()){
            debbug_log(String("\nJsonParserHelper(constructor): Error: " + getStatusStr()));
        }else{
            setJsonObject();
        }
    }

public:
    JsonParserHelper(String json_dirty, HardwareSerial *serial = nullptr, Adafruit_SSD1306 *display = nullptr){
        if(serial){
            this->serial = serial;
        }

        if(display){
            this->display = display;
        }

        init(json_dirty);
    }
};

class TagParserHelper: public JsonParserHelper{
public:
    // typedef char type_id[NTAG_STORAGE_SPACE];
    // typedef char type_token[NTAG_STORAGE_SPACE];
    typedef char* type_id;
    typedef char* type_token;

    type_id id;
    type_id token;

protected:
    /**
     * @brief Sets the values gotten from the inputted json.
     */
    void setValues(){
        if(!doc["id"].isNull()){
            id = (char*)doc["id"].as<char*>();
            setStatus(STATUS_OK);
        }else{
            setStatus(STATUS_JSON_INVALID);
            return;
        }

        // this may be used in a future implementation
        // if(!doc["token"].isNull()){
        //     token = (char*)doc["token"].as<char*>();
        //     setStatus(OK);
        // }else{
        //     setStatus(STATUS_JSON_INVALID);
        //     return;
        // }
    }

public:
    TagParserHelper(String json_dirty, HardwareSerial *serial = nullptr, Adafruit_SSD1306 *display = nullptr):
    JsonParserHelper(json_dirty, serial, display){
        setValues();
    }
};

class ApiParserHelper: JsonParserHelper{

};

#endif
