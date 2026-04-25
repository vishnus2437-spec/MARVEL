
#include <WiFi.h>

const char* ssid = "moto";
const char* password = "12345678";

// 🔴 PUT YOUR CAMERA IP HERE
String camIP = "192.168.1.100";  

#define IN1 13
#define IN2 12
#define IN3 14
#define IN4 27

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected!");
  Serial.println(WiFi.localIP());

  server.begin();
}

// MOTOR FUNCTIONS
void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void back() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void left() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void right() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopMotor() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void loop() {
  WiFiClient client = server.available();

  if (client) {
    String req = client.readStringUntil('\r');
    client.flush();

    if (req.indexOf("/F") != -1) forward();
    else if (req.indexOf("/B") != -1) back();
    else if (req.indexOf("/L") != -1) left();
    else if (req.indexOf("/R") != -1) right();
    else if (req.indexOf("/S") != -1) stopMotor();

    // 🌐 WEBPAGE
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html");
    client.println();

    client.println("<html><head>");
    client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
    client.println("</head><body style='text-align:center;'>");

    client.println("<h2>ESP32 Robot + Camera</h2>");

    // 🎥 CAMERA STREAM
    client.print("<img src='http://");
    client.print(camIP);
    client.println(":81/stream' width='320'><br><br>");

    // 🎮 CONTROLS
    client.println("<a href='/F'><button style='width:80px;height:40px;'>F</button></a><br><br>");
    client.println("<a href='/L'><button>Left</button></a>");
    client.println("<a href='/S'><button>Stop</button></a>");
    client.println("<a href='/R'><button>Right</button></a><br><br>");
    client.println("<a href='/B'><button>Back</button></a>");

    client.println("</body></html>");
    client.stop();
  }
}
