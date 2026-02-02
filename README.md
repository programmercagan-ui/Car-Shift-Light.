ESP32 Shift Light

ESP32 tabanlı, araçtaki RPM değerine göre LED şeritleri kontrol eden bir Shift Light projesi. ELM327 OBD-II üzerinden RPM verilerini alır ve hem Günlük hem de Cadde modunda farklı LED efektleri uygular. Ayrıca Wi-Fi üzerinden bir web arayüzü ile mod ve LED ayarlarını değiştirebilirsiniz.

Özellikler

OBD-II ile gerçek zamanlı RPM takibi

Günlük Modu: RPM seviyesine göre LED renk değişimi ve kırmızıda strobe efekti

Cadde Modu: Renkli chase efekti ve strobe

Web arayüzü:

Mod seçimi (Günlük / Cadde)

RPM ayarları (Başlangıç, Yeşil, Sarı, Kırmızı)

LED parlaklık ve strobe hızı ayarı

EEPROM ile ayarların kalıcı olması

FastLED kütüphanesi ile LED kontrolü

Donanım Gereksinimleri

ESP32 geliştirme kartı

WS2812 veya benzeri adreslenebilir LED şerit (Örnek: 8 LED)

ELM327 Bluetooth OBD-II adaptör

Araç (OBD-II portu)

Yazılım Gereksinimleri

Arduino IDE veya PlatformIO

Kütüphaneler:

WiFi.h

WebServer.h

EEPROM.h

BluetoothSerial.h

ELMduino

FastLED

Kurulum

Arduino IDE veya PlatformIO’yu açın ve ESP32 kart desteğini ekleyin.

Gerekli kütüphaneleri yükleyin.

Kod içindeki ELM327 MAC adresini aracınıza göre ayarlayın:

uint8_t address[6] = {0xAA, 0xBB, 0xCC, 0x11, 0x22, 0x33};


LED sayısını ve pinini ayarlayın:

#define LED_PIN 5
#define NUM_LEDS 8


Arduino’ya yükleyin ve ESP32 başlatıldığında Wi-Fi ağı SHIFT_LIGHT açılacaktır.

Kullanım

Araçta OBD-II adaptörünü bağlayın.

ESP32 otomatik olarak ELM327 ile bağlanır.

Wi-Fi üzerinden ESP32’ye bağlanın (SSID: SHIFT_LIGHT, şifre: 12345678)

Web tarayıcısında 192.168.4.1 adresine gidin.

Mod ve RPM değerlerini ayarlayın, LED parlaklığı ve strobe hızını değiştirin.

LED Modları
Günlük Modu

RPM < rpmStart: LED kapalı

rpmStart ≤ RPM < rpmGreenEnd: Yeşil LED

rpmGreenEnd ≤ RPM < rpmYellowEnd: Sarı LED

rpmYellowEnd ≤ RPM < rpmRedEnd: Kırmızı LED

RPM ≥ rpmRedEnd: Kırmızı strobe

Cadde Modu

RPM < 800: LED kapalı

RPM ≥ rpmRedEnd: Strobe

RPM diğer değerler: Renkli chase efekti, hız RPM’ye göre değişir

EEPROM Ayarları

0: Mod (0 = Günlük, 1 = Cadde)

4: RPM başlangıç

8: RPM yeşil son

12: RPM sarı son

16: RPM kırmızı son

20: LED parlaklık

24: Strobe hızı

Dikkat Edilecekler

OBD bağlantısı olmadan kod LED efektlerini çalıştırmaz.

Strobe hızı ve LED parlaklığı çok yüksek olursa LED’ler zarar görebilir.

Arduino Serial Monitör ile hata mesajlarını takip edebilirsiniz.

Lisans

MIT License © 2026
