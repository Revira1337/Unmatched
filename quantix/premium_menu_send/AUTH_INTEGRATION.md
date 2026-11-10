# Premium Menu - Auth System Integration

Bu dokümantasyon, Quantix Auth sisteminin Premium Menu ile entegrasyonunu açıklar.

## Özellikler

✅ **HWID Doğrulama** - Donanım tabanlı benzersiz kimlik kontrolü
✅ **Discord ID Kontrolü** - Kayıtlı Discord hesabı zorunluluğu  
✅ **Real-time Validation** - Anlık lisans durumu kontrolü
✅ **Heartbeat System** - 60 saniyede bir otomatik kontrol
✅ **Freeze Key Support** - Dondurulan keyler giriş yapamaz
✅ **Ban System** - Banlanan kullanıcılar engellenır
✅ **Expiry Check** - Süresi dolan keyler otomatik reddedilir
✅ **Admin Panel Integration** - Web üzerinden HWID reset

## Nasıl Çalışır?

### 1. Başlangıç (Program açıldığında)

```
Program Başlatılıyor
    ↓
HWID Hesaplanıyor (CPU + MAC + Motherboard)
    ↓
Discord ID Alınıyor (discord_config.dat dosyasından)
    ↓
Auth Server'a Doğrulama İsteği (/api/hwid/validate)
    ↓
Sunucu Kontrolleri:
  - Discord ID kayıtlı mı?
  - Kullanıcı banned mı?
  - Key frozen mı?
  - Key expired mı?
  - HWID eşleşiyor mu?
    ↓
✅ Başarılı → Program açılır + Heartbeat başlar
❌ Başarısız → Hata mesajı + Program kapanır
```

### 2. Heartbeat (Her 60 saniyede)

Program çalışırken arka planda her 60 saniyede bir auth server'a ping atar:

```
Heartbeat İsteği (/api/hwid/heartbeat)
    ↓
Sunucu Kontrolleri:
  - Session hala geçerli mi?
  - Ban durumu değişti mi?
  - Freeze durumu değişti mi?
  - Expire oldu mu?
    ↓
✅ Devam → Program çalışmaya devam eder
❌ Geçersiz → Program otomatik kapanır
```

## Kurulum

### Server Tarafı (Auth System)

1. **Server URL'ini Ayarlayın**

`premium_menu_send\cheat\Main\Auth.cpp` dosyasında:

```cpp
// Auth server URL (VDS'nizdeki server adresi)
static const std::string AUTH_SERVER = "http://your-vds-ip:3001";
```

Örnek:
```cpp
static const std::string AUTH_SERVER = "http://185.123.45.67:3001";
```

2. **Auth Server'ı Başlatın**

```bash
cd "c:\Users\hardo\Desktop\quantix\Ryzex Auth"
npm run dev
```

Server şu adreste çalışacak: `http://localhost:3001`

### Client Tarafı (Premium Menu)

1. **libcurl Kütüphanesini Ekleyin**

Premium menu'nün HTTP istekleri yapabilmesi için `libcurl` gerekli.

**Visual Studio'da:**
- Project Properties → C/C++ → General → Additional Include Directories
  - `C:\vcpkg\installed\x64-windows\include` (veya libcurl kurulu olduğu dizin)
  
- Project Properties → Linker → General → Additional Library Directories
  - `C:\vcpkg\installed\x64-windows\lib`
  
- Project Properties → Linker → Input → Additional Dependencies
  - `libcurl.lib` ekleyin

**vcpkg ile kurulum:**
```bash
vcpkg install curl:x64-windows
```

2. **Projeyi Derleyin**

Visual Studio'da `premium_menu.sln`'i açın ve Build edin.

3. **Discord Config Dosyası Oluşturun**

Premium menu .exe dosyasının yanına `discord_config.dat` dosyası oluşturun:

```
123456789012345678
```

İçine Discord ID'nizi yazın (18-20 haneli sayı).

**Discord ID nasıl bulunur?**
- Discord'da Settings → Advanced → Developer Mode'u açın
- Profilinize sağ tıklayın → "Copy User ID"

## Admin Panel Kullanımı

### Kullanıcı Ekleme

1. Admin panele giriş yapın: `http://localhost:5173` (veya VDS IP)
2. **Management → License Key** sayfasına gidin
3. Bilgileri doldurun:
   - **Discord ID**: Kullanıcının Discord ID'si
   - **UUID/HWID**: Boş bırakın (ilk girişte otomatik set edilecek)
   - **Plan**: month / 3months / year / lifetime seçin
4. **Add Member** butonuna tıklayın

### HWID Reset (Bilgisayar Değişimi)

Kullanıcı farklı bir bilgisayarda giriş yapmak istiyorsa:

1. **Management → Users** sayfasına gidin
2. Kullanıcıya tıklayın
3. **Reset HWID** butonuna tıklayın
4. Kullanıcı yeni bilgisayardan giriş yaptığında yeni HWID otomatik kaydedilecek

### Freeze (Süre Dondurma)

Key süresini durdurmak için:

1. **Management → Users** sayfasına gidin  
2. Kullanıcıya tıklayın
3. **Freeze** butonuna tıklayın

Frozen key'ler giriş yapamaz ve süreleri işlemez.

### Ban

1. **Management → Users** sayfasına gidin
2. Kullanıcıya tıklayın  
3. Ban reason yazın (opsiyonel)
4. **Ban** butonuna tıklayın

Banlı kullanıcılar giriş yapamaz.

## API Endpoints

### POST /api/hwid/validate
HWID ve Discord ID doğrulama

**Request:**
```json
{
  "hwid": "abc123def456",
  "discord_id": "123456789012345678"
}
```

**Response (Success):**
```json
{
  "success": true,
  "message": "Authentication successful",
  "user": {
    "discord_id": "123456789012345678",
    "discord_username": "Username",
    "plan": "Month",
    "expires_at": "2026-08-11 10:30:00",
    "remaining": "30 days, 5 hours",
    "remaining_ms": 2628000000,
    "status": "active"
  }
}
```

**Response (Error):**
```json
{
  "success": false,
  "error": "Your account is banned"
}
```

### POST /api/hwid/heartbeat
Session durumu kontrolü (her 60 saniye)

**Request:**
```json
{
  "hwid": "abc123def456",
  "discord_id": "123456789012345678"
}
```

**Response:**
```json
{
  "success": true,
  "remaining_ms": 2628000000,
  "status": "active"
}
```

### POST /api/hwid/reset/:id (Admin Only)
HWID sıfırlama

**Headers:**
```
Authorization: Bearer <admin_token>
```

**Response:**
```json
{
  "success": true,
  "message": "HWID reset successfully"
}
```

## Hata Mesajları

| Hata | Açıklama | Çözüm |
|------|----------|-------|
| `No active license found for this Discord ID` | Discord ID sistemde kayıtlı değil | Admin panelden kullanıcı ekleyin |
| `Your account is banned` | Kullanıcı banlanmış | Admin panelden ban kaldırın |
| `Your license is currently frozen` | Key dondurulmuş | Admin panelden unfreeze yapın |
| `Your license has expired` | Key süresi dolmuş | Admin panelden süre ekleyin |
| `HWID mismatch` | Farklı bilgisayar | Admin panelden HWID reset yapın |
| `Failed to connect to auth server` | Server'a ulaşılamıyor | Server çalışıyor mu kontrol edin |

## Security Logging

Tüm auth işlemleri Security Log'a kaydedilir:

- ✅ Başarılı girişler (success)
- ⚠️ Başarısız giriş denemeleri (warning)
- 🚫 Ban/Freeze durumunda giriş denemeleri (danger)
- 🔄 HWID reset işlemleri (info)
- 🔄 HWID ilk kayıt (success)

**Admin panelde görüntüleme:**
Security → Activity Log sayfasından tüm logları görüntüleyebilirsiniz.

## Geliştirme ve Test

### Test için Local Setup

1. Auth server'ı local'de çalıştırın:
```bash
cd "c:\Users\hardo\Desktop\quantix\Ryzex Auth"
npm run dev
```

2. Auth.cpp'de local server kullanın:
```cpp
static const std::string AUTH_SERVER = "http://localhost:3001";
```

3. Test Discord ID oluşturun:
```bash
# Admin panelde Management → License Key
Discord ID: Kendi Discord ID'niz
Plan: lifetime
```

4. Premium menu'yu çalıştırın

### Production Deployment

1. VDS'ye auth sistemini kurun
2. Auth.cpp'de production URL'i ayarlayın:
```cpp
static const std::string AUTH_SERVER = "http://your-vds-ip:3001";
```

3. SSL sertifikası ekleyin (production'da önemli):
```cpp
curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
```

4. Firewall ayarlarını yapın (Port 3001 açık olmalı)

## İleride Eklenebilecek Özellikler

- [ ] ImGui tabanlı Discord ID input dialog
- [ ] Otomatik HWID değişiklik bildirimi (Discord webhook)
- [ ] Multi-HWID support (2-3 bilgisayar)
- [ ] Session replay koruması
- [ ] Coğrafi konum bazlı kısıtlamalar
- [ ] 2FA (Two-Factor Auth) ekstra güvenlik katmanı
- [ ] License key bazlı aktivasyon (Discord ID yerine)

## Destek

Sorularınız için:
- GitHub Issues
- Discord support server
- Admin panel → Staff Chat

---

**© 2026 Quantix - Premium Menu Auth System**
