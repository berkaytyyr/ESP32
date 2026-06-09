#include <esp_now.h>
#include <WiFi.h>
#include <esp_sleep.h>

// S3-ETH MAC Adresini buraya yaz (Senin verdiğin adres)
uint8_t broadcastAddress[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; 


const char* DEVICE_NAME = "MB2-1"; // id' yi degistir

#define CONTACT_PIN GPIO_NUM_26 // Senin kullandığın pin

typedef struct {
    char id[16];      // S3-ETH ile aynı boyutta (16) olmalı
    bool state;       
    int eventType;    // 1: Uyandı, 2: Uykuya Dalıyor, 0: Normal Veri
} struct_message;

struct_message myData;

void sendSignal(int event) {
    myData.eventType = event;
    esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
    delay(150); // Gönderimin tamamlanması için güvenli süre
}

void setup() {
    // Seri portu sadece test için açabilirsin (isteğe bağlı)
    // Serial.begin(115200);

    pinMode(CONTACT_PIN, INPUT_PULLUP);
    
    // Wi-Fi Ayarları
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    if (esp_now_init() != ESP_OK) return;

    // Peer (Eşleşme) Ayarları
    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    // Kritik Düzeltme: String ataması snprintf ile yapılır
    snprintf(myData.id, sizeof(myData.id), "%s", DEVICE_NAME);
    
    // 1. ADIM: "Uyandı" Sinyali Gönder
    sendSignal(1);

    // 2. ADIM: Kontak Durumunu Gönder (Ters Mantık)
    // Kontak 0 (LOW) ise state=true (1), 1 (HIGH) ise state=false (0)
    int okunan = digitalRead(CONTACT_PIN);
    myData.state = (okunan == LOW) ? true : false;
    sendSignal(0); 

    // 3. ADIM: "Uykuya Dalıyor" Sinyali Gönder
    sendSignal(2);

    // Uyandırma Ayarı: Mevcut durumun tersi bir sinyal gelirse uyan
    esp_sleep_enable_ext0_wakeup(CONTACT_PIN, !okunan);
    
    // Derin Uykuya Başla
    esp_deep_sleep_start();
}

void loop() {
    // Derin uykuda burası asla çalışmaz
}
