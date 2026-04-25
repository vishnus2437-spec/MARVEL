#include "esp_camera.h"
#include <WiFi.h>
#include <WebSocketsServer.h>
#include "esp_http_server.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

const char* ssid = "Oh Yeah";
const char* password = "12345678";

// Motor pins
#define IN1 12
#define IN2 13
#define IN3 14
#define IN4 15

// PWM channels
#define CH1 0
#define CH2 1

WebSocketsServer webSocket = WebSocketsServer(82);
httpd_handle_t stream_httpd = NULL;

// ================= MOTOR CONTROL =================
void motorControl(String cmd, int speed){
  int pwm = map(speed, 0, 100, 0, 255);

  if(cmd == "F"){
    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  }
  else if(cmd == "B"){
    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  }
  else if(cmd == "L"){
    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  }
  else if(cmd == "R"){
    ledcWrite(CH1, pwm);
    ledcWrite(CH2, pwm);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  }
  else{
    ledcWrite(CH1, 0);
    ledcWrite(CH2, 0);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
  }
}

// ================= WEBSOCKET =================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length){
  if(type == WStype_TEXT){
    String msg = String((char*)payload);

    // Format: F:80
    String cmd = msg.substring(0,1);
    int speed = msg.substring(2).toInt();

    motorControl(cmd, speed);
  }
}

// ================= STREAM =================
esp_err_t stream_handler(httpd_req_t *req){
  camera_fb_t * fb = NULL;
  httpd_resp_set_type(req, "multipart/x-mixed-replace; boundary=frame");

  while(true){
    fb = esp_camera_fb_get();
    if(!fb) continue;

    httpd_resp_send_chunk(req,"--frame\r\n",9);
    httpd_resp_send_chunk(req,"Content-Type: image/jpeg\r\n\r\n",28);
    httpd_resp_send_chunk(req,(const char*)fb->buf,fb->len);
    httpd_resp_send_chunk(req,"\r\n",2);

    esp_camera_fb_return(fb);
    delay(30);
  }
}

// ================= SETUP =================
void setup(){
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  ledcSetup(CH1, 1000, 8);
  ledcSetup(CH2, 1000, 8);
  ledcAttachPin(IN1, CH1);
  ledcAttachPin(IN3, CH2);

  // Camera init
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

  config.frame_size = FRAMESIZE_QVGA;
  config.jpeg_quality = 15;
  config.fb_count = 2;

  esp_camera_init(&config);

  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED){ delay(500); }

  Serial.println(WiFi.localIP());

  // WebSocket
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Stream server
  httpd_config_t config_http = HTTPD_DEFAULT_CONFIG();
  config_http.server_port = 81;

  httpd_start(&stream_httpd, &config_http);

  httpd_uri_t uri = {
    .uri="/stream",
    .method=HTTP_GET,
    .handler=stream_handler,
    .user_ctx=NULL
  };

  httpd_register_uri_handler(stream_httpd, &uri);
}

void loop(){
  webSocket.loop();
}
