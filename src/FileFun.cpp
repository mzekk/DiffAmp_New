#include "Arduino.h"
#include "FileFun.h"
#include "DiffAmp.h"
#include <Wire.h>
#include "pin_config.h"
#include <CSV_Parser.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>
#include <string>
#include "stdarg.h"
#include "esp_rom_crc.h" // For crc32_le
#include "../lib/PSRAM_Allocator/PSRAM_Allocator.h"
#include "SysOptions.h"
#include "../lib/optionsManager/OptionsModel.h"
//#include "EspNowManager.h"

sysOptions PredefinedOptions = {
        1	        , //ADC_VbattCorr
        0.99872	    , //ADC_VmeasR1NegGain
        -0.00111	, //ADC_VmeasR1NegOffset 
        0.99872	    , //ADC_VmeasR2NegGain
        -0.00111	, //ADC_VmeasR2NegOffset
        -0.0111	    , //ADC_VmeasR1Offset
        0.99956	    , //ADC_VmeasR1PosGain
        -0.021	    , //ADC_VmeasR1PosOffset
        -0.0111	    , //ADC_VmeasR2Offset
        0.99956	    , //ADC_VmeasR2PosGain
        -0.021	    , //ADC_VmeasR2PosOffset
        1	        , //ADC_VrefGain
        0	        , //ADC_VrefOffset
        1	        , //ADC_x2_GainCorr
        1	        , //ADC_x4_GainCorr
        1	        , //ADC_x8_GainCorr
        1	        , //ADC_x16_GainCorr
        1	        , //ADC_x32_GainCorr
        1	        , //ADC_x64_GainCorr
        1	        , //ADC_x128_GainCorr
        3.4	        , //BattLowThreshold
        3.6	        , //BattOkThreshold
        1	        , //DAC_V_Iref_Gain
        0	        , //DAC_V_Iref_Offset
        1	        , //DAC_V_Iset_Gain
        0	        , //DAC_V_Iset_Offset
        1	        , //DAC_Voffs_Gain
        0	        , //DAC_Voffs_Offset
        1	        , //DAC_Vsetn_Gain
        0	        , //DAC_Vsetn_Offset
        1	        , //DAC_Vsetp_Gain
        0	        , //DAC_Vsetp_Offset
        5000	    , //Diode_I_LED
        100	        , //Diode_I_LowVF
        100	        , //Diode_I_Zener
        4	        , //Diode_V_LED
        2	        , //Diode_V_LowVF
        12	        , //Diode_V_Zener
        10000	    , //Ohm_I_1R
        3000	    , //Ohm_I_1kR
        300	        , //Ohm_I_10kR
        30	        , //Ohm_I_100kR
        3	        , //Ohm_I_1MR
        1	        , //Ohm_I_10MR
        3	        , //Ohm_V_1R
        3	        , //Ohm_V_1kR
        3	        , //Ohm_V_10kR
        3	        , //Ohm_V_100kR
        3	        , //Ohm_V_1MR
        3	        , //Ohm_V_10MR
        20	        , //SMU_I_Lim
        5	        , //SMU_V_Lim
        0	        , //BattProtect
        0	        , //DiodeMeasMode
        0	        , //Diode_Buzz
        0	        , //Ohm_Buzz
        0	        , //Ohm_MeasMode
        0	        , //Opt_FlipScreen
        1	        , //SleepWithCharger
        1	        , //TimeDaylightOffset
        3	        , //ADC_Averages
        2	        , //ADC_Sample_Rate
        0	        , //ADC_VdiffGain
        1	        , //ADC_VnPGain
        72	      , //BacklightBrightness
        6	        , //BacklightLowBrght
        15	      , //BacklightTout
        30	      , //BattMinChargeLeft
        0	        , //DiodeType
        0	        , //LastScreen
        0	        , //Ohm_Range
        0x0111	  , //Revision
        12	      , //SleepMaxTime
        43	      , //SleepNumCyclesToMeas
        700	      , //SleepTimeCycleMs
        0	        , //StandbyTout
        3000	    , //SwitchTurnOffTime
        0	        , //Volt_Range
        -8	        , //TimeZone
        "24:62:AB:F5:01:48"	, //MAC_Address_Device
        "68:B6:B3:23:38:8C"	, //MAC_Address_Remote
        0	        //dataChecksum - This will be calculated on first use.
};

// Define types that use our custom PSRAM allocator
using psram_string = std::basic_string<char, std::char_traits<char>, PSRAM_Allocator<char>>;
using psram_map_string_string = std::map<psram_string, psram_string, std::less<psram_string>,
                                         PSRAM_Allocator<std::pair<const psram_string, psram_string>>>;

// The global map for comments now uses PSRAM, freeing up the main heap.
psram_map_string_string optionComments;

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -28800; // US Pacific
//const long gmtOffset_sec = 3600;         // Central Europe
const int   daylightOffset_sec = 3600;
String timeZoneStr = "UTC";  // Default fallback

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

extern char time_str[30], date_str[12], time2_str[10];
extern struct sysOptions sysOpt;
extern struct_message incomingReadings;
extern String macAddressMaster;
extern const char* optionsPath;
extern const char* optionsTestSave;

extern float incomingVdiff;
extern float incomingVbatt;
extern float incomingVSE;

uint32_t calcOptionDataChecksum(const sysOptions &options);


//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables
unsigned long previousMillis = 0;
const long interval = 20000;  // interval to wait for Wi-Fi connection (milliseconds)

// Set LED GPIO
const int ledPin = 2;
// Stores LED state

String ledState;
bool espNowPaused = false;


void serialPrintDebug(const char * message, ...) {
  if (PRINT_DEBUG_MESSAGES == 1) {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
  }
}


// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin()) {
    log_e("LittleFS mount failed, formatting...");
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS format failed!");
        while (true); // stop here
    }
}
log_e("LittleFS mounted successfully.");
}

String webUser, webPass;

/**
 * @brief Loads the web interface credentials from a JSON file.
 *
 * Opens the file "/config.json" in the LittleFS and parses it as a JSON object.
 * The JSON object must contain two string values: "username" and "password".
 * If the file does not exist or the JSON object is invalid, false is returned.
 * Otherwise, the credentials are stored in the global variables webUser and webPass.
 * @return true if the credentials were loaded successfully, false otherwise.
 */
bool loadCredentials() {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    serialPrintDebug("Failed to open config file\n");
    return false;
  }

  size_t size = configFile.size();
  std::unique_ptr<char[]> buf(new char[size + 1]);
  configFile.readBytes(buf.get(), size);
  buf[size] = '\0';

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, buf.get());

  if (error) {
    serialPrintDebug("Failed to parse config file\n");
    return false;
  }

  webUser = doc["username"].as<String>();
  webPass = doc["password"].as<String>();

  serialPrintDebug("Loaded credentials: %s / %s\n", webUser.c_str(), webPass.c_str());
  return true;
}

/**
 * @brief Authenticates a web request using stored credentials.
 *
 * This function checks if the incoming web request is authenticated
 * by comparing the credentials provided in the request with the stored
 * username and password.
 *
 * @param request A pointer to the AsyncWebServerRequest object representing
 * the incoming HTTP request.
 * @return true if the request is authenticated successfully, false otherwise.
 */

bool isAuthenticated(AsyncWebServerRequest *request) {
  return request->authenticate(webUser.c_str(), webPass.c_str());
}

/**
 * @brief Connects to a Wi-Fi network and starts a web server to
 * manage the network settings.
 *
 * This function reads the SSID, password, IP address, and gateway
 * from the LittleFS file system and attempts to connect to a Wi-Fi
 * network. If the connection is successful, it starts a web server
 * to manage the network settings. The web server allows the user to
 * enter a new SSID, password, IP address, and gateway. The new values
 * are written to the LittleFS file system and the ESP is restarted to
 * apply the changes.
 * 
 * The Webserver Handlers supported are:
 * - Upload a file to LittleFS (/upload)
 * - Download a file from LittleFS (/download)
 * - Delete a file from LittleFS (/delete)
 * - List files in LittleFS (/list) 
 * - View a file in LittleFS (/view)
 * - Set LED ON state (/on) (No longer used)
 * - Get LED OFF state (/off) (No longer used)
 * 
 * 
 * @return true if the Wi-Fi connection is successful and the web
 * server is started, false otherwise.
 */
bool WiFi_Service(void){
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  //serialPrintDebug("ssid: %s \n, pass: %s\n, ip: %s\n, gateway: %s\n", ssid.c_str(), pass.c_str(), ip.c_str(), gateway.c_str());

  loadCredentials();  // inside setup()  

  // Load values saved in LittleFS
  
  if(initWiFi_AP()) {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    if (!LittleFS.exists("/index2.html.gz")) {
      request->send(500, "text/plain", "Missing index.html.gz");
      return;
    }
    AsyncWebServerResponse* response = request->beginResponse(LittleFS, "/index2.html.gz", "text/html");
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });

// Route to load form for file Deleted
    server.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request){
      if (!request->hasParam("name", true)) {
        request->send(400, "text/plain", "Missing file name");
        return;
      }
      String filename = request->getParam("name", true)->value();
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }
      log_printf("Deleting file: %s\n", filename.c_str());
  
      if (LittleFS.exists(filename)) {
        LittleFS.remove(filename);
        request->send(200, "text/plain", "Deleted");
      } 
      else {
        request->send(500, "text/plain", "File not found");
      }
    });

// Route to load form for file download from LittleFS
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
      if (!request->hasParam("file")) {
        request->send(400, "text/plain", "Missing 'file' parameter");
      return;
      }

      String filename = request->getParam("file")->value();
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }

      if (!LittleFS.exists(filename)) {
        log_printf("File not found: %s\n", filename.c_str());
        request->send(404, "text/plain", "File not found");
        return;
      }
      log_printf("Viewing file: %s\n", filename.c_str());
      request->send(LittleFS, filename, "application/octet-stream", true); 
    });


// Route to list directory
    server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request){
      String output = "[";
      File root = LittleFS.open("/");
      File file = root.openNextFile();
      bool first = true;

      while (file) {
        if (!first) output += ",";
        output += "{\"name\":\"" + String(file.name()) + "\",\"size\":" + String(file.size()) + "}";
        first = false;
        file = root.openNextFile();
      }
      output += "]";
      request->send(200, "application/json", output);
    });

// Route for file upload  
    server.on("/upload", HTTP_POST, 
    [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Upload complete");
    }, 
    [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
      static File uploadFile;

      if (!index) {
        log_printf("Upload start: %s\n", filename.c_str());
        uploadFile = LittleFS.open("/" + filename, FILE_WRITE);
        if (!uploadFile) {
          log_printf("Failed to open file for writing\n");
          return;
        }
      }
      if (uploadFile) {
        uploadFile.write(data, len);
      }

      if (final) {
        uploadFile.close();
        log_printf("Upload complete: %s, %d bytes\n", filename.c_str(), index + len);
      }
    }
    );

// View command to list file content
  server.on("/view", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("file")) {
      request->send(400, "text/plain", "Missing file param");
      return;
    }

    String filename = request->getParam("file")->value();
    if (!filename.startsWith("/")) filename = "/" + filename;

    if (!LittleFS.exists(filename)) {
      request->send(404, "text/plain", "File not found");
      return;
    }

    File file = LittleFS.open(filename, FILE_READ);
    if (!file) {
      request->send(500, "text/plain", "Failed to open file");
      return;
    }

    request->sendChunked("text/plain", [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
      if (!file.available()) {
        file.close();
        return 0; // done
        }

        size_t len = file.read(buffer, maxLen);
        return len;
      });
    });     

  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    log_printf("GPIO ON");
    request->send(200, "text/plain", "GPIO ON");
  });

  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    log_printf("GPIO OFF");
    request->send(200, "text/plain", "GPIO OFF");
  });
  server.begin();
  log_printf("HTTP server started");
    
  return true; //
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    serialPrintDebug("Setting AP (Access Point)\n");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", "esp32pass");

    IPAddress IP = WiFi.softAPIP();
    serialPrintDebug("AP IP address: %s\n", IP);

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      //request->send(LittleFS, "/wifimanager.html", "text/html");
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    //server.serveStatic("/", LittleFS, "/");
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);

        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            serialPrintDebug("SSID set to: %s \n", ssid.c_str());
            // Write file to save value
            //writeFile(LittleFS, ssidPath, ssid.c_str());
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            serialPrintDebug("Password set to: %s \n", pass.c_str());
            // Write file to save value
            //writeFile(LittleFS, passPath, pass.c_str());
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            serialPrintDebug("IP Address set to: %s \n", ip.c_str());
            // Write file to save value
            //writeFile(LittleFS, ipPath, ip.c_str());
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            serialPrintDebug("Gateway set to: %s \n", gateway.c_str());
            // Write file to save value
            //writeFile(LittleFS, gatewayPath, gateway.c_str());
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
          //serialPrintDebug("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }
  return false;
}

// Read File from LittleFS including comments
String readFile(fs::FS &fs, const char * path){
  serialPrintDebug("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    serialPrintDebug("- failed to open file for reading\n");
    return String();
  }
  
  String fileContent = file.readString();;

  file.close();
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  serialPrintDebug("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    serialPrintDebug("- failed to open file for writing\n");
    return;
  }
  if(file.print(message)){
    serialPrintDebug("- file written\n");
  } else {
    serialPrintDebug("- frite failed\n");
  }
}

// Initialize WiFi
bool initWiFi() {
  const char* ssid       = "xxxxxxxxx";
  const char* pwd   = "xxxxxxxx";
  const char* ip   = "10.0.0.43";
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, pwd);
  //WiFi.begin(ssid.c_str(), pass.c_str());
  log_e("Connecting to WiFi...");
  return true;
}

bool initWiFi_AP() {
  if(ssid=="" || ip==""){
    serialPrintDebug("Undefined SSID or IP address.\n");
    return false;
  }

  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_AP);

  WiFi.softAP("ESP32-DiffAmp", "esp32pass");
  log_printf("AP IP: %S",WiFi.softAPIP().toString());
  
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  WiFi.begin(ssid.c_str(), pass.c_str());
  serialPrintDebug("Connecting to WiFi...\n");

  //EspNowManager::configureCallbacks(masterMacAddress, OnDataRecv, OnDataSent);
  //EspNowManager::begin(masterMacAddress, OnDataRecv, OnDataSent);

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  return true;
}

bool checkWiFiConnected(void){
  if(WiFi.status() == WL_CONNECTED){
    serialPrintDebug("Connected to WiFi\n");

    extern String macAddress;
    macAddress = "MAC Addr: " + WiFi.macAddress();
    serialPrintDebug("%s\n",macAddress.c_str());
    long gmtOffsSec = sysOpt.TimeZone * 3600;
    long daylightOffsSec = int(sysOpt.TimeDaylightOffset) * 3600;
    configTime(gmtOffsSec, daylightOffsSec, ntpServer);
    //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    //detectTimezone();   // Supposed to detect the timezone where the device is connected but does not work
    //configTzTime(timeZoneStr.c_str(), ntpServer); // Same a line above
    //WiFi.disconnect(true);
    //WiFi.mode(WIFI_OFF);
    //setenv("TZ","PST8PDT,M3.2.0,M11.1.0",1);
    if (esp_now_init() != ESP_OK) {
      serialPrintDebug("Error initializing ESP-NOW\n");
      return false;
    }
    // Once ESPNow is successfully Init, we will register for Send CB to
    // get the status of Trasnmitted packet
    esp_now_register_send_cb(OnDataSent);
    // Register peer
    esp_now_peer_info_t peerInfo = {};

    macAddressToBytes(macAddressMaster.c_str(), masterMacAddress);
    lv_label_set_text(ui_DeviceMACAddrLbl, macAddress.c_str());
    memset(&peerInfo, 0, sizeof(peerInfo));
    memcpy(peerInfo.peer_addr, masterMacAddress, 6);
    serialPrintDebug("Remote MAC Addr: %02x:%02x:%02x:%02x:%02x:%02x\n", masterMacAddress[0], masterMacAddress[1], masterMacAddress[2], masterMacAddress[3], masterMacAddress[4], masterMacAddress[5]);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    peerInfo.ifidx=WIFI_IF_AP;

    // Add peer        
    if (esp_now_add_peer(&peerInfo) != ESP_OK){
      serialPrintDebug("Failed to add peer\n");
      return false;
    }
    // Register for a callback function that will be called when data is received
    esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
    return true;
  }
  return false;  
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //serialPrintDebug("\r\nLast Packet Send Status:\t");
  //serialPrintDebug(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail\n");
  if (status ==0){
    espNowSendStatus = "Delivery Success :)";
  }
  else{
    espNowSendStatus = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  serialPrintDebug("Bytes received: %d\n", len);
  incomingVdiff = incomingReadings.vdiff;
  incomingVbatt = incomingReadings.vbatt;
  incomingVSE = incomingReadings.vse;
}

void detectTimezone() {
  HTTPClient http;
  http.begin("http://worldtimeapi.org/api/ip");

  int httpCode = http.GET();
  if (httpCode == 200) {
    String payload = http.getString();

    JsonDocument doc;
    deserializeJson(doc, payload);

    timeZoneStr = doc["timezone"].as<String>(); // e.g., "America/Los_Angeles"
    serialPrintDebug("Detected timezone: %s\n",timeZoneStr);
  } else {
    serialPrintDebug("Failed to get timezone: HTTP %d\n", httpCode);
  }
  http.end();
}

uint16_t checkTime(bool* connected, uint16_t time){
  if (*connected && time >= TIME500ms) {
    time = 0;
    //serialPrintDebug("Free heap before ESP-NOW send: %d\n", ESP.getFreeHeap());
    if(espNowPaused == true) {
      serialPrintDebug("ESP-NOW is Paused\n");
    }
    else {
      
      esp_err_t result = esp_now_send(masterMacAddress, (uint8_t *) &voltReadings, sizeof(voltReadings));
      
      //if (result == ESP_OK) 
      //  serialPrintDebug("Sent with success\n");
      //else serialPrintDebug("ESP-NOW send failed: %d (%s)\n", result, esp_err_to_name(result));
      if (printLocalTime()){
        lv_textarea_set_text(ui_timeInfo, time2_str); 
        lv_obj_set_style_img_recolor(ui_WifiImg, lv_color_hex(0xF7F039), LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_img_recolor_opa(ui_WifiImg, 255, LV_PART_MAIN | LV_STATE_DEFAULT);  
      }
    }
  }
  if(!*connected && time >= TIME500ms) {
      time = 0;
      *connected = checkWiFiConnected();
      //lv_obj_set_style_img_recolor(ui_Image1, lv_color_hex(0xF0EFEF), LV_PART_MAIN | LV_STATE_DEFAULT);
      //lv_obj_set_style_img_recolor(ui_Image1, lv_color_hex(0), LV_PART_MAIN | LV_STATE_DEFAULT);
      lv_obj_set_style_img_recolor_opa(ui_WifiImg, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
      if(*connected)
        lv_textarea_set_text(ui_timeInfo, "WIFI OK");
      else lv_textarea_set_text(ui_timeInfo, "No WIFI");  
  }
  return time;
}

bool printLocalTime() {
  struct tm info;
  time_t now;
  time(&now);
  localtime_r(&now, &info);
  if(info.tm_year > (2016 - 1900)){
      strftime (time2_str,10,"%H:%M:%S",&info);
      //serialPrintDebug("%s\n",time2_str);
      return true;
    }
  return false;
}

// Replaces placeholder with LED state value
String processor(const String& var) {
  if(var == "STATE") {
    if(digitalRead(ledPin)) {
      ledState = "ON";
    }
    else {
      ledState = "OFF";
    }
    return ledState;
  }
  return String();
}

/**
 * @brief Reads options from a CSV file into the options struct in a memory-efficient way.
 *        This function reads the file line-by-line to avoid loading the entire
 *        file into memory. It is driven by the optionDescriptors table.
 * @param fileName The full path to the options file (e.g., "/MemOptionsDiffAmp.csv").
 * @param options A reference to the sysOptions struct to populate.
 * @return True on success, false if the file can't be opened or is invalid.
 */
bool getOptionsfromFile(const char *fileName, sysOptions &options) {
    File optionsFile = LittleFS.open(fileName, "r");
    if (!optionsFile) {
        serialPrintDebug("ERROR: File not found: %s. Loading defaults.\n", fileName);
        options = PredefinedOptions;
        options.dataChecksum = calcOptionDataChecksum(options);
        return false;
    }

    optionComments.clear(); // Clear old comments from PSRAM before loading new file

    // Read header line and discard it
    if (optionsFile.available()) {
        optionsFile.readStringUntil('\n');
    }

    uint32_t storedChecksum = 0;

    while (optionsFile.available()) {
        String line = optionsFile.readStringUntil('\n');
        line.trim();
        if (line.length() == 0) continue;

        // Manually parse the CSV line
        int firstComma = line.indexOf(',');
        int secondComma = line.indexOf(',', firstComma + 1);
        int thirdComma = line.indexOf(',', secondComma + 1);

        if (firstComma == -1 || secondComma == -1) continue; // Need at least name and value

        String key = line.substring(0, firstComma);
        String valueStr = line.substring(secondComma + 1, thirdComma);
        String commentStr = (thirdComma != -1) ? line.substring(thirdComma + 1) : "";

        // Store the original comment for this key in PSRAM
        optionComments[key.c_str()] = commentStr.c_str();

        // Special case for the checksum
        if (key == "dataChecksum") {
            storedChecksum = strtoul(valueStr.c_str(), NULL, 16);
            continue;
        }

        bool keyFound = false;
        // Find the matching descriptor for the key
        for (size_t i = 0; i < numOptionDescriptors; ++i) {
            const OptionDescriptor& desc = optionDescriptors[i];
            if (key == desc.name) {
                // Pointer to the actual member in the options struct
                void* memberPtr = (uint8_t*)&options + desc.offset;

                // Set the value based on its type from the descriptor
                switch (desc.type) {
                    case TYPE_FLOAT:
                        *(static_cast<float*>(memberPtr)) = valueStr.toFloat();
                        break;
                    case TYPE_BOOL:
                        *(static_cast<bool*>(memberPtr)) = (valueStr == "true" || valueStr == "1");
                        break;
                    case TYPE_UINT16:
                        *(static_cast<uint16_t*>(memberPtr)) = (uint16_t)valueStr.toInt();
                        break;
                    case TYPE_INT16:
                        *(static_cast<int16_t*>(memberPtr)) = (int16_t)valueStr.toInt();
                        break;
                    case TYPE_STRING:
                        strncpy(static_cast<char*>(memberPtr), valueStr.c_str(), desc.size - 1);
                        (static_cast<char*>(memberPtr))[desc.size - 1] = '\0'; // Ensure null termination
                        break;
                }
                keyFound = true;
                break; // Found and processed, move to the next row
            }
        }

        if (!keyFound) {
            serialPrintDebug("WARNING: Unknown option in %s: %s\n", fileName, key.c_str());
        }
    }

    optionsFile.close();

    // Now, validate the checksum
    uint32_t calculatedChecksum = calcOptionDataChecksum(options);
    if (calculatedChecksum == storedChecksum) {
        serialPrintDebug("Options loaded successfully from %s. Checksum OK (0x%X)\n", fileName, calculatedChecksum);
        options.dataChecksum = calculatedChecksum; // Store the valid checksum
        return true;
    } else {
        serialPrintDebug("ERROR: Checksum mismatch in %s. Stored: 0x%X, Calculated: 0x%X. Loading predefined defaults.\n", fileName, storedChecksum, calculatedChecksum);
        options = PredefinedOptions; // Load defaults
        options.dataChecksum = calcOptionDataChecksum(options); // Recalculate checksum for defaults
        return false; // Indicate that defaults were loaded
    }
}

/**
 * @brief Writes the options struct to a CSV file, preserving original comments
 *        in a memory-efficient way.
 * @param fileName The full path to the options file.
 * @param options The sysOptions struct containing the data to save.
 * @return True on success, false on failure.
 */
bool WriteOptionsToFile(const char *fileName, const sysOptions &options) {
    // This function no longer needs to read the original file, as comments are stored in the PSRAM map.
    File optionsFile = LittleFS.open(fileName, "w");
    if (!optionsFile) {
        serialPrintDebug("ERROR: Failed to open file for writing: %s\n", fileName);
        return false;
    }

    // Write header
    optionsFile.println("Option_name,Type,Option_value,comment");

    for (size_t i = 0; i < numOptionDescriptors; ++i) {
        const OptionDescriptor& desc = optionDescriptors[i];
        const void* memberPtr = (const uint8_t*)&options + desc.offset;

        optionsFile.print(desc.name);
        optionsFile.print(",");

        char typeChar = '?';
        switch (desc.type) {
            case TYPE_FLOAT:  typeChar = 'f'; break;
            case TYPE_BOOL:   typeChar = 'b'; break;
            case TYPE_UINT16: typeChar = 'u'; break;
            case TYPE_INT16:  typeChar = 'i'; break;
            case TYPE_STRING: typeChar = 's'; break;
        }
        optionsFile.print(typeChar);
        optionsFile.print(",");

        switch (desc.type) {
            case TYPE_FLOAT:
                optionsFile.print(*(static_cast<const float*>(memberPtr)), 6);
                break;
            case TYPE_BOOL:
                optionsFile.print(*(static_cast<const bool*>(memberPtr)) ? "1" : "0");
                break;
            case TYPE_UINT16:
                optionsFile.print(*(static_cast<const uint16_t*>(memberPtr)));
                break;
            case TYPE_INT16:
                optionsFile.print(*(static_cast<const int16_t*>(memberPtr)));
                break;
            case TYPE_STRING:
                optionsFile.print(static_cast<const char*>(memberPtr));
                break;
        }

        // Write the original comment back from our PSRAM-backed map
        optionsFile.print(",");
        auto it = optionComments.find(psram_string(desc.name));
        if (it != optionComments.end()) {
            optionsFile.println(it->second.c_str());
        } else {
            optionsFile.println("-"); // Default placeholder comment if none was found
        }
    }

    uint32_t optionsChecksum = calcOptionDataChecksum(options); // Recalculate checksum for defaults
    optionsFile.print("dataChecksum,u,0x");
    optionsFile.print(optionsChecksum, HEX);
    optionsFile.println(",-"); // Add comma before the placeholder comment

    optionsFile.close();
    serialPrintDebug("Options saved to %s\n", fileName);
    return true;
}


/**
 * @brief Calculates a robust CRC32 checksum of the options struct.
 * @note This version iterates through the descriptor table, ensuring that only
 *       actual data members are included in the checksum, ignoring any
 *       compiler-inserted padding bytes. This makes the checksum stable
 *       across different builds and compiler versions.
 * @param options The sysOptions structure to be checksummed.
 * @return A 32-bit CRC checksum.
 */
uint32_t calcOptionDataChecksum(const sysOptions &options) {
    uint32_t crc = 0; // Initial CRC value

    for (size_t i = 0; i < numOptionDescriptors; ++i) {
        const OptionDescriptor& desc = optionDescriptors[i];
        const uint8_t* memberPtr = (const uint8_t*)&options + desc.offset;
        size_t memberSize = 0;

        switch (desc.type) {
            case TYPE_FLOAT:  memberSize = sizeof(float); break;
            case TYPE_BOOL:   memberSize = sizeof(bool); break;
            case TYPE_UINT16: memberSize = sizeof(uint16_t); break;
            case TYPE_INT16:  memberSize = sizeof(int16_t); break;
            case TYPE_STRING: memberSize = strnlen((const char*)memberPtr, desc.size); break;
        }

        if (memberSize > 0) {
            crc = esp_rom_crc32_le(crc, memberPtr, memberSize);
        }
    }
    return crc;
}


/**
 * @brief Convert a MAC address from a string to an array of bytes.
 * 
 * @param macAddress The MAC address as a string, e.g. "FF:FF:FF:FF:FF:FF"
 * @param bytes An array of six uint8_t elements to store the MAC address in binary form.
 */
void macAddressToBytes(const char* macAddress, uint8_t bytes[6]) {
    char token[3];
    int index = 0;

    while (*macAddress != '\0') {
        if (*macAddress == ':') {
            macAddress++;
            continue;
        }

        token[0] = *macAddress++;
        token[1] = *macAddress++;
        token[2] = '\0';

        bytes[index++] = (uint8_t)strtol(token, NULL, 16);
    }
}
