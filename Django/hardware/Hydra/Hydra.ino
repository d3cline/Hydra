// NodeMCU uses software serial bitbanged for UART control of probes.
#include <SoftwareSerial.h>
SoftwareSerial swSer(14, 12, false, 256);

#include <ESP8266WiFi.h> // Wifi stack
#include <WiFiClient.h> // POST
#include <ESP8266WebServer.h> // GET

ESP8266WebServer server(80);

String TOKEN = "6e61b6dae8f8eccf52d402359475763887efda8f";
String URI = "/api/v1/post/";
String HOST = "192.168.1.9";

// webserver strings.
String PH = "null";
String ORP = "null";
String EC = "null";
String RTD = "null";
String DO = "null";

/* Mode 0 = normal read
   Mode 1 = EC_cal
   Mode 2 = DO_cal
   Mode 3 = pH_cal
   Mode 4 = ORP_cal
*/
int op_mode = 0;
int uart_mode;

// UART switch functions
void uart_one() { //RTD
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  uart_mode = 1;
}
void uart_two() { //EC
  digitalWrite(D1, LOW);
  digitalWrite(D2, LOW);
  digitalWrite(D3, HIGH);
  uart_mode = 2;
}
void uart_three() { //DO
  digitalWrite(D1, LOW);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, LOW);
  uart_mode = 3;
}
void uart_four() { //PH
  digitalWrite(D1, LOW);
  digitalWrite(D2, HIGH);
  digitalWrite(D3, HIGH);
  uart_mode = 4;
}
void uart_five() { //ORP
  digitalWrite(D1, HIGH);
  digitalWrite(D2, LOW);
  digitalWrite(D3, LOW);
  uart_mode = 5;
}

void reset_read() {
  // Test/init all UARTs

  uart_one();
  // Set temp probe to single reading mode.
  swSer.write("C,0\r");
  delay(250);

  uart_two();
  // Set PH probe to single reading mode.
  swSer.write("C,0\r");
  delay(250);

  uart_three();
  swSer.write("C,0\r");
  delay(250);

  uart_four();
  swSer.write("C,0\r");
  delay(250);

  uart_five();
  swSer.write("C,0\r");
  delay(250);

} //


void cors(){
      server.sendHeader("Access-Control-Allow-Origin", "*");
  }

void EC_cal_JSON() {
  reset_read();
  op_mode = 1;
  uart_two();
  // Set EC probe to single reading mode.
  swSer.write("C,1\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void EC_cal_dry() {
  op_mode = 1;
  uart_two();
  swSer.write("Cal,dry\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  }
  
void EC_cal_high() {
  op_mode = 1;
  uart_two();
  swSer.write("Cal,high,80000\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  }
  
void EC_cal_low() {
  op_mode = 1;
  uart_two();
  swSer.write("Cal,low,12880\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  }

void DO_cal_JSON() {
  reset_read();
  op_mode = 2;
  uart_three();
  uart_three();
  uart_three();
  swSer.write("C,1\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void DO_cal_zero() {
  op_mode = 2;
  uart_three();
  swSer.write("Cal,0\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void DO_cal_atmos() {
  op_mode = 2;
  uart_three();
  swSer.write("C,1\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void PH_cal_JSON() {
  reset_read();
  op_mode = 3;
  uart_four();
  swSer.write("C,1\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void PH_cal_low() {
  op_mode = 3;
  uart_four();
  swSer.write("Cal,low,4.00\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void PH_cal_mid() {
  op_mode = 3;
  uart_four();
  swSer.write("Cal,mid,7.00\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void PH_cal_high() {
  op_mode = 3;
  uart_four();
  swSer.write("Cal,high,10.00\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void ORP_cal_JSON() {
  reset_read();
  op_mode = 4;
  uart_five();
  swSer.write("C,1\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void ORP_cal_n() {
  op_mode = 4;
  uart_five();
  swSer.write("Cal,225\r");
  delay(500);
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void RESET() {
  op_mode = 0;
  reset_read();
  cors();
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handleJSON() {
  String IP = WiFi.localIP().toString();
  long rssi = WiFi.RSSI();
  cors();
  String send_json = "{\"IP\":\"" + IP + "\",\"rssi\":\"" + String(rssi) + "\",\"mode\":\"" + String(op_mode) + "\",\"data\":{\"rtd\":\"" + RTD + "\",\"ec\":\"" + EC + "\",\"do\":\"" + DO + "\",\"ph\":\"" + PH + "\",\"orp\":\"" + ORP + "\"}}";
  server.send ( 200,  "application/javascript", send_json );
}

void handleNotFound() {
  // try to find the file in the flash
  String message = "File Not Found\n\n";
  message += "URI..........: ";
  message += server.uri();
  message += "\nMethod.....: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments..: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  cors();
  server.send(404, "text/plain", message);
}

void setup() {
  // Wait for everything to power on.
  delay(2500);

  //start serial for terminal and swSerial for probe UART
  Serial.begin(115200);
  swSer.begin(9600);

  Serial.println("HYDRA RISES");

  // Set UART switches.
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);

  //
  reset_read();
  delay(50);

  //connecting to a WiFi network
  Serial.print("Connecting to ");
  Serial.println("Network");

  WiFi.mode(WIFI_STA);
  WiFi.begin("HYDRA", "Hydra123");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    if (WiFi.status() == WL_DISCONNECTED) {
      Serial.println("Connecting");
    }
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial.println("Check SSID");
    }
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Check password");
    }
    if (WiFi.status() == WL_CONNECTION_LOST) {
      Serial.println("Connection Lost");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.print("Connected  ");
      Serial.println(WiFi.localIP().toString());
    }

  }
  delay(50);
  
  // init webserver
  server.on("/", handleJSON);

  // Reset to read mode
  server.on("/reset", RESET);

  // Start EC cal
  server.on("/ec_cal", EC_cal_JSON);

  //  Send EC Dry
  server.on("/ec_cal_dry", EC_cal_dry);

  //  Send EC Low (12880)
  server.on("/ec_cal_low", EC_cal_low);

  //  Send EC High (80000)
  server.on("/ec_cal_high", EC_cal_high);

  // Start DO cal
  server.on("/do_cal", DO_cal_JSON);

  // Send DO Cal, 0
  server.on("/do_cal_zero", DO_cal_zero);

  // Send DO Cal, Atmospheric
  server.on("/do_cal_atmos", DO_cal_atmos);

  // Start PH cal
  server.on("/ph_cal", PH_cal_JSON);

  // Send PH Mid
  server.on("/ph_cal_mid", PH_cal_mid);

  // Send PH Low
  server.on("/ph_cal_low", PH_cal_low);

  // Send PH High
  server.on("/ph_cal_high", PH_cal_high);

  // Start ORP cal
  server.on("/orp_cal", ORP_cal_JSON);

  // Cal ORP n
  server.on("/orp_cal_n", ORP_cal_n);

  // Not found
  server.onNotFound(handleNotFound);
  
  server.begin();

  // turn on led, boot finished
  pinMode(D0, OUTPUT);
  digitalWrite(D0, LOW);
  delay(2500);

} // send setup


String sensorstring = "";
boolean sensor_stringcomplete = false;

void reader() {
  while (swSer.available() > 0) {
    char inchar = (char)swSer.read();
    sensorstring += inchar;
    if (inchar == '\r') {
      sensor_stringcomplete = true;
    }

    if (sensor_stringcomplete) { //if a string from the Atlas Scientific product has been received in its entirety
        //Serial.println(sensorstring);
        if (uart_mode == 1) {
          if (sensorstring == "*OK\r") {}
          else {
            sensorstring.trim();
            RTD = sensorstring;
          }
        }
        else if (uart_mode == 2) {
          if (sensorstring == "*OK\r") {}
          else {
            sensorstring.trim();
            EC = sensorstring;
          }
        }
        else if (uart_mode == 3) {
          if (sensorstring == "*OK\r") {}
          else {
            sensorstring.trim();
            DO = sensorstring;
          }
        }
        else if (uart_mode == 4) {
          if (sensorstring == "*OK\r") {}
          else {
            sensorstring.trim();
            PH = sensorstring;
          }
        }
        else if (uart_mode == 5) {
          if (sensorstring == "*OK\r") {}
          else {
            sensorstring.trim();
            ORP = sensorstring;
          }
        }
        //end if uart

      sensorstring = "";  //clear the string:
      sensor_stringcomplete = false;  //reset the flag used to tell if we have received a completed string from the Atlas Scientific product
    } //end if sensor_stringcomplete

  }// end while swSerail
}// end reader

long postPreviousMillis = 0;
long postInterval = 300000;           // interval at which to send POST 5 minutes)
long checkPreviousMillis = 0;

unsigned long currentMillis; // NOW

void loop() {
currentMillis = millis();

// check if cal mode is stale, if so reset, if not calc timer. 
if(op_mode > 0){
  if(currentMillis - checkPreviousMillis > 900000 ) {
    checkPreviousMillis = currentMillis;
     
      reset_read();
      op_mode = 0;
     }

}
else{
  checkPreviousMillis = currentMillis;
  }


  if (op_mode == 0) {

    uart_one();
    swSer.write("R\r");
    delay(1000);
    reader();
    delay(500);
    server.handleClient();
  }
 if (op_mode == 0) {
    uart_two();
    swSer.write("R\r");
    delay(1000);
    reader();
    delay(500);
    server.handleClient();
 }
    
 if (op_mode == 0) {
    uart_three();
    swSer.write("R\r");
    delay(1000);
    reader();
    delay(500);
    server.handleClient();
 }
    
 if (op_mode == 0) {
    uart_four();
    swSer.write("R\r");;
    delay(1000);
    reader();
    delay(500);
    server.handleClient();
 }
    
 if (op_mode == 0) {
    uart_five();
    swSer.write("R\r");
    delay(1000);
    reader();
    delay(500);
    server.handleClient();
 }
// end read loop
   if (op_mode == 0) { 
    
  if(currentMillis - postPreviousMillis > postInterval) {

  String IP = WiFi.localIP().toString();
  long rssi = WiFi.RSSI();
    
    postPreviousMillis = currentMillis;
    String PostData = "{\"rtd\":" + RTD + ", \"ec\":" + EC + ", \"do\":" + DO + ", \"ph\":" + PH + ", \"orp\":" + ORP + ", \"ip\":\"" + IP + "\", \"rssi\":" + rssi + "}";
    Serial.println (PostData);

    WiFiClient client;
    const int httpPort = 80;
    char host[50];
    HOST.toCharArray(host, 50);
  
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  client.println("POST "+URI+" HTTP/1.1"); // API URI
  client.println("Host: "+HOST); // HOST
  client.println("Cache-Control: no-cache"); // all data is new data
  client.println("Authorization: Token "+TOKEN); //
  client.println("Content-Type: application/json"); // sets JSON type
  client.print("Content-Length: "); //Required
  client.println(PostData.length());
  client.println();
  client.println(PostData);

  while (client.available() == 0) {

    if (millis() - currentMillis > 10000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
    
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
     Serial.print(line);
  }
  // Serial.println();
  // Serial.println("closing connection");
  
  } // end current mils
  } // end opmode
  else {
    reader();
  }

  // Listen for web requests
  server.handleClient();

} // end loop
