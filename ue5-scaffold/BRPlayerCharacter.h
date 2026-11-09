// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BRPlayerCharacter.h
// منظور الشخص الأول (First Person) + تفاعل + حمل فيزيائي.
//
// المكوّنات:
//   CapsuleComponent   → التصادم والحركة (من ACharacter)
//   FirstPersonCamera  → الكاميرا عند مستوى العين
//   HoldPoint          → نقطة أمام الكاميرا يُثبَّت عليها الجسم المحمول
//   PhysicsHandle      → يحرّك الجسم المحمول فيزيائياً بسلاسة
//
// التفعيل في المحرر (ضروري، لا يعمل بدونه):
//   1. أنشئ BP_BRPlayerCharacter وارث منه.
//   2. أسنِد Input Actions (Move / Look / Interact / Grab) وMapping Context.
//   3. أسنِد Skeletal Mesh (Mixamo) للـ Mesh، وأخفِ الرأس إن ظهر.
//   4. في GameMode: DefaultPawnClass = BP_BRPlayerCharacter.
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "GameFramework/Character.h"
#include "BRPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class UPhysicsHandleComponent;
class USceneComponent;
struct FInputActionValue;

UCLASS(Blueprintable)
class BARANRAMO_API ABRPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABRPlayerCharacter();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// -------------------------------------------------------------------------
	// المكوّنات
	// -------------------------------------------------------------------------

	/** كاميرا الشخص الأول — مثبّتة على الكبسولة عند ارتفاع العين. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaranRamo|Camera")
	TObjectPtr<UCameraComponent> FirstPersonCamera;

	/** مقبض فيزيائي: يجعل الجسم المحمول يتبع نقطة الهدف دون كسر التصادم. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaranRamo|Interaction")
	TObjectPtr<UPhysicsHandleComponent> PhysicsHandle;

	/** نقطة أمام الكاميرا يوضع فيها الجسم المحمول. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "BaranRamo|Interaction")
	TObjectPtr<USceneComponent> HoldPoint;

	// -------------------------------------------------------------------------
	// إعدادات التفاعل
	// -------------------------------------------------------------------------

	/** مدى التفاعل بالسنتيمتر — 250 سم كما في تصميم اللعبة. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Interaction",
		meta = (ClampMin = "50.0", ClampMax = "1000.0", Units = "cm"))
	float InteractDistance = 250.f;

	/** قناة الـ Trace. اجعلها Visibility، أو أنشئ قناة مخصّصة Interactable. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Interaction")
	TEnumAsByte<ECollisionChannel> InteractTraceChannel = ECC_Visibility;

	/** يرسم خط الـ Trace في اللعب — للتشخيص فقط، أوقفه في الإصدار. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Interaction")
	bool bDrawDebugTrace = false;

	/** كل كم ثانية نعيد حساب الهدف تحت المؤشر (0.1 = 10 مرات/ثانية). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Interaction",
		meta = (ClampMin = "0.02", ClampMax = "1.0"))
	float FocusTraceInterval = 0.1f;

	// -------------------------------------------------------------------------
	// إعدادات الحمل
	// -------------------------------------------------------------------------

	/** بُعد نقطة الحمل أمام الكاميرا. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Grab", meta = (ClampMin = "50.0", Units = "cm"))
	float HoldDistance = 200.f;

	/** أقصى كتلة يمكن حملها (كغ). الأثقل يحتاج مركبة أو رافعة. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Grab", meta = (ClampMin = "1.0", Units = "kg"))
	float MaxGrabMass = 60.f;

	/** سرعة تثبيت الجسم في اليد — أعلى = أصلب وأكثر اهتزازاً. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BaranRamo|Grab", meta = (ClampMin = "1.0"))
	float GrabInterpolationSpeed = 15.f;

	// -------------------------------------------------------------------------
	// Input (Enhanced Input — الافتراضي في UE 5.1+)
	// -------------------------------------------------------------------------

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Input")
	TObjectPtr<UInputAction> LookAction;

	/** زر E — التفاعل مع IBPI_Interactable. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Input")
	TObjectPtr<UInputAction> InteractAction;

	/** زر الفأرة الأيسر أو F — حمل/إفلات. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "BaranRamo|Input")
	TObjectPtr<UInputAction> GrabAction;

	// -------------------------------------------------------------------------
	// API عام
	// -------------------------------------------------------------------------

	/**
	 * ينفّذ Line Trace بطول InteractDistance، وإذا كان الهدف يطبّق
	 * IBPI_Interactable يستدعي OnInteract.
	 * @return true إذا وُجد هدف تفاعلي ونُفِّذ.
	 */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Interaction")
	bool TryInteract();

	/** يحاول حمل الجسم أمام الكاميرا (يتحقق من الكتلة وقابلية الحركة). */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Grab")
	bool TryGrab();

	/** يُفلت الجسم المحمول (يسقط بالجاذبية الطبيعية). */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Grab")
	void ReleaseGrab();

	/** يمسح التركيز الحالي ويُبلّغ الـ UI. */
	UFUNCTION(BlueprintCallable, Category = "BaranRamo|Interaction")
	void ClearFocus();

	UFUNCTION(BlueprintPure, Category = "BaranRamo|Grab")
	bool IsGrabbing() const;

	UFUNCTION(BlueprintPure, Category = "BaranRamo|Interaction")
	AActor* GetFocusedActor() const { return FocusedActor.Get(); }

	/** حدث للـ UI: تغيّر الهدف تحت المؤشر (nullptr = لا شيء). */
	UFUNCTION(BlueprintImplementableEvent, Category = "BaranRamo|UI")
	void BP_OnFocusChanged(AActor* NewFocusedActor);

protected:
	// معالجات Input — التوقيع يجب أن يكون (const FInputActionValue&)
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void InteractPressed();
	void GrabPressed();

	/** يعيد حساب الهدف تحت المؤشر ويستدعي OnFocusGet عند التغيّر فقط. */
	void UpdateFocus();

	/** Line Trace من الكاميرا إلى الأمام. */
	bool TraceFromView(FHitResult& OutHit, float Distance) const;

	/** يحدّث موضع الجسم المحمول كل إطار. */
	void ApplyHoldTarget();

	/** الهدف الحالي — TWeakObjectPtr لأن Actor قد يُدمَّر بين إطارين. */
	UPROPERTY()
	TWeakObjectPtr<AActor> FocusedActor;

	/** مؤقّت داخلي لتحديث التركيز. */
	float FocusTimer = 0.f;
};
