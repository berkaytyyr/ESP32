#include <esp_now.h>
#include <WiFi.h>

// Kuru kontak pinleri
#define PIN_K1 27
#define PIN_K2 26
#define PIN_K3 25
#define PIN_K4 33

// ESP32-S3 ETH MAC adresi (alıcı)
uint8_t receiverMAC[] = {0x00, 0x00, 0x00, 0x0, 0x00, 0x00}; 

typedef struct struct_message {
  char mesaj[64]; // String ID ve JSON içeriği için boyut 64
} struct_message;

struct_message data;

// Önceki durumları kontrol için
bool prevK1 = false;
bool prevK2 = false;
bool prevK3 = false;
bool prevK4 = false;

// --- ID DEĞİŞİKLİĞİ BURADA ---
const char* id = "MB1-1"; 


unsigned long sonPingZamani = 0;
const unsigned long pingAraligi = 3000; 

void setup() {
  Serial.begin(115200);

  pinMode(PIN_K1, INPUT_PULLUP);
  pinMode(PIN_K2, INPUT_PULLUP);
  pinMode(PIN_K3, INPUT_PULLUP);
  pinMode(PIN_K4, INPUT_PULLUP);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW başlatılamadı");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receiverMAC, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  Serial.println("WROOM hazır, veri gonderimi bekleniyor...");
}

void loop() {
  bool k1 = !digitalRead(PIN_K1);
  bool k2 = !digitalRead(PIN_K2);
  bool k3 = !digitalRead(PIN_K3);
  bool k4 = !digitalRead(PIN_K4);

  if (k1 != prevK1 || k2 != prevK2 || k3 != prevK3 || k4 != prevK4) {

    sprintf(data.mesaj, "{\"k1\":%d,\"k2\":%d,\"k3\":%d,\"k4\":%d,\"id\":\"%s\"}", k1, k2, k3, k4, id);

    esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));
    Serial.println(data.mesaj);

    prevK1 = k1;
    prevK2 = k2;
    prevK3 = k3;
    prevK4 = k4;
  }

  unsigned long suAn = millis();
  if (suAn - sonPingZamani >= pingAraligi) {
    sonPingZamani = suAn;
    sprintf(data.mesaj, "PING");
    esp_now_send(receiverMAC, (uint8_t*)&data, sizeof(data));
  }

  delay(100); 
}
