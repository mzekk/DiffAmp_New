#ifndef Filefun_h
#define FileFun_h
#include "DiffAmp.h"
#include <WiFi.h>
#include <esp_now.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
//#include <LittleFs.h>
//#include <LittleFS.h>
//#include <SPIFFS.h>
#include <LittleFS.h>
#include "..\src\ui\ui.h"


void initLittleFS(void);
bool WiFi_Service(void);
String readFile(fs::FS &fs, const char * path);
String readFileNoComments(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
bool initWiFi(void);
bool initWiFi_AP(void);
uint16_t checkTime(bool *connected, uint16_t time);
bool checkWiFiConnected(void);
void detectTimezone();
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len);
bool printLocalTime(void);
void macAddressToBytes(const char* macAddress, uint8_t bytes[6]);
String processor(const String& var);
extern String espNowSendStatus;
extern uint8_t masterMacAddress[6];
extern struct_message voltReadings;
//const char* optionsPath;
#endif