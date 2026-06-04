# ESP32 Local Chat Server (SoftAP) 💬

Bu proje, bir ESP32 mikrodenetleyicisini internete ihtiyaç duymayan yerel bir sohbet sunucusuna dönüştürür. ESP32, kendi Wi-Fi ağını (Access Point) oluşturur ve kullanıcılar bu ağa bağlanarak tarayıcı üzerinden birbirleriyle gerçek zamanlı olarak mesajlaşabilir.



## ✨ Özellikler
- **SoftAP Modu:** Harici bir modem veya internet gerekmez.
- **Gerçek Zamanlı Arayüz:** JavaScript `fetch` ve `setInterval` ile sayfayı yenilemeden mesajlaşma.
- **Aktif Kullanıcı Takibi:** Kimlerin çevrimiçi olduğunu ve IP adreslerini görme imkanı.
- **Hafıza Yönetimi:** "Ring Buffer" yapısı sayesinde sınırlı bellekte çökme olmadan mesaj döngüsü.
- **Modern Tasarım:** WhatsApp benzeri, mobil uyumlu CSS arayüzü.

## 🛠 Donanım Gereksinimleri
* Herhangi bir **ESP32** geliştirme kartı (WROOM, S3, C3 vb.)
* USB kablosu ve güç kaynağı.

## 🚀 Kurulum ve Kullanım
1. Kodu Arduino IDE üzerinden ESP32 kartınıza yükleyin.
2. Bilgisayarınızdan veya telefonunuzdan Wi-Fi ağlarını taratın.
3. **"ESP32_Lokal_Sohbet"** ağına bağlanın.
4. Tarayıcınızı açın ve adres çubuğuna şu IP'yi yazın: `192.168.4.1`
5. Adınızı girin ve sohbete başlayın!

## 📂 Kod Yapısı Hakkında
* **WebServer.h:** Gelen HTTP isteklerini yönetmek için kullanılır.
* **JSON Parçalama:** Veriler `application/json` formatında, ESP32'nin RAM'ini yormamak için "Chunked" (parçalı) yöntemle gönderilir.
* **Hafıza Sınırı:** Varsayılan olarak son 40 mesaj ve 20 kullanıcıyı hafızada tutar.

## 🤝 Katkıda Bulunma
Bu proje geliştirilmeye açıktır. Örneğin; mesajların kalıcı olması için SD kart desteği veya şifreleme özellikleri eklenebilir. Pull request açarak katkıda bulunabilirsiniz!

---
**Geliştiren:** [Adınız/Kullanıcı Adınız]
