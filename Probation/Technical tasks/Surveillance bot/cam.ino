
#include "esp_camera.h"
#include <WiFi.h>
#include "esp_http_server.h"

const char* ssid = "Oh Yeah";
const char* password = "12345678";

#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"

httpd_handle_t server = NULL;

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

void setup(){
  Serial.begin(115200);

  // 🔧 Initialize Camera FIRST
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

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.println("Camera init FAILED");
    return;
  }

  Serial.println("Camera init SUCCESS");

  // 🔧 Now connect WiFi
  Serial.println("Connecting WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("Camera IP: ");
  Serial.println(WiFi.localIP());

  // 🔧 Start server
  httpd_config_t conf = HTTPD_DEFAULT_CONFIG();
  conf.server_port = 81;

  httpd_uri_t uri = {
    .uri="/stream",
    .method=HTTP_GET,
    .handler=stream_handler,
    .user_ctx=NULL
  };

  httpd_start(&server,&conf);
  httpd_register_uri_handler(server,&uri);

  Serial.println("Stream ready at: /stream");
}



void loop(){}
