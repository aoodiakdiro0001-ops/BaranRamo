// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BRPlayerState.cpp
// =============================================================================

#include "BRPlayerState.h"

#include "Net/UnrealNetwork.h"

ABRPlayerState::ABRPlayerState()
{
	// APlayerState يُفعّل التكرار (bReplicates) تلقائياً؛ نضبط القيم الابتدائية فقط.
	PlayerHealth = 100.f;
	MaxHealth = 100.f;
	CurrentMoney = 0;
	LastKnownMoney = 0;
}

// -----------------------------------------------------------------------------
// Replication
// -----------------------------------------------------------------------------

void ABRPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// لا تنسَ Super وإلا فقدت خصائص APlayerState الأصلية (PlayerName, Ping, ...).
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// COND_None = يُرسَل لكل من يرى هذا الـ Actor، عند كل تغيير.
	DOREPLIFETIME(ABRPlayerState, PlayerHealth);
	DOREPLIFETIME(ABRPlayerState, CurrentMoney);
	DOREPLIFETIME(ABRPlayerState, PlayerDisplayName);
}

// -----------------------------------------------------------------------------
// Initialisation (server only)
// -----------------------------------------------------------------------------

void ABRPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);

	// ClientInitialize يُستدعى على السيرفر بعد ربط الـ PlayerController.
	if (HasAuthority())
	{
		// نأخذ الاسم الذي ضبطه المحرك/المنصة (Steam, EOS, أو Name في الـ URL).
		const FString EngineName = GetPlayerName();
		PlayerDisplayName = EngineName.IsEmpty() ? TEXT("Farmer") : EngineName;
	}
}

// -----------------------------------------------------------------------------
// Economy
// -----------------------------------------------------------------------------

void ABRPlayerState::SetInitialMoney(int32 NewMoney)
{
	if (!HasAuthority())
	{
		return;
	}

	const int32 OldMoney = CurrentMoney;
	CurrentMoney = FMath::Max(0, NewMoney);
	LastKnownMoney = CurrentMoney;

	// على السيرفر المستضيف (Listen Server) لا يصل OnRep، لذا نحدّث الواجهة يدوياً.
	if (CurrentMoney != OldMoney)
	{
		const int32 Delta = CurrentMoney - OldMoney;
		OnMoneyChanged.Broadcast(CurrentMoney, Delta);
		BP_OnMoneyChanged(CurrentMoney, Delta);
	}
}

void ABRPlayerState::ApplyMoneyDelta(int32 Delta)
{
	// حماية مزدوجة: هذه الدالة تعمل على السيرفر فقط.
	if (!HasAuthority() || Delta == 0)
	{
		return;
	}

	const int32 OldMoney = CurrentMoney;
	CurrentMoney = FMath::Clamp<int32>(CurrentMoney + Delta, 0, MAX_int32);
	LastKnownMoney = CurrentMoney;

	if (CurrentMoney != OldMoney)
	{
		const int32 AppliedDelta = CurrentMoney - OldMoney;
		OnMoneyChanged.Broadcast(CurrentMoney, AppliedDelta);
		BP_OnMoneyChanged(CurrentMoney, AppliedDelta);
	}
}

void ABRPlayerState::ServerAddMoney_Implementation(int32 Delta)
{
	// العميل لا يثق به: أي طلب يمر من هنا يُعاد التحقق منه.
	if (!HasAuthority())
	{
		return;
	}

	ApplyMoneyDelta(Delta);
}

float ABRPlayerState::GetHealthPercent() const
{
	return MaxHealth > 0.f ? FMath::Clamp(PlayerHealth / MaxHealth, 0.f, 1.f) : 0.f;
}

// -----------------------------------------------------------------------------
// OnRep (clients)
// -----------------------------------------------------------------------------

void ABRPlayerState::OnRep_CurrentMoney()
{
	// OnRep لا يعطي الفرق، لذا نحسبه من آخر قيمة شاهدها هذا الجهاز.
	const int32 Delta = CurrentMoney - LastKnownMoney;
	LastKnownMoney = CurrentMoney;

	OnMoneyChanged.Broadcast(CurrentMoney, Delta);
	BP_OnMoneyChanged(CurrentMoney, Delta);
}

void ABRPlayerState::OnRep_PlayerHealth()
{
	BP_OnHealthChanged(PlayerHealth);

	if (PlayerHealth <= 0.f)
	{
		BP_OnDeath();
	}
}

void ABRPlayerState::OnRep_PlayerDisplayName()
{
	BP_OnDisplayNameChanged(PlayerDisplayName);
}
