#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_INA219.h>

const char *ssid = "SATURNO";
const char *password = "erickkaren";
const char *serverName = "http://svuv-api.herokuapp.com/data_read/load";

int UVOUT = 36;   /* Pino D36 do ESP32 conetado ao Out do sensor UV*/
int REF   = 34;   /* Pino D34 do ESP32 conectado ao EN do sensor UV */
String UV_index = "0";
String risco = "0";

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
  float loadvoltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
 
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  int uvLevel = averageAnalogRead(UVOUT);           /* Armazena a leitura analógica do pino OUT */
  int refLevel = averageAnalogRead(REF);            /* Armazena a leitura analógica do pino EN */
  /* Use 3.3V como referencia no calculo de tensão */
  float outputVoltage = 3.3 / refLevel * uvLevel; /* Indica a tensão de saída do sensor */
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); /* Intensidade raios UV */

  int tensao = map(uvLevel, 0, refLevel, 0, 1023);
  
  //Compara com valores tabela UV_Index
  if (tensao > 0 && tensao < 50)
  {
    UV_index = "0";
    risco = "baixo";
  }
  else if (tensao > 50 && tensao <= 227)
  {
    UV_index = "0";
    risco = "baixo";
  }
  else if (tensao > 227 && tensao <= 318)
  {
    UV_index = "1";
    risco = "baixo";
  }
  else if (tensao > 318 && tensao <= 408)
  {
    UV_index = "2";
    risco = "baixo";
  }
  else if (tensao > 408 && tensao <= 503)
  {
    UV_index = "3";
    risco = "moderado";
  }
  else if (tensao > 503 && tensao <= 606)
  {
    UV_index = "4";
    risco = "moderado";
  }
  else if (tensao > 606 && tensao <= 696)
  {
    UV_index = "5";
    risco = "moderado";
  }
  else if (tensao > 696 && tensao <= 795)
  {
    UV_index = "6";
    risco = "alto";
  }
  else if (tensao > 795 && tensao <= 881)
  {
    UV_index = "7";
    risco = "alto";
  }
  else if (tensao > 881 && tensao <= 976)
  {
    UV_index = "8";
    risco = "muito alto";
  }
  else if (tensao > 976 && tensao <= 1079)
  {
    UV_index = "9";
    risco = "muito alto";
  }
  else if (tensao > 1079 && tensao <= 1170)
  {
    UV_index = "10";
    risco = "muito alto";
  }
  else if (tensao > 1170)
  {
    UV_index = "11";
    risco = "extremo";
  }

  Serial.print("Classificacao: ");
  Serial.println (tensao);
  Serial.println(UV_index);
  Serial.println(risco);
  Serial.println(); 
  delay(200);
 
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/json");
    String body = "{\"voltage\": "+ String(loadvoltage) + " ,\"uv\": "+ String(UV_index) + ", \"risco\": "+ String(risco) + "}";
    int httpResponseCode = http.POST(body);
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
