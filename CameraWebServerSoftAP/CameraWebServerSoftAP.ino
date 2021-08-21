#include "esp_camera.h"
#include <WiFi.h>
#include <stdio.h>
#include <stdlib.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

//8bit
//#define MAX_SERVO_WIDTH 28
//#define MID_SERVO_WIDTH 20
//#define MIN_SERVO_WIDTH 12

//10bit
#define MAX_SERVO_WIDTH 85
#define MID_SERVO_WIDTH 70
#define MIN_SERVO_WIDTH 55

#define MAX_ACCEL_WIDTH 1024
#define MIN_ACCEL_WIDTH 0

#define N 3

#include "camera_pins.h"

//ESP32 SoftAP Configration
const char ssid[] = "ESP32-NET";
const char pass[] = "mypass12345";
const IPAddress ip(192,168,0,12);
const IPAddress server_ip(192, 168, 0, 13);
const IPAddress subnet(255,255,255,0);
const int port1 = 11411; 
const int port2 = 11412; 

WiFiClient client1;
WiFiClient client2;

void startCameraServer();

void setup() {

  // Serial setup ///////////////
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Camera config /////////////
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
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  // WiFi setup ////////////////////
/*
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
*/
  //SoftAP
  WiFi.softAP(ssid,pass);
  delay(100);
  WiFi.softAPConfig(ip,ip,subnet);
  IPAddress myIP = WiFi.softAPIP();

  /*
  WiFi.begin(ssid, pass);
  while( WiFi.status() != WL_CONNECTED) {
    delay(500);  
    Serial.print(".");
  }  
  */

  // Connect port
  Serial.print("Local port: ");
  Serial.println(port1);
  client1.connect(server_ip, port1);
  client2.connect(server_ip, port2);

  
  Serial.println("ESP32 SoftAP Mode start.");
  Serial.print("SSID:");
  Serial.println(ssid);

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  //Serial.print(WiFi.localIP());
  Serial.print(myIP);
  Serial.println("' to connect");

  // PWM /////////////////////////
  // Assign pin
  //const int ledPin     = A10; 
  //const int drivePin   = A10; 
  const int servoPin   = A12; 
  const int forwardPin = A16;
  const int backPin    = A13;
    
  //pwm setup ledcSetup(channel, frequency, resolution);  resolution->(8bit = 256division, 10bit = 1024division）
  //ledcSetup(15,50,10); 
  //ledcSetup(15,12800,8); 
  ledcSetup(14,50,10); 
  //ledcSetup(14,50,8);
  ledcSetup(13,50,10); 
  ledcSetup(12,50,10); 
  
  //attach pin to channel(pin, channel)
  //ledcAttachPin(ledPin,15);
  //ledcAttachPin(drivePin,15);
  ledcAttachPin(servoPin,14);
  ledcAttachPin(forwardPin,13);
  ledcAttachPin(backPin,12);
}

void loop() {
  int val_a, val_s;

  // put your main code here, to run repeatedly:
  //ledcWrite(15,255);
  //ledcWrite(14,MAX_SERVO_WIDTH);
  //delay(1000);
  //ledcWrite(15,128);
  //ledcWrite(14,MID_SERVO_WIDTH);
  //delay(1000);
  //ledcWrite(15,8);
  //ledcWrite(14,MIN_SERVO_WIDTH);
  //delay(1000);
  
  //Serial.printf("pass0\n");
  if(client1.available()) {
    String line = client1.readStringUntil('\n');
    val_s = atoi(line.c_str());
    //Serial.println(line);
    Serial.printf("val_s=%d ",val_s);
    if      ( val_s > MAX_SERVO_WIDTH) val_s = MAX_SERVO_WIDTH;
    else if ( val_s < MIN_SERVO_WIDTH) val_s = MIN_SERVO_WIDTH;
    Serial.printf("servo_val=%d",val_s);
    ledcWrite(14,val_s);  
  }
  if(client2.available()) {
    Serial.printf("pass2\n");
    String line = client2.readStringUntil('\n');
    val_a = atoi(line.c_str());
    //Serial.printf("pass3\n");
    Serial.printf("val_a=%d ",val_a);
    if(val_a >= 0){
      if (val_a > MAX_ACCEL_WIDTH)val_a = MAX_ACCEL_WIDTH;
      ledcWrite(13,val_a);
      ledcWrite(12,0);
    }
    else{
      if (val_a < -MAX_ACCEL_WIDTH)val_a = -MAX_ACCEL_WIDTH;
      ledcWrite(13,0);
      ledcWrite(12,-val_a);
    }
    Serial.printf("accel_val=%d\n",val_a);
  }

  
}
