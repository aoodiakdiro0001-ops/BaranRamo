# BaranRamo — خارطة الطريق التقنية (Tech Roadmap)

> **المشروع:** BaranRamo — لعبة مزرعة وبقاء تعاونية (Co-op Farming & Survival)
> **المالك والمخرج:** Baran Avisat · **الميزانية:** 0$ · **القيد:** لا برامج مدفوعة، لا خوادم، لا اشتراكات
> **النسخة:** 1.0 · **التاريخ:** 8 نوفمبر 2026 · **الحالة:** معتمدة للتنفيذ

---

## 0) الملخص في 60 ثانية

| السؤال | الجواب |
|---|---|
| ما المُسلَّم اليوم؟ | نسخة ويب **HTML5** تعمل على أي هاتف فوراً — نقطة الدخول: `game/BaranRamo.html` |
| ما المحرك التالي؟ | **Godot 4.x** — رخصة MIT، مجاني، يبني Android APK و iOS |
| متى UE5؟ | المرحلة 3، على جهاز **Windows** فقط — وليس على macOS |
| كيف يلعب الأصدقاء؟ | رابط itch.io (ويب، صفر تثبيت) أو ملف APK أندرويد مباشر |
| كم يكلّف حتى أول إصدار؟ | **0$** (itch.io مجاني + APK بلا رسوم) |

---

## أ) الواقع التقني المصحح (اقرأ هذا أولاً)

### أ-1. Unreal Engine 5 على macOS — غير صالح كمسار إنتاج لهذا المشروع

الخطة الأصلية كانت: «نطوّر على ماك بـ UE5 ونبني اللعبة». هذه الخطة **غير قابلة للتنفيذ** في وضعنا الحالي، والسبب عملي وليس ذوقياً:

| العائق | التفصيل | الأثر على المشروع |
|---|---|---|
| المحرر على macOS مسار ثانوي | المحرر يُنشر لـ macOS، لكن الأدوات والموثوقية تُختبر أولاً على Windows: إضافات كثيرة (Marketplace/Fab) وWindows-only، وسلوك Metal يختلف عن D3D12 | أعطال وقت الضغط، وخصائص لا تعمل |
| لا بناء لـ Windows من macOS | التغليف لـ Windows يحتاج سلسلة أدوات Windows (cross-compile غير مدعوم رسمياً) | تُخرج نسخة ماك/iOS فقط، ولا تستطيع خدمة أغلب اللاعبين |
| متطلبات العتاد | UE5 + مشروع مفتوح يحتاج RAM 32GB+ وSSD سريع وGPU قوي | على ماك بمواصفات متوسطة = تجربة تحرير مؤلمة |
| منحنى التعلّم | C++ + Blueprints + Replication + Lighting | أسابيع قبل أول «شيء يلعب» |
| الميزانية | 0$ | لا يمكن شراء أصول ولا عتاد ولا وقت |

> **دقّة المصطلح (للمصداقية):** الشائع أن «Epic أوقفت دعم Mac كمنصة تطوير» — هذا غير دقيق. المحرر متاح فعلاً لـ macOS (Intel وApple Silicon)، وبناء iOS من ماك مدعوم. الصحيح هو أن macOS **ليس مسار الإنتاج المكافئ** لـ Windows، وفجواته تظهر بالضبط في المشاريع الكبيرة. الخلاصة التشغيلية لا تتغيّر: **لا تبنِ خطة المشروع على UE5 على ماك.**

**القرار:** UE5 يبقى هدف المرحلة 3، وينفَّذ على جهاز Windows (ملك، أو إعارة، أو سحابة). حتى ذلك الحين: HTML5 ثم Godot.

### أ-2. ملف iOS (`.ipa`) — لا يمكن تسليمه للأصدقاء مجاناً)

| الطريقة | التكلفة | القيود الحقيقية |
|---|---|---|
| Apple ID مجاني + Xcode | 0$ | التطبيق يعمل **7 أيام فقط** ثم يتوقف؛ حد **3 أجهزة** مسجّلة؛ التثبيت **عبر Xcode فقط** (كابل/شبكة) — لا رابط، لا TestFlight، لا مشاركة |
| Apple Developer Program | **99$/سنة** | TestFlight: 100 مستخدم خارجي عبر رابط، 90 يوماً لكل بناء |
| متجر App Store | 99$/سنة + مراجعة | ليس خياراً بميزانية صفر |

**النتيجة:** iOS غير قابل للتوزيع المجاني. أي وعد لصديق بـ«تطبيق آيفون» سيكسر بعد أسبوع.

### أ-3. Android — القناة المجانية الحقيقية

| الطريقة | التكلفة | الملاحظات |
|---|---|---|
| APK جانبي (side-load) | **0$** | ترسل الملف على واتساب/تيليجرام، يفعّل «تثبيت من مصادر غير معروفة»، ينتهي. لا متجر، لا توقيع مدفوع (debug keystore يكفي) |
| Google Play | 25$ مرة واحدة | اختياري لاحقاً؛ حسابات شخصية جديدة تفرض 12 مختبراً لمدة 14 يوماً قبل النشر العام، وملف `.aab` |

### أ-4. جدول قرار المنصات

| المنصة | قناة التوزيع | التكلفة | الجاهزية |
|---|---|---|---|
| أي هاتف/حاسوب | الويب (itch.io) | 0$ | ✅ فوري |
| Android | APK مباشر | 0$ | ✅ المرحلة 1 |
| iOS | Xcode (7 أيام) | 0$ | ⚠️ للاختبار الشخصي فقط |
| iOS | TestFlight | 99$/سنة | ⏸ مؤجّل |
| Windows | itch.io | 0$ | ⏸ المرحلة 3 |

---

## ب) البدائل المجانية 100% (مرتبة بالأولوية)

### ب-1. نسخة الويب HTML5 — الأصل الاستراتيجي

- **ما هي:** لعبة تعمل في المتصفح، لاعب واحد + لاعب ثاني على نفس الجهاز (same-screen).
- **لماذا أولاً:** صفر تثبيت، صفر توقيع، صفر متجر. رابط واحد يعمل على iPhone وAndroid وWindows.
- **النشر الدائم مجاناً على itch.io:**
  1. أنشئ حساباً على `itch.io` ثم `Dashboard → Create new project`.
  2. `Kind of project = HTML`، ارفع ملف ZIP يحتوي `index.html` في الجذر (لا مجلدات متداخلة).
  3. فعّل `This file will be played in the browser` · `Mobile friendly` · `Embed in page`.
  4. الحجم الموصى به < 20MB ليعمل على شبكات بطيئة.
  5. تحديثات لاحقة بدون متصفح: أداة `butler` المجانية
     ```bash
     butler push build/web baranavisat/baranramo:html5
     ```
- **الحدود التقنية:** صوت المتصفح يحتاج تفاعلاً أولياً (زر «ابدأ»)؛ الحفظ محلي عبر `localStorage`؛ لا co-op شبكي حقيقي.

### ب-2. Godot 4.x — المحرك الأساسي للمراحل 1-2

**الرخصة:** MIT — استخدام تجاري، بلا رسوم، بلا مشاركة أرباح. يعمل أصلياً على Apple Silicon.

**التحميل (بدون مدير حزم):**

```bash
# 1) نزّل "Godot Engine – macOS (Universal)" من godotengine.org/download/macos
# 2) فك الضغط وانقل Godot.app إلى /Applications
# 3) تحقّق
/Applications/Godot.app/Contents/MacOS/Godot --version
```

**Export Templates** (لا تصدير بدونها): من داخل المحرر
`Editor → Manage Export Templates → Download and Install`
المسار بعد التثبيت:
```
~/Library/Application Support/Godot/export_templates/4.x.stable/
```

**تجهيز Android (خطوات فعلية):**

```bash
# المتطلب: OpenJDK 17 (Godot 4 لا يعمل مع Java 8 ولا 21 في بعض الإصدارات)
java -version

# هيكل SDK
mkdir -p "$HOME/Library/Android/sdk/cmdline-tools"
# فك ضغط commandlinetools-mac-*.zip بحيث يصبح:
#   ~/Library/Android/sdk/cmdline-tools/latest/bin/sdkmanager

export ANDROID_HOME="$HOME/Library/Android/sdk"
export PATH="$ANDROID_HOME/cmdline-tools/latest/bin:$ANDROID_HOME/platform-tools:$PATH"

sdkmanager --licenses
sdkmanager "platform-tools" "platforms;android-34" "build-tools;34.0.0"
```

**مفتاح التوقيع (debug keystore):**

```bash
mkdir -p "$HOME/.android"
keytool -keyalg RSA -genkeypair -alias androiddebugkey \
  -keypass android -storepass android \
  -keystore "$HOME/.android/debug.keystore" \
  -dname "CN=Android Debug,O=Android,C=US" -validity 9999 -deststoretype pkcs12
```

**داخل المحرر:**
`Editor → Editor Settings → Export → Android` → اضبط `Android SDK Path = ~/Library/Android/sdk` و`Debug Keystore = ~/.android/debug.keystore` و`Java SDK Path` لمجلد JDK 17.

**Export Preset:** `Project → Export… → Add… → Android`
- `Package → Unique Name`: `com.baranavisat.baranramo`
- `Version → Code/Name`: `1` / `1.0.0`
- `Architectures`: `arm64-v8a` فقط (أصغر حجماً وأسرع؛ أضف `armeabi-v7a` فقط لأجهزة قديمة)
- `Keystore → Debug`: استخدم المفتاح أعلاه
- `Gradle Build`: مفعّل (أول بناء ينزّل التبعيات — يحتاج إنترنت مرة واحدة)

**One-Click Export:** وصّل الهاتف بـ USB، فعّل `USB debugging`، ثم من أعلى اليمين اضغط أيقونة أندرويد (One Click Deploy). أو من الطرفية:

```bash
/Applications/Godot.app/Contents/MacOS/Godot --headless --path . \
  --export-debug "Android" build/BaranRamo-debug.apk

adb install -r build/BaranRamo-debug.apk
```

**لمشاركة اللعبة مع صديق:** أرسل `BaranRamo-debug.apk` (لا يحتاج إنترنت ولا توقيع مدفوع). للنسخة النهائية استخدم `--export-release "Android"` مع keystore إصدار (يمكن إنشاؤه بنفس `keytool` باسم آخر) — لكن التوزيع الجانبي يقبل debug بلا مشاكل.

**مسار iOS:**

```bash
/Applications/Godot.app/Contents/MacOS/Godot --headless --path . \
  --export-debug "iOS" build/ios/
open build/ios/BaranRamo.xcodeproj
```
ثم في Xcode: `Signing & Capabilities → Team = Personal Team` → اختر الجهاز → `Run`.
تذكير: 7 أيام و3 أجهزة كحد أقصى بهذا المسار.

### ب-3. UE5 على Windows — لاحقاً فقط

الهدف: «العالم المفتوح الكامل» بميزة **Chaos Vehicles**. لا يُبدأ قبل أن تستقر نسخة Godot وتُثبت أن اللعبة ممتعة. جهاز ويندوز مطلوب (ملك، إعارة، أو جلسة سحابية مجانية محدودة).

---

## ج) خارطة الطريق على مراحل

### المرحلة 0 — مُسلَّمة: HTML5

| البند | الحالة |
|---|---|
| لاعب واحد + لاعب محلي (نفس الشاشة) | ✅ |
| حلقة لعب أساسية: زراعة، حصاد، بيع، شراء | ✅ |
| تعمل على الهاتف والمتصفح | ✅ |
| حفظ محلي | جزئي (`localStorage`) |

### المرحلة 1 — Godot 4 + Android (4-6 أسابيع)

1. **الأسبوع 1:** تثبيت Godot وتصدير مشروع فارغ إلى APK — إثبات أن السلسلة تعمل قبل كتابة أي منطق.
2. **الأسبوع 2:** نقل حلقة اللعب (Grid-based tilemap للزراعة، Inventory كـ Resource، UI بـ Control nodes).
3. **الأسبوع 3:** نظام الحفظ: `JSON` عبر `FileAccess` في `user://save_slot_1.json` (يعمل على Android/iOS/Desktop بلا تعديل).
4. **الأسبوع 4:** لاعبان على نفس الجهاز (جهاز تحكم + لوحة مفاتيح)، ثم اختبار APK على هاتف حقيقي.
5. **الأسبوع 5-6:** صقل (صوت، جزيئات، توازن الاقتصاد)، بناء APK نهائي، نشر صفحة itch.io.

**تعريف الإنجاز:** صديقان يلعبان 15 دقيقة على هاتفين مختلفين بلا crash، والحفظ يعمل بعد إغلاق التطبيق.

### المرحلة 2 — Co-op أونلاين + مركبات + حيوانات (6-8 أسابيع)

| النظام | التنفيذ المقترح (مجاني) |
|---|---|
| الاستضافة | لاعب يستضيف (listen server) — لا خوادم مدفوعة |
| الشبكة | **Godot ENet** (مدمج) أو **Epic Online Services** المجاني للـ NAT punch-through |
| المركبات | `VehicleBody3D` (مدمج في Godot 4 physics) |
| الحيوانات | `BehaviorTree` عبر إضافة `Beehave` (مفتوحة المصدر) أو FSM يدوية |
| المزامنة | `MultiplayerSynchronizer` + `MultiplayerSpawner` (مدمجان) |

### المرحلة 3 — UE5 على Windows (8-12 أسبوعاً)

هذا هو المخطط الأصلي، محفوظ للتنفيذ على ويندوز:

| النظام | المواصفة التقنية |
|---|---|
| **GameMode** | `BaranRamoGameMode` (Server Only)، `DefaultPawnClass`، `PlayerStateClass`، `bUseSeamlessTravel = true` |
| **Replication** | `PlayerState` بمتغيرات `Replicated` + `OnRep_*` لتحديث الـ UI؛ `Server RPC` لتعديل النقود |
| **التفاعل** | `BPI_Interactable` (Interface) + `Line Trace` بمسافة **250 سم** من الكاميرا |
| **الحمل** | `PhysicsHandleComponent` مع `GrabComponentAtLocationWithRotation` و`SetTargetLocationAndRotation` في `Tick` |
| **المباني** | `Grid Snapping` بخطوة 100 سم على المحاور X/Y، وتحقق من التداخل بـ `Box Overlap` |
| **المركبات** | **Chaos Vehicles** (`ChaosWheeledVehicleMovementComponent`) |
| **الحيوانات** | **Behavior Trees** + `Blackboard` (Hunger / Threat / HomeLocation) |
| **الاقتصاد** | `Sell Zone` (Trigger + `OnComponentBeginOverlap`) يقرأ الأسعار من `DT_ItemPrices` (DataTable) |
| **دورة اليوم** | تدوير `Directional Light` بمعدل **0.1°/ثانية**، وبثّ `CurrentTime` للعملاء كل 2 ثانية |
| **الحفظ** | `SaveGame` باسم `BaranRamoSave.sav` عبر `UGameplayStatics::SaveGameToSlot` |

### جدول المراحل

| المرحلة | المخرجات | المدة | الموارد |
|---|---|---|---|
| **0 — HTML5** | نسخة ويب قابلة للعب، لاعب + لاعب محلي | مُسلَّمة | 0$ — محرر نصوص + متصفح |
| **1 — Godot/Android** | APK يعمل على أندرويد، حفظ JSON، لاعبان محلياً | 4-6 أسابيع | 0$ — Godot (MIT) + JDK 17 + Android SDK |
| **2 — Co-op** | لاعبان أونلاين (ENet/EOS)، مركبة، حيوانات بـ BT | 6-8 أسابيع | 0$ — EOS مجاني، Beehave مفتوح |
| **3 — UE5** | عالم مفتوح، Chaos Vehicles، حفظ كامل | 8-12 أسبوعاً | جهاز Windows + UE5 (مجاني حتى 1M$ إيراد) |

### بوابات الانتقال (لا تنتقل قبل تحققها)

- **0→1:** 10 أشخاص لعبوا النسخة الويب وأكملوا دورة زراعة→بيع كاملة.
- **1→2:** APK مثبّت على 3 هواتف مختلفة، ومعدل crash = 0 في جلسة 20 دقيقة.
- **2→3:** 4 لاعبين في جلسة واحدة، وزمن استجابة < 150ms على شبكة منزلية.

---

## د) مصادر الموارد المجانية 100%

| المصدر | ماذا يُعطي | الرخصة | متى يُستخدم |
|---|---|---|---|
| **Quixel Megascans** | نسيج ومجسمات واقعية (صخور، تربة، نبات) | مجاني مع UE وفق Epic Content License — **تحقّق من شروط Fab قبل الاستخدام خارج UE** | المرحلة 3 (بيئات UE5) |
| **Adobe Mixamo** | شخصيات بشرية + مكتبة أنيميشن (مشي، جري، حمل، حصاد) | مجاني بحساب Adobe؛ استخدام تجاري مسموح، **يُمنع إعادة توزيع الأنيميشن كأصل مستقل** | المرحلة 1-3 (شخصيات اللاعب/العمال) |
| **Freesound.org** | مؤثرات صوتية ومحيط (رياح، طيور، خطوات، آلات) | **مختلط**: CC0 وCC-BY — فلتر بـ CC0 فقط لتفادي الإسناد | كل المراحل (صوت) |
| **Kenney.nl** | حزم 2D/3D كاملة، واجهات UI، أيقونات، أصوات | **CC0** (بلا شروط) | المرحلة 0-1 (أصول سريعة، UI، placeholders) |
| **OpenGameArt.org** | نسيج، شخصيات، موسيقى من المجتمع | **مختلط**: CC0 / CC-BY / GPL — اقرأ رخصة كل أصل على حدة | ملء الفجوات فقط |

**قاعدة العمل:** أي أصل يدخل المشروع يُسجَّل في `docs/ASSETS.md` بسطر واحد: الاسم، الرابط، الرخصة، تاريخ الإضافة. هذا يحمي المشروع قانونياً ويجعل النشر التجاري ممكناً لاحقاً.

---

## هـ) النشر والحقوق

| الوجهة | التكلفة | الجمهور | ملاحظة |
|---|---|---|---|
| **itch.io** | 0$ | عالمي، ويب + تحميل | الوجهة الأولى؛ ادعم «Pay what you want» الاختياري |
| **APK مباشر** | 0$ | أصدقاء وعائلة | أرسله على واتساب/تيليجرام |
| **Google Play** | 25$ مرة | أندرويد عام | لاحقاً؛ يحتاج 12 مختبراً/14 يوماً للحساب الشخصي الجديد |
| **TestFlight** | 99$/سنة | iOS | مؤجّل حتى يوجد إيراد |

### بطاقة الحقوق (تُلصق في صفحة itch.io وفي About داخل اللعبة)

```
BaranRamo
© 2026 Baran Avisat — All Rights Reserved.
Author, Designer & Director: Baran Avisat
Email: Baranavisat@gmail.com
WhatsApp: +90 531 326 37 22
GitHub: aoodiakdiro0001-ops

المحرك: Godot Engine (MIT License) — حقوق Godot محفوظة لأصحابها.
الأصول الخارجية: مسجّلة في docs/ASSETS.md مع رخصها الأصلية.
```

---

## و) المخاطر والافتراضات

| الخطر | الاحتمال | التخفيف |
|---|---|---|
| نقل النسخة إلى Godot يستغرق أطول من المتوقع | مرتفع | نقل الحلقة فقط أولاً (زراعة/بيع)، ثم الجماليات |
| Android SDK/JDK يتعطّل بعد تحديث النظام | متوسط | توثيق النسخ المثبّتة في `docs/toolchain-versions.md` |
| UE5 على ويندوز يتأخر (لا جهاز) | متوسط | المرحلتان 0-2 قائمتان بذاتهما ولا تعتمدان عليه |
| نضوب الحافز أثناء المرحلة 2 | مرتفع | إصدار علني كل أسبوعين على itch.io |

---

## ز) الخطوة التالية (هذا الأسبوع)

1. نشر نسخة HTML5 على itch.io والحصول على رابط دائم.
2. تثبيت Godot 4.x وتصدير مشروع فارغ إلى APK — **إثبات السلسلة قبل أي كود**.
3. إنشاء `docs/ASSETS.md` و`docs/toolchain-versions.md`.
4. بدء نقل نظام الزراعة إلى Godot (TileMap + Resource للبذور).

---

## ملحق: هيكل `ue5-scaffold/`

الملفات جاهزة للمرحلة 3، تُنقل إلى مشروع UE5 بالترتيب:

| الملف | مكانه في مشروع UE5 |
|---|---|
| `BaranRamo.Build.cs` | `Source/BaranRamo/BaranRamo.Build.cs` |
| `BaranRamoGameMode.h/.cpp` | `Source/BaranRamo/` |
| `BRPlayerState.h/.cpp` | `Source/BaranRamo/` |
| `BRPlayerCharacter.h/.cpp` | `Source/BaranRamo/` |
| `BRSaveGame.h/.cpp` | `Source/BaranRamo/` |
| `BPI_Interactable.h` | `Source/BaranRamo/` |

بعد النسخ: انقر بزر الفأرة الأيمن على ملف `.uproject` → `Generate Project Files` → ابنِ من Visual Studio. المتطلبات: UE 5.3+، Windows 10/11، Visual Studio 2022 مع `Game development with C++`.

---

*آخر تحديث: 8 نوفمبر 2026 · يُراجع هذا المستند في نهاية كل مرحلة.*
