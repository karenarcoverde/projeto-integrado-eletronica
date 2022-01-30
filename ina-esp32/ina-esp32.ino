#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_INA219.h>

const char *ssid = "SATURNO";
const char *password = "erickkaren";
//const char *serverName = "http://192.168.0.21:1880/controleBateria";
const char *serverName = "http://177.65.22.167:8080/diego";

int UVOUT = 36;   /* Pino D36 do ESP32 conetado ao Out do sensor */
int REF   = 34;   /* Pino D34 do ESP32 conectado ao EN do sensor */



WiFiClient client;
HTTPClient http;

Adafruit_INA219 ina219;

void connectToInternet();
void printConnectionInformation();

void setup(void)
{
   Serial.begin(115200);
   pinMode(UVOUT, INPUT);
   pinMode(REF, INPUT);
  
  
  delay(1000);
  while (!Serial) {
    delay(1);
  }

  connectToInternet();

  uint32_t currentFrequency;

  if (! ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) {
      delay(10);
    }
  }
  Serial.println("Measuring voltage and current with INA219 ...");

  
}



void loop(void)
{
  float shuntvoltage = 0;
  float busvoltage = 0;
  float current_mA = 0;
  float loadvoltage = 0;
  float power_mW = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
  current_mA = ina219.getCurrent_mA();
  power_mW = ina219.getPower_mW();
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  int uvLevel = averageAnalogRead(UVOUT);           /* Armazena a leitura analógica do pino OUT */
  int refLevel = averageAnalogRead(REF);            /* Armazena a leitura analógica do pino EN */
  /* Use 3.3V como referencia no calculo de tensão */
  float outputVoltage = 3.3 / refLevel * uvLevel; /* Indica a tensão de saída do sensor */
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0); /* Intensidade raios UV */
  
  Serial.println(uvLevel);
  Serial.println(refLevel);
  Serial.print(" Intensidade UV: ");
  Serial.println(uvIntensity);
  int onda = map(uvLevel, 0, refLevel, 0, 1023);
  Serial.print("Classificacao: ");
  Serial.println (onda);
  Serial.println(); 
  delay(200);

 
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    http.addHeader("Content-Type", "text/plain");
    String httpRequestData = String(busvoltage) + "," +
                             String(shuntvoltage) + "," +
                             String(loadvoltage) + "," +
                             String(current_mA) + "," +
                             String(power_mW) + "," +
                             String(uvIntensity);
    int httpResponseCode = http.POST(httpRequestData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);

    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
  
  delay(2000);
}

 
//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 
 
  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
 
  return(runningValue);
}
 
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void connectToInternet() {
  WiFi.begin(ssid, password);
  Serial.print("[INFO] Attempting Connection - SSID: ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("\n[INFO] Connection Successful.\n");
  printConnectionInformation();
}

void printConnectionInformation() {
  Serial.print("[INFO] SSID: ");
  Serial.println(WiFi.SSID());

  // Exibe o endereço MAC do roteador
  Serial.print("[INFO] BSSID: ");
  Serial.println(WiFi.BSSIDstr());

  // Exibe a intensidade do sinal recebido
  Serial.print("[INFO] Received Signal Strength Indication (RSSI): ");
  Serial.println(WiFi.RSSI());

  // Exibe o endereço IP
  Serial.print("[INFO] Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  // Exibe o endereço MAC do dispositivo
  Serial.print("[INFO] MAC Address: ");
  Serial.println(WiFi.macAddress());
}
