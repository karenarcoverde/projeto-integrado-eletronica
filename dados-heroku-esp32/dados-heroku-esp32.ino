#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_INA219.h>
#include <ML8511.h>

const char *ssid = "SATURNO";
const char *password = "erickkaren";
const char *serverName = "http://svuv-api.herokuapp.com/data_read/load";

int UVOUT = 36;   /* Pino D36 do ESP32 conetado ao Out do sensor UV*/
int REF   = 34;   /* Pino D34 do ESP32 conectado ao EN do sensor UV */


ML8511 light(UVOUT, REF);

WiFiClient client;
HTTPClient http;

Adafruit_INA219 ina219;

void connectToInternet();
void printConnectionInformation();

void setup(void)
{
   Serial.begin(115200);
  // manually enable / disable the sensor.
  light.enable();

  // adjust to your ADC specification.
  light.setVoltsPerStep(3.3, 4095);      // 12 bit DAC

  light.setDUVfactor(1.61);    // calibrate your sensor - default 1.61

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
  String risco = "0";
  float shuntvoltage = 0;
  float busvoltage = 0;
  float loadvoltage = 0;

  shuntvoltage = ina219.getShuntVoltage_mV();
  busvoltage = ina219.getBusVoltage_V();
 
  loadvoltage = busvoltage + (shuntvoltage / 1000);

  // 0 - 2  BAIXO
  // 3 - 5 MODERADO
  // 6 - 7 ALTO
  // 8 - 10  MUITO ALTO
  // 11+ EXTREMO
  
  float UV = light.getUV();
  float DUV = light.estimateDUVindex(UV); // 0 a 15

  Serial.print("\tmW cm^2");
  Serial.print("\tDUV index");
  Serial.println();
  
  Serial.print(UV, 4);
  Serial.print("\t");
  Serial.print(DUV, 1);
  Serial.println();
  
  //Compara com valores tabela UV_Index
  if (DUV >= 0 && DUV < 3)
  {
    risco = "baixo";
  }
  else if (DUV >= 3 && DUV < 6)
  {
    risco = "moderado";
  }
  else if (DUV >= 6 && DUV < 8)
  {
    risco = "alto";
  }
  else if (DUV >= 8 && DUV < 11)
  {
    risco = "muito alto";
  }
  else if (DUV >= 11)
  {
    risco = "extremo";
  }

  Serial.println("risco: ");
  Serial.println(risco);

  delay(1000);
  
  if (WiFi.status() == WL_CONNECTED) {
    http.begin(client, serverName);

    http.addHeader("Content-Type", "application/json");
    String body = "{\"voltage\": "+ String(loadvoltage) + " ,\"uv\": "+ String(DUV) + ", \"risco\": "+ String(risco) + "}";
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
