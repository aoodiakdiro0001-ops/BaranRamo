// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BaranRamoGameMode.cpp
// =============================================================================

#include "BaranRamoGameMode.h"

#include "BRPlayerCharacter.h"
#include "BRPlayerState.h"
#include "Engine/World.h"
#include "GameFramework/GameSession.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

ABaranRamoGameMode::ABaranRamoGameMode()
{
	// --- الأصناف الافتراضية ---
	// C++ classes هنا، ثم تُستبدل في BP_BaranRamoGameMode بالنسخ المشتقة
	// (BP_BRPlayerCharacter, BP_BRPlayerState) لأن الـ Meshes والـ Input
	// لا يمكن ضبطها من C++.
	DefaultPawnClass = ABRPlayerCharacter::StaticClass();
	PlayerStateClass = ABRPlayerState::StaticClass();
	PlayerControllerClass = APlayerController::StaticClass();
	GameSessionClass = AGameSession::StaticClass();

	// --- إعدادات الجلسة ---
	// الانتقال بين الخرائط بدون قطع الاتصال (مهم للـ co-op).
	bUseSeamlessTravel = true;

	// نُنشئ Pawn فوراً عند الدخول بدل تركه متفرجاً.
	bStartPlayersAsSpectators = false;

	// لعبة تعاونية مستمرة: الإيقاف المؤقت يعطّل السيرفر للجميع.
	bPauseable = false;
}

void ABaranRamoGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// GetNetMode بديل آمن عن IsRunningDedicatedServer() ويوضّح نوع الجلسة في اللوج.
	const TCHAR* NetModeLabel = TEXT("Standalone");
	switch (GetNetMode())
	{
	case NM_DedicatedServer: NetModeLabel = TEXT("DedicatedServer"); break;
	case NM_ListenServer:    NetModeLabel = TEXT("ListenServer");    break;
	case NM_Client:          NetModeLabel = TEXT("Client");          break;
	default: break;
	}

	UE_LOG(LogTemp, Log, TEXT("[BaranRamo] InitGame | Map=%s | NetMode=%s | DayLength=%.0fs"),
		*MapName, NetModeLabel, DayLengthSeconds);
}

void ABaranRamoGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	// في هذه المرحلة يكون PlayerState قد أُنشئ وربط بالـ Controller.
	if (ABRPlayerState* PS = NewPlayer->GetPlayerState<ABRPlayerState>())
	{
		// السيرفر وحده يملك صلاحية تعديل النقود.
		PS->SetInitialMoney(StartingMoney);

		UE_LOG(LogTemp, Log, TEXT("[BaranRamo] %s joined. Players online: %d"),
			*PS->PlayerDisplayName, GetNumPlayers());
	}
}

void ABaranRamoGameMode::Logout(AController* Exiting)
{
	if (const APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		if (const ABRPlayerState* PS = PC->GetPlayerState<ABRPlayerState>())
		{
			UE_LOG(LogTemp, Log, TEXT("[BaranRamo] %s left."), *PS->PlayerDisplayName);
		}
	}

	Super::Logout(Exiting);
}

AActor* ABaranRamoGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
	if (Starts.Num() == 0)
	{
		// لا يوجد PlayerStart في الخريطة: نترك المحرك يقرر (قد يفشل بالظهور).
		return Super::ChoosePlayerStart_Implementation(Player);
	}

	// نجمع مواقع اللاعبين الحاليين لتجنّب ظهور اللاعب الجديد فوق أحدهم.
	TArray<FVector> OccupiedLocations;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (const APlayerController* PC = It->Get())
		{
			if (const APawn* Pawn = PC->GetPawn())
			{
				OccupiedLocations.Add(Pawn->GetActorLocation());
			}
		}
	}

	// نختار نقطة البداية الأبعد عن أقرب لاعب (farthest-point heuristic).
	AActor* BestStart = Starts[0];
	float BestScore = -1.f;

	for (AActor* Start : Starts)
	{
		float NearestDistanceSq = TNumericLimits<float>::Max();
		for (const FVector& Location : OccupiedLocations)
		{
			NearestDistanceSq = FMath::Min(NearestDistanceSq,
				FVector::DistSquared(Start->GetActorLocation(), Location));
		}

		if (NearestDistanceSq > BestScore)
		{
			BestScore = NearestDistanceSq;
			BestStart = Start;
		}
	}

	return BestStart;
}
