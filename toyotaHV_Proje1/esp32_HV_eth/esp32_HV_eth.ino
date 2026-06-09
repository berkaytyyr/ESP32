#include <WiFi.h>
#include <SPI.h>
#include <Ethernet.h>  
#include <esp_now.h>
#include <PubSubClient.h>

#define W5500_MOSI 11
#define W5500_MISO 12
#define W5500_SCK  13
#define W5500_CS   14
#define W5500_RST  9
// çalıştırmak için bilgileri giriniz
byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
IPAddress local_IP(0,0,0,0);
IPAddress gateway(0,0,0,0);
IPAddress subnet(0,0,0,0);

const char* mqtt_server = "";
const int mqtt_port = ;
const char* mqtt_user = "";
const char* mqtt_pass = "";
const char* mqtt_topic = "";

EthernetClient ethClient;  
PubSubClient client(ethClient);

typedef struct struct_message {
  char mesaj[64]; // Vericiyle aynı boyutta olmalı
} struct_message;

volatile bool yeniVeri = false;
char gelenMesaj[64];

volatile unsigned long sonMesajZamani = 0;
bool ilkMesajGeldi = false;    
const unsigned long zamanAsimi = 10000; 
unsigned long lastReconnectAttempt = 0;

void OnDataRecv(const esp_now_recv_info *info, const uint8_t *data, int len) {
  memset(gelenMesaj, 0, sizeof(gelenMesaj));
  memcpy(gelenMesaj, data, min(len, (int)sizeof(gelenMesaj)-1));
  sonMesajZamani = millis(); 
  yeniVeri = true;
}

void setupETH() {
  pinMode(W5500_RST, OUTPUT);
  digitalWrite(W5500_RST, LOW);
  delay(100);
  digitalWrite(W5500_RST, HIGH);
  delay(150);

  SPI.begin(W5500_SCK, W5500_MISO, W5500_MOSI, W5500_CS);
  Ethernet.init(W5500_CS);
  Ethernet.begin(mac, local_IP, gateway, gateway, subnet);
  delay(1000);

  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Kritik Hata: W5500 Cipi Bulunamadi!");
  } else if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Hata: Ethernet Kablosu Yok.");
  } else {
    Serial.print("Ethernet Basarili! IP: ");
    Serial.println(Ethernet.localIP());
  }
  client.setServer(mqtt_server, mqtt_port);
}

boolean reconnectMQTT() {
  Serial.print("MQTT Baglantisi deneniyor...");
  if (client.connect("ESP32_ETH_Client", mqtt_user, mqtt_pass)) {
    Serial.println("Baglandi!");
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  setupETH();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW hatasi!");
    return;
  }
  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Sistem hazir...");
}

void loop() {
  unsigned long suAn = millis();

  if (!client.connected()) {
    if (suAn - lastReconnectAttempt > 5000) { 
      lastReconnectAttempt = suAn;
      if (reconnectMQTT()) lastReconnectAttempt = 0;
    }
  } else {
    client.loop();
  }



  if (yeniVeri) {
    yeniVeri = false;
    ilkMesajGeldi = true;

    if (strcmp(gelenMesaj, "PING") != 0) {
      Serial.printf("Ham veri: %s\n", gelenMesaj);
      if (client.connected()) {
        client.publish(mqtt_topic, gelenMesaj);
      }
    }
  }
}
