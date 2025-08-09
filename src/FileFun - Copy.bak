#include "Arduino.h"
#include "FileFun.h"
#include "DiffAmp.h"
#include <Wire.h>
#include "pin_config.h"
#include <CSV_Parser.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "stdarg.h"
//#include "EspNowManager.h"

sysOptions PredefinedOptions = {
        1	        , //ADC_VbattCorr
        0.99872	    , //ADC_VmeasNegR1Gain
        -0.00111	, //ADC_VmeasNegR1Offset
        0.99872	    , //ADC_VmeasNegR2Gain
        -0.00111	, //ADC_VmeasNegR2Offset
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
        0x0111	  , //Revisions: Hw revision [bit 0-3]; SW revision Minor [bit 4-7]; SW revision Major [bit 8-11]; Submodel [bit 12-15]
        12	      , //SleepMaxTime
        43	      , //SleepNumCyclesToMeas
        700	      , //SleepTimeCycleMs
        0	        , //StandbyTout
        3000	    , //SwitchTurnOffTime
        0	        , //Volt_Range
        -8	        , //TimeZone
        "24:62:AB:F5:01:48"	, //MAC_Address_Device
        "68:B6:B3:23:38:8C"	, //MAC_Address_Remote
        0x99D6A9	 //dataChecksum
};


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

char **getCSV_Values(String dataOptions, CSV_Parser cp);
void saveCSVFile(const char * path, CSV_Parser &cp);


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

  DynamicJsonDocument doc(256);
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

// Read File from LittleFS
String readFileNoComments(fs::FS &fs, const char * path){
  serialPrintDebug("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    serialPrintDebug("- failed to open file for reading\n");
    return String();
  }
  String result = "";

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() == 0 || line[0] == '#') continue;

    int firstComma = line.indexOf(',');
    int secondComma = line.indexOf(',', firstComma + 1);
    int thirdComma = line.indexOf(',', secondComma + 1);

    if (firstComma == -1 || secondComma == -1) {
      serialPrintDebug("Invalid line: %s\n", line.c_str());
      continue;
    }

    // Trim off the comment (4th column)
    String trimmedLine = (thirdComma != -1)
                         ? line.substring(0, thirdComma)
                         : line;

    result += trimmedLine + "\n";
  }

  file.close();
  return result;
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

void saveCSVFile(const char * path, CSV_Parser &cp){
  File file = LittleFS.open(path, FILE_WRITE);  // this overwrites the file
  if (!file) {
    serialPrintDebug("Failed to open file for writing\n");
    return;
  }
  char **optionNames = (char**)cp["Option_name"];
  char **optionType = (char**)cp["Type"];
  char **optionValues = (char**)cp["Option_value"];
  char **comments = (char**)cp["comment"];

  // Write header line
  file.println("Option_name,Type,Option_value,comment");
  // Write CSV data
  size_t rows = cp.getRowsCount();
  for (size_t i = 0; i < rows; i++) {
    // Safely write each column in one line
    if (optionNames[i])  file.print(optionNames[i]);
    file.print(",");

    if (optionType[i])   file.print(optionType[i]);
    file.print(",");

    if (optionValues[i]) file.print(optionValues[i]);
    file.print(",");

    if (comments[i])     file.println(comments[i]);
    else                 file.println();
  }

  file.close();
  serialPrintDebug("CSV saved successfully.\n");
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

    DynamicJsonDocument doc(1024);
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
 * @brief Parse a CSV file with options and fill a sysOptions struct with the parsed values.
 * @param dataOptions The content of the CSV file as a string.
 * @param sysOpt The sysOptions struct to be filled with the parsed values.
 */
/**
 * The expected format of the CSV file is as follows:
 *
 * Option_name,Type,Option_value,comment
 * ADC_TifGain,float,0.999,-
 * ADC_TifOffset,float,-0.021,-
 * ...,
 * ...,
 */
/**
 * The options are:
 * - ADC_TifGain, ADC_TifOffset, ADC_VmeasNegGain, ADC_VmeasNegOffset, ADC_VmeasOffset, ADC_VmeasPosGain, ADC_VmeasPosOffset, ADC_VrefGain, ADC_VrefOffset, DAC_pVsSet, DAC_VposSet, iADC_VbattCorr, LowBattThreshold, OkBattThreshold, Res_0p1, Res_1p0, Res_10p0, Res_100p0, Res_1k0, Res_10k0, Res_100k0, Res_1M0
 * - ADC_Averages, ADC_Sample_Rate, LastResistanceSet, LastScreen, LastVmeasDirection, LastVoltageSet, LCD_Brightness
 * - EN_IgenPort, EN_ItstSense, IN_RevPort, RHoldOption, VHoldOption, VR_SelPort
 * - MAC_Address_Device, MAC_Address_Remote
 */
/**
 * The values are parsed using the atof() and atoi() functions.
 * The boolean values are parsed by checking if the value is 0 or not.
 * The strings are parsed as is.
 */
void getOptionsfromFile(String dataOptions, sysOptions* sysOpt){
  // Create a CSV parser object
  CSV_Parser cp(dataOptions.c_str(), "sss-");
  serialPrintDebug("Number of lines: %d\n", cp.getRowsCount());
  int newlineIndex = dataOptions.indexOf('\n');
  serialPrintDebug("Header (raw): '");
  char header[60];
  sprintf(header,"%s", dataOptions.substring(0, newlineIndex).c_str());
  serialPrintDebug(header);
  serialPrintDebug("'\n");
  char **optionNames = (char**)cp["Option_name"];
  char **optionType = (char**)cp["Type"];
  char **optionValues = (char**)cp["Option_value"];
  //char **comments = (char**)cp["comment"];
  // Parse the CSV data
  for(int row = 0; row < cp.getRowsCount(); row++)
    serialPrintDebug("%s, %s\n", optionNames[row], optionValues[row]);

  sysOpt->ADC_VbattCorr = atof(optionValues[OptAddr::ADC_VbattCorr]);
  sysOpt->ADC_VmeasR1NegGain = atof(optionValues[OptAddr::ADC_VmeasR1NegGain]);
  sysOpt->ADC_VmeasR1NegOffset = atof(optionValues[OptAddr::ADC_VmeasR1NegOffset]);
  sysOpt->ADC_VmeasR2NegGain = atof(optionValues[OptAddr::ADC_VmeasR2NegGain]);
  sysOpt->ADC_VmeasR2NegOffset = atof(optionValues[OptAddr::ADC_VmeasR2NegOffset]);
  sysOpt->ADC_VmeasR1Offset = atof(optionValues[OptAddr::ADC_VmeasR1Offset]);
  sysOpt->ADC_VmeasR1PosGain = atof(optionValues[OptAddr::ADC_VmeasR1PosGain]);
  sysOpt->ADC_VmeasR1PosOffset = atof(optionValues[OptAddr::ADC_VmeasR1PosOffset]);
  sysOpt->ADC_VmeasR2Offset = atof(optionValues[OptAddr::ADC_VmeasR2Offset]);
  sysOpt->ADC_VmeasR2PosGain = atof(optionValues[OptAddr::ADC_VmeasR2PosGain]);
  sysOpt->ADC_VmeasR2PosOffset = atof(optionValues[OptAddr::ADC_VmeasR2PosOffset]);
  sysOpt->ADC_VnPGain = atof(optionValues[OptAddr::ADC_VnPGain]);
  sysOpt->ADC_VrefGain = atof(optionValues[OptAddr::ADC_VrefGain]);
  sysOpt->ADC_VrefOffset = atof(optionValues[OptAddr::ADC_VrefOffset]);
  sysOpt->ADC_x2_GainCorr = atof(optionValues[OptAddr::ADC_x2_GainCorr]);
  sysOpt->ADC_x4_GainCorr = atof(optionValues[OptAddr::ADC_x4_GainCorr]);
  sysOpt->ADC_x8_GainCorr = atof(optionValues[OptAddr::ADC_x8_GainCorr]);
  sysOpt->ADC_x16_GainCorr = atof(optionValues[OptAddr::ADC_x16_GainCorr]);
  sysOpt->ADC_x32_GainCorr = atof(optionValues[OptAddr::ADC_x32_GainCorr]);
  sysOpt->ADC_x64_GainCorr = atof(optionValues[OptAddr::ADC_x64_GainCorr]);
  sysOpt->ADC_x128_GainCorr = atof(optionValues[OptAddr::ADC_x128_GainCorr]);
  sysOpt->BattLowThreshold = atof(optionValues[OptAddr::BattLowThreshold]);
  sysOpt->BattOkThreshold = atof(optionValues[OptAddr::BattOkThreshold]);
  sysOpt->DAC_V_Iref_Gain = atof(optionValues[OptAddr::DAC_V_Iref_Gain]);
  sysOpt->DAC_V_Iref_Offset = atof(optionValues[OptAddr::DAC_V_Iref_Offset]);
  sysOpt->DAC_V_Iset_Gain = atof(optionValues[OptAddr::DAC_V_Iset_Gain]);
  sysOpt->DAC_V_Iset_Offset = atof(optionValues[OptAddr::DAC_V_Iset_Offset]);
  sysOpt->DAC_Voffs_Gain = atof(optionValues[OptAddr::DAC_Voffs_Gain]);
  sysOpt->DAC_Voffs_Gain = atof(optionValues[OptAddr::DAC_Voffs_Gain]);
  sysOpt->DAC_Voffs_Offset = atof(optionValues[OptAddr::DAC_Voffs_Offset]);
  sysOpt->DAC_Vsetn_Gain = atof(optionValues[OptAddr::DAC_Vsetn_Gain]);
  sysOpt->DAC_Vsetn_Offset = atof(optionValues[OptAddr::DAC_Vsetn_Offset]);
  sysOpt->DAC_Vsetp_Gain = atof(optionValues[OptAddr::DAC_Vsetp_Gain]);
  sysOpt->DAC_Vsetp_Offset = atof(optionValues[OptAddr::DAC_Vsetp_Offset]);
  sysOpt->Diode_I_LED = atof(optionValues[OptAddr::Diode_I_LED]);
  sysOpt->Diode_I_LowVF = atof(optionValues[OptAddr::Diode_I_LowVF]);
  sysOpt->Diode_I_Zener = atof(optionValues[OptAddr::Diode_I_Zener]);
  sysOpt->Diode_V_LED = atof(optionValues[OptAddr::Diode_V_LED]);
  sysOpt->Diode_V_LowVF = atof(optionValues[OptAddr::Diode_V_LowVF]);
  sysOpt->Diode_V_Zener = atof(optionValues[OptAddr::Diode_V_Zener]);
  sysOpt->Ohm_I_1R = atof(optionValues[OptAddr::Ohm_I_1R]);
  sysOpt->Ohm_I_1kR = atof(optionValues[OptAddr::Ohm_I_1kR]);
  sysOpt->Ohm_I_10kR = atof(optionValues[OptAddr::Ohm_I_10kR]);
  sysOpt->Ohm_I_100kR = atof(optionValues[OptAddr::Ohm_I_100kR]);
  sysOpt->Ohm_I_1MR = atof(optionValues[OptAddr::Ohm_V_1MR]);
  sysOpt->Ohm_I_10MR = atof(optionValues[OptAddr::Ohm_I_10MR]);
  sysOpt->Ohm_V_1R = atof(optionValues[OptAddr::Ohm_V_1R]);
  sysOpt->Ohm_V_1kR = atof(optionValues[OptAddr::Ohm_V_1kR]);
  sysOpt->Ohm_V_10kR = atof(optionValues[OptAddr::Ohm_V_10kR]);
  sysOpt->Ohm_V_100kR = atof(optionValues[OptAddr::Ohm_V_100kR]);
  sysOpt->Ohm_V_1MR = atof(optionValues[OptAddr::Ohm_V_1MR]);
  sysOpt->Ohm_V_10MR = atof(optionValues[OptAddr::Ohm_V_10MR]);
  sysOpt->SMU_I_Lim = atof(optionValues[OptAddr::SMU_I_Lim]);
  sysOpt->SMU_V_Lim = atof(optionValues[OptAddr::SMU_V_Lim]);

  sysOpt->ADC_Averages = atoi(optionValues[OptAddr::ADC_Averages]);
  sysOpt->ADC_Sample_Rate = atoi(optionValues[OptAddr::ADC_Sample_Rate]);
  sysOpt->ADC_VdiffGain = atoi(optionValues[OptAddr::ADC_VdiffGain]);
  sysOpt->LastScreen = atoi(optionValues[OptAddr::LastScreen]);
  sysOpt->BacklightBrightness = atoi(optionValues[OptAddr::BacklightBrightness]);
  sysOpt->BacklightLowBrght = atoi(optionValues[OptAddr::BacklightLowBrght]);
  sysOpt->BacklightTout = atoi(optionValues[OptAddr::BacklightTout]);
  sysOpt->BattMinChargeLeft = atoi(optionValues[OptAddr::BattMinChargeLeft]);
  sysOpt->DiodeType = atoi(optionValues[OptAddr::DiodeType]);
  sysOpt->LastScreen = atoi(optionValues[OptAddr::LastScreen]);
  sysOpt->Ohm_Range = atoi(optionValues[OptAddr::Ohm_Range]);
  sysOpt->Revision = uint16_t(strtoul(optionValues[OptAddr::Revision], NULL, 16));
  sysOpt->SleepMaxTime= atoi(optionValues[OptAddr::SleepMaxTime]);
  sysOpt->SleepNumCyclesToMeas= atoi(optionValues[OptAddr::SleepNumCyclesToMeas]);
  sysOpt->SleepTimeCycleMs = atoi(optionValues[OptAddr::SleepTimeCycleMs]);
  sysOpt->StandbyTout = atoi(optionValues[OptAddr::StandbyTout]);
  sysOpt->SwitchTurnOffTime = atoi(optionValues[OptAddr::SwitchTurnOffTime]);
  sysOpt->TimeZone = atoi(optionValues[OptAddr::TimeZone]);
  sysOpt->Volt_Range = atoi(optionValues[OptAddr::Volt_Range]);
  
  if (atoi(optionValues[OptAddr::BattProtect]) == 0) 
    sysOpt->BattProtect = false;
  else sysOpt->BattProtect = true;
  if (atoi(optionValues[OptAddr::Diode_MeasMode]) == 0) 
    sysOpt->Diode_MeasMode = false;
  else sysOpt->Diode_MeasMode = true;
  if (atoi(optionValues[OptAddr::Diode_Buzz]) == 0) 
    sysOpt->Diode_Buzz = false;
  else sysOpt->Diode_Buzz = true;
  if (atoi(optionValues[OptAddr::Ohm_Buzz]) == 0) 
    sysOpt->Ohm_Buzz = false;
  else sysOpt->Ohm_Buzz = true;
  if (atoi(optionValues[OptAddr::Ohm_MeasMode]) == 0) 
    sysOpt->Ohm_MeasMode = false;
  else sysOpt->Ohm_MeasMode = true;
  if (atoi(optionValues[OptAddr::Opt_FlipScreen]) == 0) 
    sysOpt->Opt_FlipScreen = false;
  else sysOpt->Opt_FlipScreen = true;
  if (atoi(optionValues[OptAddr::SleepWithCharger]) == 0) 
    sysOpt->SleepWithCharger = false;
  else sysOpt->SleepWithCharger = true;
  if (atoi(optionValues[OptAddr::TimeDaylightOffset]) == 0) 
    sysOpt->TimeDaylightOffset = false;
  else sysOpt->TimeDaylightOffset = true;

  // Copy MAC_Address_Device
  if (optionValues[OptAddr::MAC_Address_Device]) {
      strncpy(sysOpt->MAC_Address_Device,
      optionValues[OptAddr::MAC_Address_Device],
      sizeof(sysOpt->MAC_Address_Device) - 1);
      sysOpt->MAC_Address_Device[sizeof(sysOpt->MAC_Address_Device) - 1] = '\0'; // ensure null-termination
  } 
  else {
      sysOpt->MAC_Address_Device[0] = '\0';
  }

  // Copy MAC_Address_Remote
  if (optionValues[OptAddr::MAC_Address_Remote]) {
      strncpy(sysOpt->MAC_Address_Remote,
      optionValues[OptAddr::MAC_Address_Remote],
      sizeof(sysOpt->MAC_Address_Remote) - 1);
      sysOpt->MAC_Address_Remote[sizeof(sysOpt->MAC_Address_Remote) - 1] = '\0';
  } 
  else {
      sysOpt->MAC_Address_Remote[0] = '\0';
  }

  serialPrintDebug("MAC Address Device: (d)%s (s)%s\n", sysOpt->MAC_Address_Device, optionValues[OptAddr::MAC_Address_Device]);
  serialPrintDebug("MAC Address Remote: (d)%s (s)%s\n", sysOpt->MAC_Address_Remote, optionValues[OptAddr::MAC_Address_Remote]);

  uint64_t storedChecksum = strtoull(optionValues[OptAddr::dataChecksum], NULL, 16);
  serialPrintDebug("Stored checksum %lld\n", storedChecksum);
  sysOpt->dataChecksum = calcOptionDataChecksum(sysOpt);
  if(storedChecksum == 10081961){
    serialPrintDebug("Valid checksum (Init) 0x%lld\n", storedChecksum);
    return;
  }
  if(storedChecksum == sysOpt->dataChecksum){
    serialPrintDebug("Valid checksum (Calc) 0x%lld\n", sysOpt->dataChecksum);
    return;
  }
  *sysOpt = PredefinedOptions;
  serialPrintDebug("Data Options corrupted (C) %X (S) %X. Loading Predefined Options\n", sysOpt->dataChecksum, storedChecksum);
  // Data Options corrupted, add gui message to display.
  return;
}

/**
 * Writes the values from a sysOptions structure to a CSV file.
 *
 * This function converts the values from the sysOptions structure into string format
 * and updates the CSV_Parser object with these values. It handles conversion of 
 * float, integer, and boolean types to their string representations. The updated 
 * CSV data is then saved to a file specified by the optionsPath.
 *
 * @param dataOptions A string containing the CSV data to be parsed.
 * @param sysOpt A pointer to the sysOptions structure containing the options to write.
 */

void WriteOptionsToFile(String dataOptions, sysOptions* sysOpt){
  //dataOptions.replace('\t', ',');  
 
  CSV_Parser cp(dataOptions.c_str(), "ssss");
  size_t rows = cp.getRowsCount();
  if (rows == 0) {
    serialPrintDebug("No rows found in CSV data.\n");
    return;
  }

  serialPrintDebug("Number of lines: %d\n", cp.getRowsCount());
  char **optionNames = (char**)cp["Option_name"];
  char **optionType = (char**)cp["Type"];
  char **optionValues = (char**)cp["Option_value"];
  char **comments = (char**)cp["comment"];
  // Parse the CSV data
  //for(int row = 0; row < 37; row++)
  for(int row = 0; row < cp.getRowsCount(); row++)
    serialPrintDebug("%s, %s, %s\n", optionNames[row], optionValues[row], comments[row]);
  
  // Prepare modifiable copy for option values
  std::vector<String> values(rows);
  for (size_t i = 0; i < rows; i++) {
    values[i] = optionValues[i] ? optionValues[i] : "";
  }

  // ---- Update values safely ----
  char buf[32]; // temp buffer for dtostrf

    // Float options to string conversion 
  dtostrf(sysOpt->ADC_VbattCorr, 6, 5, buf); values[OptAddr::ADC_VbattCorr] = buf;
  dtostrf(sysOpt->ADC_VmeasR1NegGain, 6, 5, buf); values[OptAddr::ADC_VmeasR1NegGain] = buf; 
  dtostrf(sysOpt->ADC_VmeasR1NegOffset, 6, 5, buf); values[OptAddr::ADC_VmeasR1NegOffset] = buf; 
  dtostrf(sysOpt->ADC_VmeasR2NegGain, 6, 5, buf); values[OptAddr::ADC_VmeasR2NegGain] = buf;
  dtostrf(sysOpt->ADC_VmeasR2NegOffset, 6, 5, buf); values[OptAddr::ADC_VmeasR2NegOffset] = buf;
  dtostrf(sysOpt->ADC_VmeasR1Offset, 6, 5,  buf); values[OptAddr::ADC_VmeasR1Offset] = buf;
  dtostrf(sysOpt->ADC_VmeasR1PosGain, 6, 5, buf); values[OptAddr::ADC_VmeasR1PosGain] = buf;
  dtostrf(sysOpt->ADC_VmeasR1PosOffset, 6, 5, buf); values[OptAddr::ADC_VmeasR1PosOffset] = buf;
  dtostrf(sysOpt->ADC_VmeasR2Offset, 6, 5, buf); values[OptAddr::ADC_VmeasR2Offset] = buf;
  dtostrf(sysOpt->ADC_VmeasR2PosGain, 6, 5, buf); values[OptAddr::ADC_VmeasR2PosGain] = buf;
  dtostrf(sysOpt->ADC_VmeasR2PosOffset, 6, 5, buf); values[OptAddr::ADC_VmeasR2PosOffset] = buf;
  dtostrf(sysOpt->ADC_VrefGain, 6, 5, buf); values[OptAddr::ADC_VrefGain] = buf;
  dtostrf(sysOpt->ADC_VrefOffset, 6, 5, buf); values[OptAddr::ADC_VrefOffset] = buf;
  dtostrf(sysOpt->ADC_x2_GainCorr, 6, 5, buf); values[OptAddr::ADC_x2_GainCorr] = buf;
  dtostrf(sysOpt->ADC_x4_GainCorr, 6, 5, buf); values[OptAddr::ADC_x4_GainCorr] = buf;
  dtostrf(sysOpt->ADC_x8_GainCorr, 6, 5, buf); values[OptAddr::ADC_x8_GainCorr] = buf;
  dtostrf(sysOpt->ADC_x16_GainCorr, 6, 5, buf); values[OptAddr::ADC_x16_GainCorr] = buf;
  dtostrf(sysOpt->ADC_x32_GainCorr, 6, 5, buf); values[OptAddr::ADC_x32_GainCorr] = buf;
  dtostrf(sysOpt->ADC_x64_GainCorr, 6, 5, buf); values[OptAddr::ADC_x64_GainCorr] = buf;
  dtostrf(sysOpt->ADC_x128_GainCorr, 6, 5, buf); values[OptAddr::ADC_x128_GainCorr] = buf;
  dtostrf(sysOpt->BattLowThreshold, 6, 5, buf); values[OptAddr::BattLowThreshold] = buf;
  dtostrf(sysOpt->BattOkThreshold, 6, 5, buf); values[OptAddr::BattOkThreshold] = buf;
  dtostrf(sysOpt->DAC_V_Iref_Gain, 6, 5, buf); values[OptAddr::DAC_V_Iref_Gain] = buf;
  dtostrf(sysOpt->DAC_V_Iref_Offset, 6, 5, buf); values[OptAddr::DAC_V_Iref_Offset] = buf;
  dtostrf(sysOpt->DAC_V_Iset_Gain, 6, 5, buf); values[OptAddr::DAC_V_Iset_Gain] = buf;
  dtostrf(sysOpt->DAC_V_Iset_Offset, 6, 5, buf); values[OptAddr::DAC_V_Iset_Offset] = buf;
  dtostrf(sysOpt->DAC_Voffs_Gain, 6, 5, buf); values[OptAddr::DAC_Voffs_Gain] = buf;
  dtostrf(sysOpt->DAC_Voffs_Gain, 6, 5, buf); values[OptAddr::DAC_Voffs_Gain] = buf;
  dtostrf(sysOpt->DAC_Voffs_Offset, 6, 5, buf); values[OptAddr::DAC_Voffs_Offset] = buf;
  dtostrf(sysOpt->DAC_Vsetn_Gain, 6, 5, buf); values[OptAddr::DAC_Vsetn_Gain] = buf;
  dtostrf(sysOpt->DAC_Vsetn_Offset, 6, 5, buf); values[OptAddr::DAC_Vsetn_Offset] = buf;
  dtostrf(sysOpt->DAC_Vsetp_Gain, 6, 5, buf); values[OptAddr::DAC_Vsetp_Gain] = buf;
  dtostrf(sysOpt->DAC_Vsetp_Offset, 6, 5, buf); values[OptAddr::DAC_Vsetp_Offset] = buf;
  dtostrf(sysOpt->Diode_I_LED, 6, 2, buf); values[OptAddr::Diode_I_LED] = buf;
  dtostrf(sysOpt->Diode_I_LowVF, 6, 2, buf); values[OptAddr::Diode_I_LowVF] = buf;
  dtostrf(sysOpt->Diode_I_Zener, 6, 2, buf); values[OptAddr::Diode_I_Zener] = buf;
  dtostrf(sysOpt->Diode_V_LED, 6, 2, buf); values[OptAddr::Diode_V_LED] = buf;
  dtostrf(sysOpt->Diode_V_LowVF, 6, 2, buf); values[OptAddr::Diode_V_LowVF] = buf;
  dtostrf(sysOpt->Diode_V_Zener, 6, 2, buf); values[OptAddr::Diode_V_Zener] = buf;
  dtostrf(sysOpt->Ohm_I_1R, 6, 2, buf); values[OptAddr::Ohm_I_1R] = buf;
  dtostrf(sysOpt->Ohm_I_1kR, 6, 2, buf); values[OptAddr::Ohm_I_1kR] = buf;
  dtostrf(sysOpt->Ohm_I_10kR, 6, 2, buf); values[OptAddr::Ohm_I_10kR] = buf;
  dtostrf(sysOpt->Ohm_I_100kR, 6, 2, buf); values[OptAddr::Ohm_I_100kR] = buf;
  dtostrf(sysOpt->Ohm_I_1MR, 6, 2, buf); values[OptAddr::Ohm_V_1MR] = buf;
  dtostrf(sysOpt->Ohm_I_10MR, 6, 2, buf); values[OptAddr::Ohm_I_10MR] = buf;
  dtostrf(sysOpt->Ohm_V_1R, 6, 2, buf); values[OptAddr::Ohm_V_1R] = buf;
  dtostrf(sysOpt->Ohm_V_1kR, 6, 2, buf); values[OptAddr::Ohm_V_1kR] = buf;
  dtostrf(sysOpt->Ohm_V_10kR, 6, 2, buf); values[OptAddr::Ohm_V_10kR] = buf;
  dtostrf(sysOpt->Ohm_V_100kR, 6, 2, buf); values[OptAddr::Ohm_V_100kR] = buf;
  dtostrf(sysOpt->Ohm_V_1MR, 6, 2, buf); values[OptAddr::Ohm_V_1MR] = buf;
  dtostrf(sysOpt->Ohm_V_10MR, 6, 2, buf); values[OptAddr::Ohm_V_10MR] = buf;
  dtostrf(sysOpt->SMU_I_Lim, 6, 2, buf); values[OptAddr::SMU_I_Lim] = buf;
  dtostrf(sysOpt->SMU_V_Lim, 6, 2, buf); values[OptAddr::SMU_V_Lim] = buf;
  // Integer options to string conversion
  values[OptAddr::ADC_Averages] = String(int(sysOpt->ADC_Averages));
  values[OptAddr::ADC_Sample_Rate] = String(sysOpt->ADC_Sample_Rate);
  values[OptAddr::ADC_VdiffGain] = String(sysOpt->ADC_VdiffGain);
  values[OptAddr::ADC_VnPGain] = String(sysOpt->ADC_VnPGain);
  values[OptAddr::LastScreen] = String(sysOpt->LastScreen);
  values[OptAddr::BacklightBrightness] = String(sysOpt->BacklightBrightness);
  values[OptAddr::BacklightLowBrght] = String(sysOpt->BacklightLowBrght);
  values[OptAddr::BacklightTout] = String(sysOpt->BacklightTout);
  values[OptAddr::BattMinChargeLeft] = String(sysOpt->BattMinChargeLeft);
  values[OptAddr::DiodeType] = String(sysOpt->DiodeType);
  values[OptAddr::LastScreen] = String(sysOpt->LastScreen);
  values[OptAddr::Ohm_Range] = String(sysOpt->Ohm_Range);
  values[OptAddr::Revision] = "0x" + String(sysOpt->Revision, HEX);

  values[OptAddr::SleepMaxTime] = String(sysOpt->SleepMaxTime);
  values[OptAddr::SleepNumCyclesToMeas] = String(sysOpt->SleepNumCyclesToMeas);
  values[OptAddr::SleepTimeCycleMs] = String(sysOpt->SleepTimeCycleMs);
  values[OptAddr::StandbyTout] = String(sysOpt->StandbyTout);
  values[OptAddr::SwitchTurnOffTime] = String(sysOpt->SwitchTurnOffTime);
  values[OptAddr::TimeZone] = String(sysOpt->TimeZone);
  values[OptAddr::Volt_Range] = String(sysOpt->Volt_Range);

  // Binary options to string conversion
  values[OptAddr::BattProtect] = String(int(sysOpt->BattProtect), BIN); 
  values[OptAddr::Diode_MeasMode] = String(int(sysOpt->Diode_MeasMode), BIN); 
  values[OptAddr::Diode_Buzz] = String(int(sysOpt->Diode_Buzz), BIN);
  values[OptAddr::Ohm_Buzz] = String(int(sysOpt->Ohm_Buzz), BIN);
  values[OptAddr::Ohm_MeasMode] = String(int(sysOpt->Ohm_MeasMode), BIN);
  values[OptAddr::Opt_FlipScreen] = String(int(sysOpt->Opt_FlipScreen), BIN);
  values[OptAddr::SleepWithCharger] = String(int(sysOpt->SleepWithCharger), BIN);
  values[OptAddr::TimeDaylightOffset] = String(int(sysOpt->TimeDaylightOffset), BIN);
  
  values[OptAddr::MAC_Address_Device] = String(sysOpt->MAC_Address_Device);
  values[OptAddr::MAC_Address_Remote] = String(sysOpt->MAC_Address_Remote);
  
  sysOpt->dataChecksum = calcOptionDataChecksum(sysOpt);
  values[OptAddr::dataChecksum] = "0x" + String(sysOpt->dataChecksum, HEX);

  // ---- Save updated CSV ----
  File file = LittleFS.open(optionsPath, FILE_WRITE);
  if (!file) {
    serialPrintDebug("Failed to open file for writing\n");
    return;
  }
  // Write header
  file.println("Option_name,Type,Option_value,comment");

  // Write rows
  for (size_t i = 0; i < rows; i++) {
    if (optionNames[i])  
      file.print(optionNames[i]);
    file.print(",");

    if (optionType[i])   
      file.print(optionType[i]);
    file.print(",");

    file.print(values[i]);   // safe modified value
    file.print(",");

    if (comments[i])     
      file.println(comments[i]);
    else                 
      file.println();
  }

  file.close();
  serialPrintDebug("CSV updated and saved successfully.");
}


/**
 * @brief Extracts option values from the provided CSV data.
 * 
 * This function parses CSV data using the CSV_Parser object to extract
 * various fields such as option names, types, values, and comments. 
 * The number of lines in the CSV data is logged, and for each row,
 * the option name, value, and comment are printed. Finally, it returns
 * the array of option values.
 * 
 * @param dataOptions A string containing the CSV data to be parsed.
 * @param cp A CSV_Parser object initialized with the dataOptions.
 * 
 * @return A pointer to an array of strings containing the option values.
 */

char **getCSV_Values(String dataOptions, CSV_Parser cp){
  serialPrintDebug("Number of lines: %d\n", cp.getRowsCount());
  char **optionNames = (char**)cp["Option_name"];
  char **optionType = (char**)cp["Type"];
  char **optionValues = (char**)cp["Option_value"];
  char **comments = (char**)cp["comment"];
  // Parse the CSV data
  //for(int row = 0; row < 37; row++)
  for(int row = 0; row < cp.getRowsCount(); row++)
    serialPrintDebug("%s, %s, %s\n", optionNames[row], optionValues[row], comments[row]);
  return optionValues;
}

uint64_t calcOptionDataChecksum(sysOptions* sysOpt) {
  uint64_t sum = 0;
  unsigned char *p = (unsigned char *) sysOpt;
  uint16_t numBytesInOptions = sizeof(*sysOpt) - sizeof(sysOpt->dataChecksum);
  for (int i=0; i<numBytesInOptions; i++){
    sum += p[i];
    serialPrintDebug("Data Checksum: %lld Data: %d Byte # %d  \n", sum, p[i], i);
  }  
  return sum;
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
