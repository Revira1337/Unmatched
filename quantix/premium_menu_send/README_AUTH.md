# Premium Menu - Auth System Setup

## 🚀 HIZLI BAŞLANGIÇ

### 1. libcurl Kurulumu (Windows)

**Seçenek A: vcpkg (Önerilen)**
```bash
# vcpkg yoksa kur
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# curl'u kur
.\vcpkg install curl:x64-windows
```

**Seçenek B: Manuel İndirme**
- https://curl.se/windows/ adresinden indirin
- x64 version seçin
- Include ve lib klasörlerini not edin

### 2. Visual Studio Ayarları

**Project Properties (Alt+F7) açın:**

#### C/C++ → General → Additional Include Directories
Ekleyin:
```
C:\vcpkg\installed\x64-windows\include
```
(veya curl include klasörünüz)

#### C/C++ → Preprocessor → Preprocessor Definitions
Ekleyin:
```
CURL_STATICLIB
```

#### Linker → General → Additional Library Directories
Ekleyin:
```
C:\vcpkg\installed\x64-windows\lib
```
(veya curl lib klasörünüz)

#### Linker → Input → Additional Dependencies
Zaten eklendi (vcxproj güncel):
```
wbemuuid.lib
iphlpapi.lib
ws2_32.lib
crypt32.lib
```

### 3. Build

```bash
# Visual Studio'da
Configuration: Release
Platform: x64
Build → Build Solution (Ctrl+Shift+B)
```

### 4. discord_config.dat Oluştur

`bin\Release\` klasöründe `discord_config.dat` dosyası oluştur:

```
123456789012345678
```

**Discord ID nasıl bulunur?**
1. Discord Settings → Advanced → Developer Mode ON
2. Profiline sağ tıkla → Copy User ID

### 5. Auth Server'ı Başlat

```bash
cd "c:\Users\hardo\Desktop\quantix\Ryzex Auth"
npm install
npm run dev
```

### 6. Kullanıcı Ekle

1. http://localhost:5173 → Admin panel
2. Register (console'dan bootstrap key)
3. Management → License Key
4. Discord ID + Plan seç → Add Member

### 7. Run Premium Menu

```bash
# bin\Release\ klasöründe
premium_menu.exe
```

## ⚙️ Ayarlar

### Auth Server URL (Production)

`cheat\Main\Auth.cpp` dosyasında:

```cpp
// Line 15
static const std::string AUTH_SERVER = "http://YOUR_VDS_IP:3001";
```

**Dev:**
```cpp
static const std::string AUTH_SERVER = "http://localhost:3001";
```

**Production:**
```cpp
static const std::string AUTH_SERVER = "http://185.123.45.67:3001";
```

### SSL Certificate (Production)

`cheat\Main\Auth.cpp` dosyasında:

```cpp
// Line 138 (ValidateAuth fonksiyonunda)
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Production: 1L, Dev: 0L
```

## 📁 Gerekli Dosyalar

```
bin\Release\
├── premium_menu.exe
├── discord_config.dat          ← GEREKLI (Discord ID)
└── libcurl.dll                 ← GEREKLI (vcpkg'den kopyala)
```

**libcurl.dll nerede?**
```
C:\vcpkg\installed\x64-windows\bin\libcurl.dll
```

## 🔍 Troubleshooting

### ❌ "libcurl.dll not found"

**Çözüm:**
```bash
# libcurl.dll'i exe yanına kopyala
copy C:\vcpkg\installed\x64-windows\bin\libcurl.dll bin\Release\
```

### ❌ "Unresolved external symbol curl_easy_init"

**Çözüm:**
- Linker → Input → Additional Dependencies'e `libcurl.lib` eklenmiş mi kontrol et
- Library path doğru mu kontrol et

### ❌ "Discord ID not found"

**Çözüm:**
```bash
# bin\Release\ klasöründe discord_config.dat oluştur
echo 123456789012345678 > discord_config.dat
```

### ❌ "Failed to connect to auth server"

**Çözüm:**
1. Auth server çalışıyor mu?
   ```bash
   curl http://localhost:3001/api/health
   ```
2. Firewall/Antivirus engelliyor mu?
3. Auth.cpp'deki URL doğru mu?

### ❌ "HWID mismatch"

**Çözüm:**
- Admin panel → Management → Users
- Kullanıcı → Reset HWID
- Premium menu'yu tekrar çalıştır

## 📋 Build Checklist

Derleme öncesi kontrol et:

- [ ] vcpkg kurulu
- [ ] curl:x64-windows yüklü
- [ ] Visual Studio 2022 kurulu
- [ ] Platform Toolset v143
- [ ] Configuration: Release
- [ ] Platform: x64
- [ ] Auth.cpp ve Auth.hpp mevcut
- [ ] vcxproj güncel (Auth dosyaları ekli)
- [ ] discord_config.dat hazır
- [ ] Auth server çalışıyor

## 🎯 Test

### Başarılı Giriş
1. Premium menu çalıştır
2. MessageBox: "Welcome, Username!"
3. Program açılır

### Başarısız Giriş
1. Premium menu çalıştır
2. MessageBox: "Authentication failed"
3. Program kapanır

### Runtime Ban Test
1. Program çalışırken admin panelden ban yap
2. 60 saniye bekle (heartbeat)
3. Program otomatik kapanır

## 📖 Detaylı Dokümantasyon

- **Tam Entegrasyon:** `AUTH_INTEGRATION.md`
- **Test Guide:** `..\QUICK_TEST.md`
- **Özet:** `..\INTEGRATION_SUMMARY.md`

## 🆘 Destek

Sorun yaşarsan:
1. Log dosyalarını kontrol et: `cheat_debug.log`
2. Auth server logs kontrol et
3. Security → Activity Log kontrol et (admin panel)

---

**Başarılar! 🚀**
