#include <WiFi.h>
#include <Wire.h>
#include "DHT.h"
#include <Adafruit_BMP085.h>
#include <MQ135.h>

const char* ssid = "RT-WiFi-ED7E";
const char* password = "2839000860";

WiFiServer server(80);
DHT dht(4, DHT11); 
Adafruit_BMP085 bmp;
MQ135 gasSensor = MQ135(32);

String header;
String output25State = "off";
const int output25 = 25;
String output26State = "off";
const int output26 = 26;

float temperature;
float humidity;
float pressure;
float co2;
String smile;

void setup() {
  Serial.begin(115200);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");  
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
  server.begin();
  Serial.println("HTTP server started");

  pinMode(output25, OUTPUT);
  digitalWrite(output25, 0);
  pinMode(output26, OUTPUT);
  digitalWrite(output26, 0);
  
  bmp.begin(); 
//  pinMode(4, INPUT);
  dht.begin();
}

void loop()
{
  temperature = (bmp.readTemperature() + dht.readTemperature()) / 2;
  humidity = dht.readHumidity(); 
  pressure = bmp.readPressure() / 133; 
  co2 = gasSensor.getPPM();
  smile = SmileFace(temperature, humidity, 19, 27, 20, 85);
  
  WiFiClient client = server.available();
  if (client) 
  {                       
    Serial.println("New Client."); 
    String currentLine = "";  
    while (client.connected()) 
    { 
      if (client.available()) 
      {
        char c = client.read();     
        Serial.write(c);            
        header += c;
        if (c == '\n') {            
          if (currentLine.length() == 0) 
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            if (header.indexOf("GET /25/on") >= 0) 
            {
              Serial.println("GPIO 25 on");  //  "GPIO25 включен"
              output25State = "on";
              digitalWrite(output25, HIGH);
            } 
            else if (header.indexOf("GET /25/off") >= 0) 
            {
              Serial.println("GPIO 25 off");  //  "GPIO25 выключен"
              output25State = "off";
              digitalWrite(output25, LOW);
            } 
            else if (header.indexOf("GET /26/on") >= 0) 
            {
              Serial.println("GPIO 26 on");  //  "GPIO26 включен"
              output26State = "on";
              digitalWrite(output26, HIGH);
            } 
            else if (header.indexOf("GET /26/off") >= 0) 
            {
              Serial.println("GPIO 26 off");  //  "GPIO26 выключен"
              output26State = "off";
              digitalWrite(output26, LOW);
            } 
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"Content-type\" content=\"text/html; charset=utf-8\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println("body {background-color:  #333333;} {margin-top: 50px;}");
            client.println("h1 {color: #FFFFFF; margin: 50px auto 30px;}");
            client.println("p {font-size: 24px; color: #FFFFFF; margin-bottom: 10px;}");
            client.println(".button { background-color: #669999; border: none; color: white; padding: 15px 30px; border-radius: 10px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #666666;}</style></head>");

            client.println("<title>ESP32 Умный дом</title>");
            client.println("<body><h1>УМНЫЙ ДОМ</h1>");
            client.println("<h1>В моей комнате:</h1>");
          
            client.println("<p>Температура: ");
            client.println((int)temperature);
            client.println(" °C</p>");
            client.println("<p>Влажность: ");
            client.println((int)humidity);
            client.println(" %</p>");
            client.println("<p>Давление: ");
            client.println((int)pressure);
            client.println(" мм рт. ст.</p>");
            client.println("<p>Концентрация СО₂: ");
            client.println((int)co2);
            client.println(" ‰</p>");
            client.println("<h1>");
            client.println((String)smile);
            client.println(" </h1>");
          
            if (output25State=="off") 
            {
              client.println("<p><a href=\"/25/on\"><button class=\"button\">ON</button></a></p>");
            } 
            else 
            {
              client.println("<p><a href=\"/25/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("</body></html>");
            if (output26State=="off") 
            {
              client.println("<p><a href=\"/26/on\"><button class=\"button\">ON</button></a></p>");
            } 
            else 
            {
              client.println("<p><a href=\"/26/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
            client.println("</body></html>");
            
            // конец HTTP-ответа задается 
            // с помощью дополнительной пустой строки:
            client.println();
            // выходим из цикла while:
            break;
          } 
          else 
          {  // если получили символ новой строки,
                    // очищаем текущую строку «currentLine»:
            currentLine = "";
          }
        } 
        else if (c != '\r') 
        {  // если получили любые данные,
                                 // кроме символа возврата каретки,
          currentLine += c;      // добавляем эти данные 
                                 // в конец строки «currentLine»
        }
      }
    }
    // очищаем переменную «header»:
    header = "";
    // отключаем соединение:
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

String SmileFace(float temperaturestat, float humiditystat, int minTemperature, int maxTemperature, int minHumidity, int maxHumidity)
{
  String smilestat;
  if ((temperaturestat < minTemperature) || (temperaturestat > maxTemperature) || (humiditystat < minHumidity) || (humiditystat > maxHumidity))
  {
    smilestat = " 🤕 ";
  }
  else
  {
    smilestat = " 😀 ";
  }
  return smilestat;
}
