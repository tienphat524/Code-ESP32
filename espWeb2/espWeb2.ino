#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

uint8_t broadcastAddress[] = {0xB0, 0xB2, 0x1C, 0x97, 0x7B, 0xA4}; //espDevice

typedef struct struct_message {
  int mode;
  bool flagMotor; 
  bool flagPump;
  int humThreshold; 
  int lightThreshold; 
  int soilThreshold;
  int tempThreshold;  
  int bulbStatus; 
  int fanStatus; 
  int motorStatus; 
  int pump1Status; 
  int pump2Status;
  // int chenang;
  int isLight;
  int isHum;
  int isTemp;
  int isSoil;
} struct_message;

struct_message myData;

esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

//address NTP
const char* ntpServer = "pool.ntp.org";
const int timeZone = 7;  // UTC+7 (Việt Nam)
int hourMotor, timeMotor;
int hour1Pump, time1Pump;
int hour2Pump, time2Pump;
int flagMotor = 0, flagPump = 0;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, timeZone * 3600);

/* 1. Define the WiFi credentials */
#define WIFI_SSID         "Free Wifi"
#define WIFI_PASSWORD     "12345679Aa"

/* 2. Define the API Key */
#define API_KEY           "AIzaSyD8DFQJarbBjNkYWdAMYRdAx0sKwUdYJbc"

/* 3. Define the RTDB URL */
#define DATABASE_URL      "https://sensor-7aff3-default-rtdb.firebaseio.com/" 

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL        "hnbtran23@gmail.com"
#define USER_PASSWORD     "Tran12372001"

FirebaseAuth auth;
FirebaseConfig config;

FirebaseData fdbo;

// Define Firebase Data object
FirebaseData data;

String dataPath = "/";

String childPath[19] = {"/mode",
                        "/settingThreshold/hourMotor", 
                        "/settingThreshold/hour1Pump", 
                        "/settingThreshold/humThreshold", 
                        "/settingThreshold/lightThreshold", 
                        "/settingThreshold/soilThreshold",
                        "/settingThreshold/tempThreshold", 
                        "/settingThreshold/timeMotor", 
                        "/settingThreshold/time1Pump", 
                        "/status/bulbStatus", 
                        "/status/fanStatus", 
                        // "/status/motorStatus", 
                        "/status/pump1Status", 
                        "/status/pump2Status",
                        // "/status/chenang",
                        "/settingThreshold/hour2Pump",
                        "/settingThreshold/time2Pump",
                        "/system/isLight",
                        "/system/isHum",
                        "/system/isTemp",
                        "/system/isSoil"};

void dataStreamCallback(MultiPathStreamData stream)
{
  size_t numChild = sizeof(childPath) / sizeof(childPath[0]);
  int toSend = 0;

  for (size_t i = 0; i < numChild; i++)
  {
    if (stream.get(childPath[i]))
    {
      Serial.printf("Test stream path: %s, value: %s", stream.dataPath.c_str(), stream.value.c_str());
      Serial.println();
      switch (i)
      {
        case 0:
        {
          myData.mode = stream.value.toInt();
          break;
        }
        case 1:
        {
          hourMotor = stream.value.toInt();
          break;
        }
        case 2:
        {
          hour1Pump = stream.value.toInt();
          break;
        }
        case 3:
        {
          myData.humThreshold = stream.value.toInt();
          break;
        }
        case 4:
        {
          myData.lightThreshold = stream.value.toInt();
          break;
        }
        case 5:
        {
          myData.soilThreshold = stream.value.toInt();
          break;
        }
        case 6:
        {
          myData.tempThreshold = stream.value.toInt();
          break;
        }
        case 7:
        {
          timeMotor = stream.value.toInt();
          break;
        }
        case 8:
        {
          time1Pump = stream.value.toInt();
          break;
        }
        case 9:
        {
          myData.bulbStatus = stream.value.toInt();
          break;
        }
        case 10:
        {
          myData.fanStatus = stream.value.toInt();
          break;
        }
        // case 11:
        // {
        //   myData.motorStatus = stream.value.toInt();
        //   Serial.print("Motor stream ...........................");
        //   Serial.println(stream.value.toInt());
        //   Serial.print("Motor myData        .................");
        //   Serial.println(myData.motorStatus);
        //   break;
        // }
        case 11:
        {
          myData.pump1Status = stream.value.toInt();
          break;
        }
        case 12:
        {
          myData.pump2Status = stream.value.toInt();
          break;
        }
        // case 14:
        // {
        //   myData.chenang = stream.value.toInt();
        //   Serial.print("Che nang                .......................................");
        //   Serial.println(myData.chenang);
        //   break;
        // }
        case 13:
        {
          hour2Pump = stream.value.toInt();
          break;
        }
        case 14:
        {
          time2Pump = stream.value.toInt();
          break;
        }
        case 15:
        {
          myData.isLight = stream.value.toInt();
          break;
        }
        case 16:
        {
          myData.isHum = stream.value.toInt();
          break;
        }
        case 17:
        {
          myData.isTemp = stream.value.toInt();
          break;
        }
        case 18:
        {
          myData.isSoil = stream.value.toInt();
          break;
        }
      }

      toSend = 1;
    }
  }

  if (toSend) {
    myData.flagMotor = flagMotor;
    myData.flagPump = flagPump;
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
      Serial.print("Motor status ");
      Serial.println(myData.motorStatus);
    }
    else {
      Serial.print("Error sending the data, result = ");
      Serial.println(result);
    }
  }
}

void streamTimeoutCallback(bool timeout)
{
  Serial.println("Time out ...............");
}


void setup() {

  Serial.begin(115200);

  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(WiFi.status());
    delay(500);
  }
  Serial.println();
  Serial.print("WiFi channel: ");
  Serial.println(WiFi.channel());

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  Firebase.reconnectWiFi(true);

  Firebase.begin(&config, &auth);
  Serial.println("Connected to Firebase");

  timeClient.begin();

  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// Init ESP-NOW  ///////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  Serial.println("Success initializing ESP-NOW");

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }

  if (!Firebase.beginMultiPathStream(data, dataPath))
    Serial.printf("Data stream begin error, %s\n\n", data.errorReason().c_str());
  
  delay(1000);
  
  Firebase.setMultiPathStreamCallback(data, dataStreamCallback, streamTimeoutCallback);





  if (!Firebase.beginStream(fdbo, "/status/motorStatus"))
    Serial.printf("Data stream begin error, %s\n\n", fdbo.errorReason().c_str());
  
  delay(1000);
  
  Firebase.setStreamCallback(fdbo, fdboStreamCallback, streamTimeoutCallback);

}

void fdboStreamCallback(StreamData data) {
  Serial.println("Stream Data...");
  Serial.println(data.streamPath());
  Serial.println(data.dataPath());
  Serial.println(data.dataType());
  Serial.println(data.to<int>());

  myData.motorStatus = data.to<int>();

  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
  if (result == ESP_OK) {
    Serial.println("Sent with success");
    Serial.print("Motor status ");
    Serial.println(myData.motorStatus);
  }
  else {
    Serial.print("Error sending the data, result = ");
    Serial.println(result);
  }
}

//thêm 1 biến làm flag đã gửi message qua device hay chưa
bool flagPumpSent = 0;
bool flagMotorSent = 0;

void loop() {
  timeClient.update();

  if (!flagPumpSent) {
    if ((timeClient.getHours() == hour1Pump 
                  || timeClient.getHours() == hour2Pump) 
                  && timeClient.getMinutes() == 40) {
      flagPump = 1;
      myData.flagPump = flagPump;
      flagPumpSent = 1;

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
      if (result == ESP_OK) {
        Serial.println("Sent with success flag pump");
      }
      else {
        Serial.print("Error sending the data, result = ");
        Serial.println(result);
      }

      delay(1000);
    }
  } else {
    if ((timeClient.getHours() == hour1Pump + time1Pump/60 && timeClient.getMinutes() == time1Pump%60 +40)
                  || (timeClient.getHours() == hour2Pump + time2Pump/60 && timeClient.getMinutes() == time2Pump%60 + 40)) {
      flagPump = 0;
      myData.flagPump = flagPump;
      flagPumpSent = 0;

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
      if (result == ESP_OK) {
        Serial.println("Sent with success flag pump");
      }
      else {
        Serial.print("Error sending the data, result = ");
        Serial.println(result);
      }

      delay(1000);
    }
  }

  if (!flagMotorSent) {
    if (timeClient.getHours() == hourMotor 
                  && timeClient.getMinutes() == 40) {
      flagMotor = 1;
      myData.flagMotor = flagMotor;
      flagMotorSent = 1;
      
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
      if (result == ESP_OK) {
        Serial.println("Sent with success flag motor");
      }
      else {
        Serial.print("Error sending the data, result = ");
        Serial.println(result);
      }

      delay(1000);
    }
  } else {
    if (timeClient.getHours() == hourMotor + timeMotor/60 
                  && timeClient.getMinutes() == timeMotor%60 +40) {
      flagMotor = 0;
      myData.flagMotor = flagMotor;
      flagMotorSent = 0;

      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    
      if (result == ESP_OK) {
        Serial.println("Sent with success flag motor");
      }
      else {
        Serial.print("Error sending the data, result = ");
        Serial.println(result);
      }

      delay(1000);
    }
  }
}