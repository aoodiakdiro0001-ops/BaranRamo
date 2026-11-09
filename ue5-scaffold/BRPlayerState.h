// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BRPlayerState.h
// حالة اللاعب التي تنتقل معه بين المستويات وتُزامَن (Replicated) إلى كل العملاء.
//
// لماذا PlayerState وليس Character؟
//   الـ Character يُدمَّر عند الموت أو تغيير الخريطة؛ PlayerState يبقى.
//   النقود والاسم والصحة يجب أن تنجو من ذلك، ولذا مكانها هنا.
//
// تنبيه: APlayerState يرث بالفعل الخاصية PlayerName (Replicated) من المحرك.
//   إعادة تعريفها هنا تسبب خطأ UHT (Duplicate property). لذلك نستخدم
//   PlayerName الموروث للاسم الداخلي، ونضيف PlayerDisplayName للاسم المعروض.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "BRPlayerState.generated.h"

/** يُبثّ عند تغيّر النقود: القيمة الجديدة + الفرق (سالب عند الخصم). */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBRMoneyChangedSignature, int32, NewMoney, int32, Delta);

/**
 * حالة اللاعب على الشبكة.
 * كل UPROPERTY(Replicated) هنا موجودة على السيرفر، وتُنسخ تلقائياً للعملاء.
 */
UCLASS(BlueprintType)
class BARANRAMO_API ABRPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABRPlayerState();

	/** إلزامي: نُسجّل كل خاصية مكرّرة مع شرط التكرار (هنا: بدون شروط). */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -------------------------------------------------------------------------
	// الحالة المكرّرة (Replicated State)
	// -------------------------------------------------------------------------

	/** صحة اللاعب 0..MaxHealth. عند 0 يُستدعى BP_OnDeath على السيرفر. */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerHealth, BlueprintReadOnly, Category = "BaranRamo|Stats")
	float PlayerHealth = 100.f;

	/** الحد الأقصى للصحة — ثابت الآن، لكنه جاهز للتطوير (ترقيات، أدوية). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Stats")
	float MaxHealth = 100.f;

	/** النقود. تُعدَّل عبر ServerAddMoney فقط — لا تكتب عليها من العميل. */
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMoney, BlueprintReadOnly, Category = "BaranRamo|Economy")
	int32 CurrentMoney = 0;

	/** الاسم الظاهر فوق الرأس وفي قائمة اللاعبين. */
	UPROPERTY(ReplicatedUsing = OnRep_PlayerDisplayName, BlueprintReadOnly, Category = "BaranRamo|Identity")
	FString PlayerDisplayName = TEXT("Farmer");

	// -------------------------------------------------------------------------
	// الشبكة (RPCs)
	// -------------------------------------------------------------------------

	/**
	 * Server RPC: العميل يطلب تعديل النقود، والسيرفر هو من يقرر.
	 * Delta موجب = ربح، سالب = خصم.
	 *
	 * الحماية: نتحقق من HasAuthority داخل التنفيذ، ونمنع النقود السالبة.
	 * (لا نستخدم WithValidation لأن القيمة عددية بسيطة والتحقق هنا كافٍ.)
	 *
	 * مثال من Blueprint: Server Add Money (Delta = -50) عند شراء بذور.
	 */
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "BaranRamo|Economy")
	void ServerAddMoney(int32 Delta);

	/** فحص محلي (لا يستهلك شبكة): هل يملك اللاعب ثمن هذا الشيء؟ */
	UFUNCTION(BlueprintPure, Category = "BaranRamo|Economy")
	bool CanAfford(int32 Cost) const { return Cost >= 0 && CurrentMoney >= Cost; }

	/** نسبة الصحة 0..1 — جاهزة لشريط الصحة في الـ HUD. */
	UFUNCTION(BlueprintPure, Category = "BaranRamo|Stats")
	float GetHealthPercent() const;

	/** تعيين النقود مباشرة — للسيرفر فقط (يُستدعى من GameMode عند الدخول). */
	void SetInitialMoney(int32 NewMoney);

	/** تعديل محلي على السيرفر مع بث الأحداث (تُستدعى داخلياً، وللـ GameMode). */
	void ApplyMoneyDelta(int32 Delta);

	// -------------------------------------------------------------------------
	// أحداث الـ UI
	// -------------------------------------------------------------------------

	/** يربطه الـ HUD من Blueprint: ربط مباشر بلا Cast. */
	UPROPERTY(BlueprintAssignable, Category = "BaranRamo|Events")
	FBRMoneyChangedSignature OnMoneyChanged;

	/** حدث Blueprint لتحديث شريط النقود (يُستدعى من OnRep وعلى السيرفر). */
	UFUNCTION(BlueprintImplementableEvent, Category = "BaranRamo|UI")
	void BP_OnMoneyChanged(int32 NewMoney, int32 Delta);

	/** حدث Blueprint لتحديث شريط الصحة. */
	UFUNCTION(BlueprintImplementableEvent, Category = "BaranRamo|UI")
	void BP_OnHealthChanged(float NewHealth);

	/** حدث Blueprint للاسم (قائمة اللاعبين). */
	UFUNCTION(BlueprintImplementableEvent, Category = "BaranRamo|UI")
	void BP_OnDisplayNameChanged(const FString& NewDisplayName);

	/** حدث Blueprint عند وصول الصحة إلى 0 (يُستدعى مرة واحدة لكل تغيير صحة). */
	UFUNCTION(BlueprintImplementableEvent, Category = "BaranRamo|UI")
	void BP_OnDeath();

protected:
	/** يُستدعى على السيرفر عند إنشاء الـ PlayerState: ننسخ الاسم من APlayerState. */
	virtual void ClientInitialize(AController* C) override;

	// -------------------------------------------------------------------------
	// OnRep: تُنفَّذ على العملاء فقط عند وصول قيمة جديدة من السيرفر.
	// -------------------------------------------------------------------------

	UFUNCTION()
	void OnRep_CurrentMoney();

	UFUNCTION()
	void OnRep_PlayerHealth();

	UFUNCTION()
	void OnRep_PlayerDisplayName();

private:
	/** آخر قيمة نقود شاهدها هذا الجهاز — لحساب Delta داخل OnRep. */
	UPROPERTY()
	int32 LastKnownMoney = 0;
};
