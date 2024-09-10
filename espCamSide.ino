#include "Arduino.h"
#include "WiFi.h"
#include "esp_camera.h"
#include "soc/soc.h"           // Disable brownout problems
#include "soc/rtc_cntl_reg.h"  // Disable brownout problems
#include "driver/rtc_io.h"
#include <LittleFS.h>
#include <FS.h>
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include <addons/TokenHelper.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>


#define espSerial Serial2

#define txPin 14  // TX pin (connected to RX of other ESP)
#define rxPin 15   // RX pin (صnot used for sending)

#define FLASH_GPIO_NUM 4


String urlencode(String str);
const char* ssid = "mahmoud";
const char* password = "mahmoud.32";
String HOST_NAME = "http://192.168.176.200"; // change to your PC's IP address
String PATH_NAME   = "/gradiation_project/issaac/isaac.php";

#define LED 2

AsyncWebServer server(80);


void unNamedFunc(uint8_t *data, size_t len){
  WebSerial.println("Received Data...");
  String d = "";
  for(int i=0; i < len; i++){
    d += char(data[i]);
  }
  WebSerial.println(d);
  if (d == "ON"){
    digitalWrite(FLASH_GPIO_NUM, HIGH);
  }
  if (d=="OFF"){
    digitalWrite(FLASH_GPIO_NUM, LOW);
  }
}
// Insert Firebase project API Key
#define API_KEY "AIzaSyBhhhDT4vuZw157bstNcKRAz4vMdYErzE4"

// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "main136202@gmail.com"
#define USER_PASSWORD "@final2024"

// Insert Firebase storage bucket ID e.g bucket-name.appspot.com
#define STORAGE_BUCKET_ID "esp-firebase-5fc20.appspot.com"
// For example:
//#define STORAGE_BUCKET_ID "esp-iot-app.appspot.com"

// Photo File Name to save in LittleFS
#define FILE_PHOTO_PATH "/photo"
#define BUCKET_PHOTO "/data/photo"

// OV2640 camera module pins (CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

boolean takeNewPhoto = true;
 int i =0;
 String filePath;
 String filePhoto;
 String firebase_link;
 int location=1;
//Define Firebase Data objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig configF;
char planetStats='1';

void fcsUploadCallback(FCS_UploadStatusInfo info);

bool taskCompleted = false;

void jsonToIno(String jsonString){

    // Parse JSON data
    StaticJsonDocument<200> doc; // Specify the size of the JSON document
    DeserializationError error = deserializeJson(doc, jsonString);

    // Check for parsing errors
    if (error) {
      Serial.print("Parsing failed: ");
      Serial.println(error.c_str());
      return;
    }

    // Access parsed data
    String temp = doc["planetStats"];
    planetStats = temp[0];

    // Print parsed data
    Serial.print("Planet Stats: ");
    Serial.println(planetStats);

    WebSerial.print("Planet Stats: ");
    WebSerial.println(planetStats);
}

// Capture Photo and Save it to LittleFS
void capturePhotoSaveLittleFS( void ) {
  // Dispose first pictures because of bad quality
  camera_fb_t* fb = NULL;
  // Skip first 3 frames (increase/decrease number as needed).
  for (int i = 0; i < 4; i++) {
    fb = esp_camera_fb_get();
    esp_camera_fb_return(fb);
    fb = NULL;
  }
    
  // Take a new photo
  fb = NULL;  
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    delay(1000);
    ESP.restart();
  }  

  // Photo file name
  Serial.printf("Picture file name: %s\n", filePath);
  File file = LittleFS.open(filePath, FILE_WRITE);

  // Insert the data in the photo file
  if (!file) {
    Serial.println("Failed to open file in writing mode");
  }
  else {
    file.write(fb->buf, fb->len); // payload (image), payload length
    Serial.print("The picture has been saved in ");
    Serial.print(filePath);
    Serial.print(" - Size: ");
    Serial.print(fb->len);
    Serial.println(" bytes");
  }
  // Close the file
  file.close();
  esp_camera_fb_return(fb);
}

void initWiFi(){
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
    Serial.println("WiFi Connected");
      Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
 
}

void initLittleFS(){
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS");
      WebSerial.println("An Error has occurred while mounting LittleFS!");

    ESP.restart();
  }
  else {
    delay(500);
    Serial.println("LittleFS mounted successfully");
    WebSerial.println("LittleFS mounted successfully");
  }
}

void initCamera(){
 // OV2640 camera module
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
/*
   if (psramFound()) {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
*/
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  } 

 sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, -2);     // -2 to 2
  s->set_contrast(s, 2);       // -2 to 2
  s->set_saturation(s, -2);     // -2 to 2
  //s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  //s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 1);           // 0 = disable , 1 = enable
  //s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 1);      // 0 = disable , 1 = enable
  //s->set_agc_gain(s, 0);       // 0 to 30
  //s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 1);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 1);           // 0 = disable , 1 = enable
  //s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  //s->set_vflip(s, 0);          // 0 = disable , 1 = enable
  s->set_dcw(s, 1);            // 0 = disable , 1 = enable
  //s->set_colorbar(s, 0);       // 0 = disable , 1 = enable

}

void setup() {
    espSerial.begin(115200,SERIAL_8N1,rxPin,txPin); // Initialize serial communication (replace with baud rate)

  // Serial port for debugging purposes
  Serial.begin(115200);
  initWiFi();
   WebSerial.begin(&server);
  // WebSerial.msgCallback(recvMsg);
  server.begin();
  initLittleFS();
  // Turn-off the 'brownout detector'
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  initCamera();

  //Firebase
  // Assign the api key
  configF.api_key = API_KEY;
  //Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  //Assign the callback function for the long running token generation task
  configF.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&configF, &auth);
  Firebase.reconnectWiFi(true);
  if(espSerial.availableForWrite()){  
  espSerial.write('9');
      Serial.println("ready to go");
      WebSerial.println("Ready");

  }
  else{
      Serial.println("cant write");

  }
    pinMode(FLASH_GPIO_NUM, OUTPUT);


  delay(500);


}

 void loop() {

if (espSerial.available()) {
    char data = espSerial.read();
    Serial.print("Received data: ");
    Serial.println(data);
    

    if (data == '0') {
      takePic();
      delay(500);
      uplodePic();
    } else {
      Serial.println("no");
    }

    // Send 'y' unconditionally after processing (optional)
    espSerial.write('8');
    Serial.println("Sent data: 8");
    delay(50);
    espSerial.write(planetStats);
    Serial.println("Sent planetStats");
    WebSerial.println("Sent planetStats");
    
    
    delay(300);
  } else {
    Serial.println("no data");
    WebSerial.println("No Data");
  }



}

void deleteAllfs(){
  File root = LittleFS.open("/");
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }

  while (File file = root.openNextFile()) {
    if (file.isDirectory()) {
      continue;
    }
     String fn=file.name();
     String allfn="/"+fn;
    file.close();
   Serial.printf("Removing file:%s\n ",fn);
    if (LittleFS.exists(allfn)) {
    if (LittleFS.remove(allfn)) {
      Serial.println("File removed successfully.");
    } else {
      Serial.println("Error removing file!");
      WebSerial.println("Error removing file!");
      
    }
  } else {
    
    Serial.println("File does not exist.");
    WebSerial.println("File does not exist.");

  }
  }
  Serial.println("All files removed.");
  WebSerial.println("All files removed.");

  root.close();
}

void takePic() {
  digitalWrite(FLASH_GPIO_NUM, HIGH);

   deleteAllfs();
   i++;
   filePath= FILE_PHOTO_PATH + String(i)+"_"+ String(millis()) + ".jpg";
   filePhoto = BUCKET_PHOTO + String(i) +"_"+ String(millis()) + ".jpg";
  if (takeNewPhoto) {  
    capturePhotoSaveLittleFS();
    takeNewPhoto = false;
  
  delay(1);
  if (Firebase.ready() && !taskCompleted){
    taskCompleted = true;
    Serial.print("Uploading picture... ");
    WebSerial.print("Uploading picture... ");

        digitalWrite(FLASH_GPIO_NUM, LOW);

    bool stat =  false;
    while(!stat){
     stat = Firebase.Storage.upload(&fbdo, STORAGE_BUCKET_ID ,filePath  , mem_storage_type_flash ,  filePhoto , "image/jpeg" ,fcsUploadCallback);
    }
          firebase_link=fbdo.downloadURL();
          Serial.printf("\nDownload URL: %s\n", firebase_link.c_str());
          WebSerial.printf("\nDownload URL: %s\n", firebase_link.c_str());

          takeNewPhoto = true;
          LittleFS.remove(filePath);
  }
    taskCompleted = false;
  }
}


String urlencode(String str) {
  const char *msg = str.c_str();
  const char *hex = "0123456789ABCDEF";
  String encodedMsg = "";
  while (*msg != '\0') {
    if (('a' <= *msg && *msg <= 'z') || ('A' <= *msg && *msg <= 'Z') || ('0' <= *msg && *msg <= '9') || *msg == '-' || *msg == '_' || *msg == '.' || *msg == '~') {
      encodedMsg += *msg;
    } else {
      encodedMsg += '%';
      encodedMsg += hex[(unsigned char)*msg >> 4];
      encodedMsg += hex[*msg & 0xf];
    }
    msg++;
  }
  return encodedMsg;
}


void uplodePic() {


  
  // charLocation=char(location++);
  String queryString= String("firebase_link=") + String(urlencode(firebase_link))+ String("&location=") + String(location++);
  Serial.printf("\nDownload URL: %s\n", urlencode(firebase_link));
  WebSerial.printf("\nDownload URL: %s\n", urlencode(firebase_link));
   
   HTTPClient http;

  //http.begin(HOST_NAME + PATH_NAME +"?" + queryString); //HTTP
  http.begin(HOST_NAME + PATH_NAME);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
 int httpCode = http.POST(queryString);

  // httpCode will be negative on error
  if(httpCode > 0) {
    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();  
            Serial.println("payload : ");
      Serial.println(payload);
          Serial.println("data after conv : ");


             WebSerial.println("payload : ");
      WebSerial.println(payload);
          WebSerial.println("data after conv : ");

      // jsonToIno(payload);
    } else {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] POST... code: %d\n", httpCode);
       WebSerial.printf("[HTTP] POST... code: %d\n", httpCode);

    }
  } else {
    Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
    WebSerial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());

  }
  http.end();

}

// The Firebase Storage upload callback function
void fcsUploadCallback(FCS_UploadStatusInfo info){
    if (info.status == firebase_fcs_upload_status_init){
        Serial.printf("Uploading file %s (%d) to %s\n", info.localFileName.c_str(), info.fileSize, info.remoteFileName.c_str());
    }
    else if (info.status == firebase_fcs_upload_status_upload)
    {
        Serial.printf("Uploaded %d%s, Elapsed time %d ms\n", (int)info.progress, "%", info.elapsedTime);
    }
    else if (info.status == firebase_fcs_upload_status_complete)
    {
        Serial.println("Upload completed\n");
        FileMetaInfo meta = fbdo.metaData();
        Serial.printf("Name: %s\n", meta.name.c_str());
        Serial.printf("Bucket: %s\n", meta.bucket.c_str());
        Serial.printf("contentType: %s\n", meta.contentType.c_str());
        Serial.printf("Size: %d\n", meta.size);
        Serial.printf("Generation: %lu\n", meta.generation);
        Serial.printf("Metageneration: %lu\n", meta.metageneration);
        Serial.printf("ETag: %s\n", meta.etag.c_str());
        Serial.printf("CRC32: %s\n", meta.crc32.c_str());
        Serial.printf("Tokens: %s\n", meta.downloadTokens.c_str());
        Serial.printf("Download URL: %s\n\n", fbdo.downloadURL().c_str());
    }
    else if (info.status == firebase_fcs_upload_status_error){
        Serial.printf("Upload failed, %s\n", info.errorMsg.c_str());
    }
}
