#include <WiFi.h>
#include <SPI.h>
#include <Ethernet.h>  
#include <esp_now.h>
#include <PubSubClient.h>

// --- Waveshare ESP32-S3-ETH (W5500) Pinleri ---
#define W5500_MOSI 11
#define W5500_MISO 12
#define W5500_SCK  13
#define W5500_CS   14
#define W5500_RST  9

// --- Ağ Ayarları ---
byte mac[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; 
IPAddress local_IP(0, 0, 0, 0);
IPAddress gateway(0, 0, 0, 0);
IPAddress subnet(0, 0, 0, 0);

EthernetClient ethClient;  
PubSubClient client(ethClient);


typedef struct { 
    char id[16];      // String ID (MB2-1 vb.)
    bool state; 
    int eventType;    // 1: Uyandı, 2: Uyudu, 0: Veri
} struct_message;

struct_message incoming;
volatile bool yeniVeri = false;


void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    memcpy(&incoming, data, sizeof(incoming));
    yeniVeri = true;
}

void setup() {
    Serial.begin(115200);

    // 1. W5500 Reset ve Ethernet Başlat
    pinMode(W5500_RST, OUTPUT);
    digitalWrite(W5500_RST, LOW); delay(100);
    digitalWrite(W5500_RST, HIGH); delay(150);

    SPI.begin(W5500_SCK, W5500_MISO, W5500_MOSI, W5500_CS);
    Ethernet.init(W5500_CS);
    Ethernet.begin(mac, local_IP, gateway, gateway, subnet);

    // 2. Wi-Fi ve ESP-NOW Başlat
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    if (esp_now_init() == ESP_OK) {
        esp_now_register_recv_cb((esp_now_recv_cb_t)OnDataRecv);
    }

    client.setServer("10.0.123.1", 1883);
}

void loop() {
    // MQTT Bağlantı Kontrolü
    if (!client.connected()) {
        // Rastgele ClientID ile çakışmayı önle
        String clientId = "S3_ETH_GW_" + String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), "fedmqtt", "Aa12345")) {
            Serial.println("MQTT Baglantisi Kuruldu.");
        }
    }
    client.loop();

    // Veri Geldiğinde İşle
    if (yeniVeri) {
        yeniVeri = false;

        // Olay Tipi Kontrolü (Seri Port Bildirimi)
        if (incoming.eventType == 1) {
            Serial.printf("[%s]: UYANDI\n", incoming.id); // %d yerine %s (string)
        } 
        else if (incoming.eventType == 2) {
            Serial.printf("[%s]: UYKUYA DALDI\n", incoming.id);
        }
        else if (incoming.eventType == 0) {
            // Gerçek Veri (Kontak Durumu) Geldiğinde MQTT'ye JSON Gönder
            char buffer[128];
            // JSON içinde ID tırnak içinde gönderilmeli
            snprintf(buffer, sizeof(buffer), "{\"id\":\"%s\",\"guc\":%d}", incoming.id, incoming.state);
            
            if (client.connected()) {
                client.publish("kontak/messaging", buffer);
                Serial.printf("MQTT'ye iletildi -> %s\n", buffer);
            }
        }
    }
}
