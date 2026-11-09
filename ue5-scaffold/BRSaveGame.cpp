// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BRSaveGame.cpp
// ملاحظة: نصوص UE_LOG بالإنجليزية عمداً — بعض إعدادات MSVC تُفسد
// المحارف غير اللاتينية داخل السلاسل النصية. التعليقات فقط هي بالعربية.
// =============================================================================

#include "BRSaveGame.h"

#include "Kismet/GameplayStatics.h"

const FString UBRSaveGame::SlotName = TEXT("BaranRamoSave");
const int32 UBRSaveGame::UserIndex = 0;
const int32 UBRSaveGame::CurrentSaveVersion = 1;

// -----------------------------------------------------------------------------
// Load / Save
// -----------------------------------------------------------------------------

UBRSaveGame* UBRSaveGame::LoadOrCreate(UObject* WorldContextObject, bool& bOutWasLoaded)
{
	bOutWasLoaded = false;

	// WorldContext غير مستخدم في الحساب، لكنه ضروري ليظهر المنفذ في Blueprint
	// ويُستخدم للتحقق من أن الاستدعاء جاء من عالم صالح.
	if (!IsValid(WorldContextObject))
	{
		UE_LOG(LogTemp, Error, TEXT("[BaranRamo] LoadOrCreate called with an invalid world context."));
		return nullptr;
	}

	if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		if (UBRSaveGame* Loaded = Cast<UBRSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex)))
		{
			if (Loaded->SaveVersion > CurrentSaveVersion)
			{
				// الملف أُنشئ بنسخة أحدث من الكود: قد تكون هناك حقول لا نعرفها.
				// نُكمل التحميل لكن نسجّل التحذير لتشخيص أي سلوك غريب لاحقاً.
				UE_LOG(LogTemp, Warning,
					TEXT("[BaranRamo] Save version %d is newer than supported %d. Loading anyway."),
					Loaded->SaveVersion, CurrentSaveVersion);
			}

			bOutWasLoaded = true;
			UE_LOG(LogTemp, Log, TEXT("[BaranRamo] Save '%s' loaded (%d property records)."),
				*GetSaveFileName(), Loaded->Properties.Num());
			return Loaded;
		}

		// الملف موجود لكنه تالف أو من صنف SaveGame آخر: نبدأ من جديد بدل الانهيار.
		UE_LOG(LogTemp, Warning, TEXT("[BaranRamo] Save file exists but could not be read. Starting a new game."));
	}

	UBRSaveGame* NewSave = Cast<UBRSaveGame>(UGameplayStatics::CreateSaveGameObject(UBRSaveGame::StaticClass()));
	if (NewSave)
	{
		NewSave->ResetToDefault();
		UE_LOG(LogTemp, Log, TEXT("[BaranRamo] New save object created."));
	}

	return NewSave;
}

bool UBRSaveGame::SaveToDisk()
{
	SaveVersion = CurrentSaveVersion;
	SavedAtUtc = FDateTime::UtcNow();

	const bool bSaved = UGameplayStatics::SaveGameToSlot(this, SlotName, UserIndex);

	UE_LOG(LogTemp, Log, TEXT("[BaranRamo] SaveToDisk('%s') -> %s (%d records)"),
		*GetSaveFileName(), bSaved ? TEXT("OK") : TEXT("FAILED"), Properties.Num());

	return bSaved;
}

bool UBRSaveGame::DoesSaveExist(UObject* WorldContextObject)
{
	return IsValid(WorldContextObject) && UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex);
}

bool UBRSaveGame::DeleteSave(UObject* WorldContextObject)
{
	if (!IsValid(WorldContextObject))
	{
		return false;
	}

	return UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
}

FString UBRSaveGame::GetSaveFileName()
{
	// UGameplayStatics يضيف .sav تلقائياً على القرص.
	return SlotName + TEXT(".sav");
}

// -----------------------------------------------------------------------------
// Defaults
// -----------------------------------------------------------------------------

void UBRSaveGame::ResetToDefault()
{
	SaveVersion = CurrentSaveVersion;
	SavedAtUtc = FDateTime(0);
	PlayerStats = FBRPlayerStatsRecord();
	Properties.Reset();

	// حالة بداية اللعبة: 500 عملة، صحة كاملة، اليوم الأول، الساعة 08:00.
	PlayerStats.Money = 500;
	PlayerStats.Health = 100.f;
	PlayerStats.DayNumber = 1;
	PlayerStats.TimeOfDayHours = 8.f;
}

// -----------------------------------------------------------------------------
// Property records
// -----------------------------------------------------------------------------

void UBRSaveGame::AddOrUpdateProperty(const FBRPropertyRecord& Record)
{
	if (Record.InstanceId.IsNone())
	{
		// بدون معرّف فريد لا يمكن التمييز بين نسختين من نفس النوع.
		UE_LOG(LogTemp, Warning, TEXT("[BaranRamo] AddOrUpdateProperty ignored: InstanceId is None."));
		return;
	}

	for (FBRPropertyRecord& Existing : Properties)
	{
		if (Existing.InstanceId == Record.InstanceId)
		{
			Existing = Record;
			return;
		}
	}

	Properties.Add(Record);
}

bool UBRSaveGame::FindProperty(FName InstanceId, FBRPropertyRecord& OutRecord) const
{
	if (InstanceId.IsNone())
	{
		return false;
	}

	for (const FBRPropertyRecord& Existing : Properties)
	{
		if (Existing.InstanceId == InstanceId)
		{
			OutRecord = Existing;
			return true;
		}
	}

	return false;
}
