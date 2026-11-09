// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BRSaveGame.h
// بنية الحفظ (Save Game) — تُكتب على القرص باسم  BaranRamoSave.sav
//
// أين يُحفظ الملف فعلياً؟
//   Windows : %LOCALAPPDATA%\BaranRamo\Saved\SaveGames\BaranRamoSave.sav
//   macOS   : ~/Library/Application Support/Epic/BaranRamo/Saved/SaveGames/
//   Android : /data/data/<package>/files/UE4Game/BaranRamo/Saved/SaveGames/
//
// لماذا USaveGame وليس JSON؟
//   UGameplayStatics يتولى الـ serialization والتحقق، ويعمل على كل المنصات.
//   كما أنه يحفظ USTRUCT وTArray مباشرة بلا كود تحويل.
//
// قاعدة ذهبية: لا تحفظ مؤشرات (Pointers) ولا Actors — احفظ بيانات فقط.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "BRSaveGame.generated.h"

/**
 * سجل مملوكة واحدة (محصول، بناء، صندوق، آلة، حيوان).
 * Transform يحفظ الموقع والدوران والحجم في قيمة واحدة.
 */
USTRUCT(BlueprintType)
struct FBRPropertyRecord
{
	GENERATED_BODY()

	/** معرّف فريد لهذه النسخة تحديداً (مثال: TomatoPlant_017). يُولَّد عند البناء. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FName InstanceId = NAME_None;

	/** نوع الأصل: اسم الـ Blueprint أو صف في DataTable (مثال: BP_TomatoPlant). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FName PropertyType = NAME_None;

	/** الموقع + الدوران + الحجم. ضروري لإعادة البناء في نفس المكان. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	FTransform Transform = FTransform::Identity;

	/** الصحة الحالية (0 = تالف/محصود، ويُحذف عند التحميل). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property", meta = (ClampMin = "0.0"))
	float Health = 100.f;

	/** الكمية داخل الصندوق، أو عدد الوحدات المنتَجة، أو كمية المحصول. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property", meta = (ClampMin = "0"))
	int32 Amount = 1;

	/** مرحلة النمو للمحاصيل (0 = بذرة، 3 = جاهز للحصاد). تُهمَل لغير المحاصيل. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property", meta = (ClampMin = "0", ClampMax = "3"))
	int32 GrowthStage = 0;

	/** هل يملكها اللاعب (true) أم مجرد شيء موضوع في العالم (false)؟ */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Property")
	bool bOwnedByPlayer = true;
};

/**
 * إحصائيات اللاعب المحفوظة (تُنسخ من ABRPlayerState عند الحفظ).
 */
USTRUCT(BlueprintType)
struct FBRPlayerStatsRecord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FString PlayerName = TEXT("Farmer");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0"))
	int32 Money = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0.0"))
	float Health = 100.f;

	/** الأسلحة المملوكة (أسماء صفوف DataTable مثل Sword_Basic). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TArray<FName> Weapons;

	/** عناصر الـ Inventory (بذور، محاصيل، أدوات). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	TArray<FName> InventoryItems;

	/** آخر موقع للاعب — للعودة لنفس المكان عند التحميل. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player")
	FTransform LastTransform = FTransform::Identity;

	/** اليوم الحالي في اللعبة (1 = أول يوم). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "1"))
	int32 DayNumber = 1;

	/** ساعة اليوم 0..24 — تُستخدم لاستعادة دورة الليل/النهار. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player", meta = (ClampMin = "0.0", ClampMax = "24.0"))
	float TimeOfDayHours = 8.f;
};

/**
 * كائن الحفظ الرئيسي. يُنشأ عبر LoadOrCreate ثم يُملأ ثم SaveToDisk.
 *
 * مثال من Blueprint:
 *   Load Or Create → (Was Loaded?) → املأ الحقول → Save To Disk
 */
UCLASS(BlueprintType)
class BARANRAMO_API UBRSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	/** اسم الـ Slot. UGameplayStatics يضيف الامتداد .sav تلقائياً. */
	static const FString SlotName;

	/** 0 = اللاعب الأول على هذا الجهاز. */
	static const int32 UserIndex;

	/** الإصدار الحالي للبنية — ارفعه عند تغيير الحقول لتتعامل مع الملفات القديمة. */
	static const int32 CurrentSaveVersion;

	/** رقم إصدار الملف المحفوظ. */
	UPROPERTY(BlueprintReadOnly, Category = "BaranRamo|Save")
	int32 SaveVersion = 1;

	/** وقت الحفظ (UTC) — لعرضه في قائمة «متابعة اللعب». */
	UPROPERTY(BlueprintReadOnly, Category = "BaranRamo|Save")
	FDateTime SavedAtUtc;

	/** إحصائيات اللاعب. */
	UPROPERTY(BlueprintReadWrite, Category = "BaranRamo|Save")
	FBRPlayerStatsRecord PlayerStats;

	/** كل الممتلكات في العالم. */
	UPROPERTY(BlueprintReadWrite, Category = "BaranRamo|Save")
	TArray<FBRPropertyRecord> Properties;

	// -------------------------------------------------------------------------
	// API
	// -------------------------------------------------------------------------

	/**
	 * يفتح ملف الحفظ إن وُجد، وإلا يُنشئ كائناً جديداً بقيم افتراضية.
	 *
	 * @param WorldContextObject  أي UObject في العالم (عادة Self من الـ GameMode).
	 * @param bOutWasLoaded       true إذا قُرئ من القرص، false إذا كان جديداً.
	 * @return الكائن دائماً (لا يعيد nullptr إلا عند فشل التخصيص).
	 *
	 * تحقق دائماً من bOutWasLoaded قبل تطبيق منطق «متابعة اللعب»،
	 * وإلا ستحمّل عالماً فارغاً فوق عالم جديد.
	 */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Save", meta = (WorldContext = "WorldContextObject"))
	static UBRSaveGame* LoadOrCreate(UObject* WorldContextObject, bool& bOutWasLoaded);

	/** يكتب الكائن إلى القرص ويحدّث SavedAtUtc. يعيد true عند النجاح. */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Save")
	bool SaveToDisk();

	/** يعيد كل الحقول إلى قيم البداية (لا يمسّ القرص). */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Save")
	void ResetToDefault();

	/** هل يوجد ملف محفوظ على هذا الجهاز؟ */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Save", meta = (WorldContext = "WorldContextObject"))
	static bool DoesSaveExist(UObject* WorldContextObject);

	/** يحذف ملف الحفظ (لعبة جديدة من الصفر). */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Save", meta = (WorldContext = "WorldContextObject"))
	static bool DeleteSave(UObject* WorldContextObject);

	/** اسم الملف الكامل كما يظهر على القرص: BaranRamoSave.sav */
	UFUNCTION(BlueprintPure, Category = "BaranRamo|Save")
	static FString GetSaveFileName();

	/** يضيف سجلاً جديداً أو يستبدل سجلاً بنفس InstanceId. */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Save")
	void AddOrUpdateProperty(const FBRPropertyRecord& Record);

	/** يبحث عن سجل بـ InstanceId. يعيد false إن لم يوجد. */
	UFUNCTION(BlueprintPure, Category = "BaranRamo|Save")
	bool FindProperty(FName InstanceId, FBRPropertyRecord& OutRecord) const;
};
