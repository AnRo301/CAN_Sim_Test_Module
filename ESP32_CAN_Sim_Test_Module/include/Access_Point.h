#ifndef ACCESS_POINT_H
#define ACCESS_POINT_H

#include <ESPAsyncWebServer.h>
#include <Wifi.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>
#include <Arduino.h>
#include <vector>
#include <iostream>
#include <sstream>

using namespace std;

/* --------------------- Definitions and static variables ------------------ */

typedef struct {
    std::vector<uint8_t> magic = {0x49, 0x53, 0x4f, 0x31, 0x31, 0x38, 0x39, 0x38};
    std::vector<uint8_t> version = {0x01};
    std::vector<uint8_t> framesCnt = {0x01};
    std::vector<uint8_t> options = {0x01, 0x60, 0xff};
    std::vector<uint8_t> identifier; 
    std::vector<uint8_t> frameLength; 
    std::vector<uint8_t> data; 
    std::vector<uint8_t> extFlag; 
    std::vector<uint8_t> rtrFlag;
} CANFrame;

/* --------------------------- Tasks and Functions -------------------------- */

void Init_Spiffs();
void Init_WS();
void Init_AP();

void sendRandCANData();
void sendReceivedCANData(CANFrame canFrame);
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char *path, const char *message);
String processor(const String &var);
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
CANFrame Rand_CAN_Frame();
vector<uint8_t> CANFrame_to_Vec(CANFrame canFrame);
String CANFrame_to_JSON(CANFrame canFrame);
void serveFile(AsyncWebServerRequest *request, const char *filename, const char *contentType);
void BroadcastUDP(std::vector<uint8_t> data, size_t len);
void WS_Handling();
uint8_t combineDigits(uint8_t num1, uint8_t num2);

#endif