// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BaranRamoGameMode.h
// GameMode هو "قوانين اللعبة" — وُجوده على السيرفر فقط (Server Only).
// العملاء لا يملكون نسخة منه؛ ما يرونه هو اللاعبون الذين ينشئهم.
//
// مسؤولياته:
//   1. تحديد الأصناف الافتراضية (Pawn / PlayerState / PlayerController).
//   2. استقبال اللاعبين عند الاتصال (PostLogin) وتهيئة حالتهم.
//   3. الاحتفاظ بقواعد المباراة (المال الابتدائي، طول اليوم).
//
// في المشروع: Baseه ABaranRamoGameMode ثم اشتقّ منه WBP/BPC_BaranRamoGameMode
// لضبط StartingMoney وDayLengthSeconds من الـ Blueprint.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaranRamoGameMode.generated.h"

class APlayerController;
class ABRPlayerCharacter;
class ABRPlayerState;

/**
 * GameMode تعاوني بلا شروط فوز — اللعبة مستمرة (sandbox survival).
 * يرث من AGameModeBase لأننا لا نحتاج "Match State" ولا Round restart.
 */
UCLASS(Blueprintable, Config = Game)
class BARANRAMO_API ABaranRamoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaranRamoGameMode();

	/** يُستدعى مرة واحدة عند تحميل الخريطة على السيرفر — لإعداد قواعد الجلسة. */
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	/** يُستدعى عند انضمام لاعب جديد (بعد إنشاء PlayerController وPlayerState). */
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/** يُستدعى عند خروج لاعب — لإسقاط أدواته أو إبلاغ الآخرين. */
	virtual void Logout(AController* Exiting) override;

	// -------------------------------------------------------------------------
	// قواعد اللعبة (تُضبط من Blueprint)
	// -------------------------------------------------------------------------

	/** المال الذي يبدأ به كل لاعب جديد. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Rules", meta = (ClampMin = "0"))
	int32 StartingMoney = 500;

	/** طول اليوم الكامل بالثواني (600 = 10 دقائق لعب). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Rules", meta = (ClampMin = "60.0"))
	float DayLengthSeconds = 600.f;

protected:
	/**
	 * نقطة إنشاء اللاعب. نستعمل PlayerStart الافتراضي، لكننا نتأكد أننا
	 * لا نضع لاعبين فوق بعضهم (مهم في الـ co-op).
	 */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
};
