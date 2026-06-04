#include <WiFi.h>
#include <WebServer.h>

// --- AĞ AYARLARI ---
const char* ssid = "ESP32_Lokal_Sohbet";
const char* password = ""; // İstersen buraya 8 haneli bir şifre yazabilirsin

WebServer server(80);

// --- VERİ YAPILARI ---
#define MAX_MESAJ 40
struct Mesaj {
  String gonderen;
  String metin;
  String saat;
};
Mesaj mesajlar[MAX_MESAJ];
int mesajIndeks = 0;
int mesajSayisi = 0;

#define MAX_KULLANICI 20
struct Kullanici {
  String isim;
  String ip;
  String girisSaati;
  String sonSaat;
  unsigned long sonGorulme;
};
Kullanici kullanicilar[MAX_KULLANICI];
int kullaniciSayisi = 0;

// --- HTML, CSS VE JS ARAYÜZÜ ---
const char* htmlKodu = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1">
    <title>ESP32 Lokal Sohbet</title>
    <style>
        body { margin:0; padding:0; font-family: 'Segoe UI', Arial, sans-serif; background-color: #e5ddd5; display:flex; flex-direction:column; height:100vh; }
        
        /* GİRİŞ EKRANI */
        #girisEkrani { display:flex; flex-direction:column; justify-content:center; align-items:center; height:100vh; background:#fff; }
        .giris-kutu { text-align:center; padding:30px; border-radius:10px; box-shadow:0 0 15px rgba(0,0,0,0.1); background:#f9f9f9;}
        #kullaniciAdi { padding:10px; font-size:16px; border:1px solid #ccc; border-radius:5px; margin-bottom:15px; width:80%; outline:none;}
        #girisBtn { background:#075e54; color:white; padding:10px 20px; font-size:16px; border:none; border-radius:5px; cursor:pointer; width:80%;}
        
        /* SOHBET EKRANI */
        #sohbetEkrani { display:none; flex-direction:column; height:100vh; }
        .ust-panel { background:#075e54; color:white; padding:15px; text-align:center; position:relative; box-shadow:0 2px 5px rgba(0,0,0,0.2); z-index:10;}
        #aktiflerBtn { position:absolute; right:15px; top:12px; background:#25d366; color:white; border:none; padding:5px 10px; border-radius:15px; cursor:pointer; font-weight:bold;}
        
        /* AKTİFLER LİSTESİ */
        #aktiflerPaneli { display:none; background:white; padding:10px; border-bottom:1px solid #ccc; max-height:30%; overflow-y:auto; box-shadow:inset 0 -5px 10px rgba(0,0,0,0.05);}
        .kul-satir { margin-bottom:8px; border-bottom:1px solid #eee; padding-bottom:5px; }
        .durum { display:inline-block; width:10px; height:10px; border-radius:50%; margin-right:5px; }
        .durum.yesil { background-color: #25d366; }
        .durum.kirmizi { background-color: #ff3333; }
        .kul-isim { font-weight:bold; font-size:14px; color:#333;}
        .kul-detay { display:block; font-size:11px; color:#888; margin-left:15px; margin-top:3px;}
        
        /* MESAJ ALANI */
        #mesajAlani { flex:1; overflow-y:auto; padding:15px; display:flex; flex-direction:column; }
        .mesaj { max-width:75%; padding:8px 12px; margin-bottom:10px; border-radius:10px; position:relative; word-wrap: break-word; box-shadow:0 1px 2px rgba(0,0,0,0.1);}
        .mesaj-sol { background:#ffffff; align-self:flex-start; border-top-left-radius:0; }
        .mesaj-sag { background:#dcf8c6; align-self:flex-end; border-top-right-radius:0; }
        .mesaj-gonderen { font-size:12px; font-weight:bold; color:#075e54; margin-bottom:3px; }
        .mesaj-metin { font-size:15px; color:#111; }
        .mesaj-saat { font-size:10px; color:#999; text-align:right; margin-top:4px; }
        
        /* ALT MESAJ GÖNDERME PANELİ */
        .alt-panel { display:flex; padding:10px; background:#f0f0f0; }
        #mesajKutusu { flex:1; padding:12px; border:none; border-radius:20px; outline:none; font-size:15px; }
        #gonderBtn { background:#075e54; color:white; border:none; padding:0 20px; margin-left:10px; border-radius:20px; cursor:pointer; font-weight:bold; font-size:15px;}
    </style>
</head>
<body>

    <div id="girisEkrani">
        <div class="giris-kutu">
            <h2 style="color:#075e54; margin-top:0;">Lokal Sohbete Katıl</h2>
            <input type="text" id="kullaniciAdi" placeholder="Adınızı girin..." maxlength="15" onkeypress="if(event.keyCode==13) sohbeteKatil()">
            <button id="girisBtn" onclick="sohbeteKatil()">Sohbete Başla</button>
        </div>
    </div>

    <div id="sohbetEkrani">
        <div class="ust-panel">
            <span style="font-size:18px; font-weight:bold;">Merkez Sohbet</span>
            <button id="aktiflerBtn" onclick="aktifleriAcKapat()">Aktifler (0)</button>
        </div>
        
        <div id="aktiflerPaneli"></div>
        
        <div id="mesajAlani"></div>
        
        <div class="alt-panel">
            <input type="text" id="mesajKutusu" placeholder="Mesaj yaz..." onkeypress="if(event.keyCode==13) mesajGonder()">
            <button id="gonderBtn" onclick="mesajGonder()">Gönder</button>
        </div>
    </div>

    <script>
        let benimAdim = "";
        let sonMesajSayisi = 0;

        // Tarayıcıdan o anki saati 14:05 formatında alma fonksiyonu
        function saatAl() {
            let d = new Date();
            let s = d.getHours().toString().padStart(2, '0');
            let m = d.getMinutes().toString().padStart(2, '0');
            return s + ":" + m;
        }

        function sohbeteKatil() {
            let isim = document.getElementById("kullaniciAdi").value.trim();
            if(isim === "") { alert("Lütfen bir isim girin!"); return; }
            benimAdim = isim;
            document.getElementById("girisEkrani").style.display = "none";
            document.getElementById("sohbetEkrani").style.display = "flex";
            
            veriCek(); 
            setInterval(veriCek, 1500); // Her 1.5 saniyede bir ekranı güncelle
        }

        function aktifleriAcKapat() {
            let panel = document.getElementById("aktiflerPaneli");
            panel.style.display = (panel.style.display === "block") ? "none" : "block";
        }

        function mesajGonder() {
            let kutu = document.getElementById("mesajKutusu");
            let metin = kutu.value.trim();
            if(metin === "") return;
            
            let formData = new URLSearchParams();
            formData.append('isim', benimAdim);
            formData.append('mesaj', metin);
            formData.append('saat', saatAl());

            fetch('/mesajYaz', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData
            }).then(() => {
                kutu.value = "";
                veriCek(); // Mesajı sunucuya iletir iletmez ekranı yenile
            });
        }

        function veriCek() {
            fetch('/veriAl?isim=' + encodeURIComponent(benimAdim) + '&saat=' + encodeURIComponent(saatAl()))
            .then(res => res.json())
            .then(data => {
                // 1. AKTİFLER LİSTESİNİ GÜNCELLE
                let aktiflerHtml = "";
                let aktifSayisi = 0;
                
                data.kullanicilar.forEach(k => {
                    let durumClass = k.aktif ? "yesil" : "kirmizi";
                    if(k.aktif) aktifSayisi++;
                    aktiflerHtml += `
                        <div class="kul-satir">
                            <div class="durum ${durumClass}"></div>
                            <span class="kul-isim">${k.isim}</span>
                            <span class="kul-detay">IP: ${k.ip} &nbsp;|&nbsp; Giriş: ${k.giris} &nbsp;|&nbsp; Son Görülme: ${k.son}</span>
                        </div>
                    `;
                });
                document.getElementById("aktiflerPaneli").innerHTML = aktiflerHtml;
                document.getElementById("aktiflerBtn").innerText = "Aktifler (" + aktifSayisi + ")";

                // 2. MESAJLARI GÜNCELLE
                if(data.mesajlar.length !== sonMesajSayisi) {
                    let mesajlarHtml = "";
                    data.mesajlar.forEach(m => {
                        let benimMi = (m.gonderen === benimAdim);
                        let sinif = benimMi ? "mesaj-sag" : "mesaj-sol";
                        let gonderenIsim = benimMi ? "Sen:" : m.gonderen;
                        
                        mesajlarHtml += `
                            <div class="mesaj ${sinif}">
                                <div class="mesaj-gonderen">${gonderenIsim}</div>
                                <div class="mesaj-metin">${m.metin}</div>
                                <div class="mesaj-saat">${m.saat}</div>
                            </div>
                        `;
                    });
                    
                    let alan = document.getElementById("mesajAlani");
                    alan.innerHTML = mesajlarHtml;
                    sonMesajSayisi = data.mesajlar.length;
                    alan.scrollTop = alan.scrollHeight; // Yeni mesajda ekranı en alta kaydır
                }
            });
        }
    </script>
</body>
</html>
)rawliteral";

// --- YARDIMCI FONKSİYONLAR ---
void kullaniciGuncelle(String isim, String ip, String anlikSaat) {
  if (isim == "" || isim == "null") return;
  
  for (int i = 0; i < kullaniciSayisi; i++) {
    if (kullanicilar[i].ip == ip || kullanicilar[i].isim == isim) {
      kullanicilar[i].isim = isim; 
      kullanicilar[i].sonSaat = anlikSaat;
      kullanicilar[i].sonGorulme = millis();
      return;
    }
  }
  
  // Listede yoksa yeni kullanıcı olarak kaydet
  if (kullaniciSayisi < MAX_KULLANICI) {
    kullanicilar[kullaniciSayisi].isim = isim;
    kullanicilar[kullaniciSayisi].ip = ip;
    kullanicilar[kullaniciSayisi].girisSaati = anlikSaat;
    kullanicilar[kullaniciSayisi].sonSaat = anlikSaat;
    kullanicilar[kullaniciSayisi].sonGorulme = millis();
    kullaniciSayisi++;
  }
}

// --- SUNUCU ROTALARI (Gelen İstekleri Karşılama) ---
void handleRoot() {
  server.send(200, "text/html", htmlKodu);
}

void handleMesajYaz() {
  if (server.hasArg("isim") && server.hasArg("mesaj") && server.hasArg("saat")) {
    String isim = server.arg("isim");
    String mesaj = server.arg("mesaj");
    String saat = server.arg("saat");
    String ip = server.client().remoteIP().toString();
    

    // Mesajı hafıza listesine ekle
    mesajlar[mesajIndeks].gonderen = isim;
    mesajlar[mesajIndeks].metin = mesaj;
    mesajlar[mesajIndeks].saat = saat;
    
    mesajIndeks++;
    if (mesajIndeks >= MAX_MESAJ) mesajIndeks = 0; // Kutu dolunca en baştan yazmaya başlar (Ring Buffer)
    if (mesajSayisi < MAX_MESAJ) mesajSayisi++;
    
    // Mesaj yazanı aktif tut
    kullaniciGuncelle(isim, ip, saat);
    
    server.send(200, "text/plain", "OK");
  } else {
    server.send(400, "text/plain", "Hata");
  }
}

void handleVeriAl() {
  // Biri sayfayı yeniledikçe son görülmesini güncelle
  if (server.hasArg("isim") && server.hasArg("saat")) {
    String ip = server.client().remoteIP().toString();
    kullaniciGuncelle(server.arg("isim"), ip, server.arg("saat"));
  }

  // Cihaz çökmesin diye verileri parça parça (Chunked) gönderiyoruz
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "application/json", "");
  
  // 1. KULLANICILAR JSON KISMI
  server.sendContent("{\"kullanicilar\":[");
  for (int i = 0; i < kullaniciSayisi; i++) {
    // 10 saniye boyunca hiç sinyal (veriAl isteği) göndermeyen kişi "Pasif (Kırmızı)" sayılır
    bool aktifMi = (millis() - kullanicilar[i].sonGorulme) < 10000; 
    
    String kJSON = "{";
    kJSON += "\"isim\":\"" + kullanicilar[i].isim + "\",";
    kJSON += "\"ip\":\"" + kullanicilar[i].ip + "\",";
    kJSON += "\"giris\":\"" + kullanicilar[i].girisSaati + "\",";
    kJSON += "\"son\":\"" + kullanicilar[i].sonSaat + "\",";
    kJSON += "\"aktif\":" + String(aktifMi ? "true" : "false");
    kJSON += "}";
    
    if (i < kullaniciSayisi - 1) kJSON += ",";
    server.sendContent(kJSON);
  }
  
  // 2. MESAJLAR JSON KISMI
  server.sendContent("],\"mesajlar\":[");
  
  int baslangic = (mesajSayisi < MAX_MESAJ) ? 0 : mesajIndeks;
  for (int i = 0; i < mesajSayisi; i++) {
    int gercekIndeks = (baslangic + i) % MAX_MESAJ;
    
    // Mesajın içindeki olası tırnak işaretlerini maskele (JSON bozulmasını önler)
    String temizMetin = mesajlar[gercekIndeks].metin;
    temizMetin.replace("\"", "\\\"");
    temizMetin.replace("\n", "\\n");
    
    String mJSON = "{";
    mJSON += "\"gonderen\":\"" + mesajlar[gercekIndeks].gonderen + "\",";
    mJSON += "\"metin\":\"" + temizMetin + "\",";
    mJSON += "\"saat\":\"" + mesajlar[gercekIndeks].saat + "\"";
    mJSON += "}";
    
    if (i < mesajSayisi - 1) mJSON += ",";
    server.sendContent(mJSON);
  }
  
  server.sendContent("]}");
  server.sendContent(""); // Parçalı gönderimi sonlandır
}

void setup() {
  Serial.begin(115200);
  
  // ESP32'yi Wi-Fi Vericisi (Access Point) yapıyoruz
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  Serial.println("\nWiFi Agi Baslatildi!");
  Serial.print("Telefondan baglanilacak IP Adresi: ");
  Serial.println(WiFi.softAPIP());

  // Web Sunucusu Rotalarını Başlat
  server.on("/", handleRoot);
  server.on("/mesajYaz", HTTP_POST, handleMesajYaz);
  server.on("/veriAl", HTTP_GET, handleVeriAl);

  server.begin();
  Serial.println("Lokal Sohbet Sunucusu Hazir. Yayindayiz!");
}

void loop() {
  server.handleClient(); // Arka planda gelen bağlantıları sürekli dinler
}
