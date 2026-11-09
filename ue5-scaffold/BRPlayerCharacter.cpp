// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BRPlayerCharacter.cpp
// ملاحظة: نصوص UE_LOG بالإنجليزية عمداً لتفادي مشاكل ترميز MSVC.
// =============================================================================

#include "BRPlayerCharacter.h"

#include "BPI_Interactable.h"
#include "Camera/CameraComponent.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Math/RotationMatrix.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"

// -----------------------------------------------------------------------------
// Construction
// -----------------------------------------------------------------------------

ABRPlayerCharacter::ABRPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// --- الكبسولة: مقاس إنسان قياسي (نصف قطر 34، نصف ارتفاع 88) ---
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);

	// منظور شخص أول: الدوران الأفقي يتبع الكاميرا، لا اتجاه الحركة.
	bUseControllerRotationYaw = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->MaxWalkSpeed = 400.f;
		Movement->JumpZVelocity = 420.f;
		Movement->AirControl = 0.2f;
		Movement->BrakingDecelerationWalking = 2000.f;
	}

	// --- الكاميرا ---
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f)); // ارتفاع العين
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->FieldOfView = 90.f;

	// --- نقطة الحمل: أمام الكاميرا تماماً ---
	HoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("HoldPoint"));
	HoldPoint->SetupAttachment(FirstPersonCamera);
	HoldPoint->SetRelativeLocation(FVector(HoldDistance, 0.f, 0.f));

	// --- المقبض الفيزيائي: مكوّن منطقي، لا يُثبَّت على أي هيكل ---
	PhysicsHandle = CreateDefaultSubobject<UPhysicsHandleComponent>(TEXT("PhysicsHandle"));

	// --- الجسم: لا نراه في منظور الشخص الأول، لكنه يلقي ظلاً ---
	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetOwnerNoSee(true);
		MeshComp->bCastHiddenShadow = true;
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
		MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f)); // Mixamo يواجه Y
	}
}

// -----------------------------------------------------------------------------
// Lifecycle
// -----------------------------------------------------------------------------

void ABRPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	// تسجيل Mapping Context — بدونه لا تصل أي ضغطة زر إلى اللاعب.
	if (const APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, /*Priority=*/0);
			}
		}
	}

	if (PhysicsHandle)
	{
		// كلما زادت القيمة صار الجسم أصلب في اليد (وأكثر عرضة للاهتزاز).
		PhysicsHandle->SetInterpolationSpeed(GrabInterpolationSpeed);
	}
}

void ABRPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// نُفلت أي جسم محمول وإلا بقي معلّقاً في الهواء بعد موت اللاعب/الخروج.
	ReleaseGrab();

	Super::EndPlay(EndPlayReason);
}

void ABRPlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	ApplyHoldTarget();

	// تحديث التركيز على فترات، لا كل إطار: توفير للمعالج على الهاتف.
	FocusTimer += DeltaSeconds;
	if (FocusTimer >= FMath::Max(0.02f, FocusTraceInterval))
	{
		FocusTimer = 0.f;
		UpdateFocus();
	}
}

// -----------------------------------------------------------------------------
// Input
// -----------------------------------------------------------------------------

void ABRPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInput)
	{
		// السبب الشائع: DefaultInputComponentClass ليس EnhancedInputComponent.
		// الحل: Project Settings → Input → Default Classes.
		UE_LOG(LogTemp, Error,
			TEXT("[BaranRamo] EnhancedInputComponent missing. Check Project Settings > Input > Default Classes."));
		return;
	}

	if (MoveAction)
	{
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABRPlayerCharacter::Move);
	}
	if (LookAction)
	{
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABRPlayerCharacter::Look);
	}
	if (InteractAction)
	{
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Started, this, &ABRPlayerCharacter::InteractPressed);
	}
	if (GrabAction)
	{
		EnhancedInput->BindAction(GrabAction, ETriggerEvent::Started, this, &ABRPlayerCharacter::GrabPressed);
	}
}

void ABRPlayerCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (Axis.IsNearlyZero() || !Controller)
	{
		return;
	}

	// نستخدم الـ Yaw فقط حتى لا يطير اللاعب عند النظر للأعلى.
	const FRotator YawOnly(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

	AddMovementInput(Forward, Axis.Y);
	AddMovementInput(Right, Axis.X);
}

void ABRPlayerCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();

	AddControllerYawInput(Axis.X);
	// السالب هنا لأن محور الفأرة Y موجب للأسفل؛ إن كان معكوساً عندك،
	// أزل السالب أو استخدم Modifier = Negate في الـ Mapping Context.
	AddControllerPitchInput(-Axis.Y);
}

void ABRPlayerCharacter::InteractPressed()
{
	TryInteract();
}

void ABRPlayerCharacter::GrabPressed()
{
	// نفس الزر يحمل ويُفلت — سلوك متوقع في ألعاب البقاء.
	if (IsGrabbing())
	{
		ReleaseGrab();
	}
	else
	{
		TryGrab();
	}
}

// -----------------------------------------------------------------------------
// Interaction
// -----------------------------------------------------------------------------

bool ABRPlayerCharacter::TraceFromView(FHitResult& OutHit, float Distance) const
{
	if (!FirstPersonCamera || !GetWorld())
	{
		return false;
	}

	const FVector Start = FirstPersonCamera->GetComponentLocation();
	const FVector End = Start + FirstPersonCamera->GetForwardVector() * Distance;

	// نتجاهل اللاعب نفسه، وأي جسم نحملة (وإلا صار الـ Trace يصطدم به دائماً).
	FCollisionQueryParams Params(FName(TEXT("BRInteractTrace")), /*bTraceComplex=*/false, this);
	if (PhysicsHandle)
	{
		if (UPrimitiveComponent* Grabbed = PhysicsHandle->GetGrabbedComponent())
		{
			Params.AddIgnoredComponent(Grabbed);
		}
	}

	const ECollisionChannel Channel = static_cast<ECollisionChannel>(InteractTraceChannel.GetValue());
	const bool bHit = GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, Channel, Params);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebugTrace)
	{
		DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Green : FColor::Red, false, 0.5f, 0, 1.f);
		if (bHit)
		{
			DrawDebugPoint(GetWorld(), OutHit.ImpactPoint, 8.f, FColor::Yellow, false, 0.5f);
		}
	}
#endif

	return bHit;
}

bool ABRPlayerCharacter::TryInteract()
{
	FHitResult Hit;
	if (!TraceFromView(Hit, InteractDistance))
	{
		return false;
	}

	AActor* HitActor = Hit.GetActor();
	if (!HitActor || !HitActor->Implements<UBPI_Interactable>())
	{
		return false;
	}

	// تنفيذ الواجهة. ملاحظة شبكية: إذا كان الـ Actor مملوكاً للسيرفر
	// (صناديق، محاصيل مشتركة) فاستدعِ Server RPC هنا بدل التنفيذ المباشر،
	// ثم نفّذ Execute_OnInteract داخل الـ RPC على السيرفر.
	IBPI_Interactable::Execute_OnInteract(HitActor, this);

	return true;
}

void ABRPlayerCharacter::UpdateFocus()
{
	AActor* NewFocus = nullptr;

	FHitResult Hit;
	if (TraceFromView(Hit, InteractDistance))
	{
		AActor* HitActor = Hit.GetActor();
		if (HitActor && HitActor->Implements<UBPI_Interactable>())
		{
			NewFocus = HitActor;
		}
	}

	AActor* OldFocus = FocusedActor.Get();
	if (NewFocus == OldFocus)
	{
		return; // لا تغيير: لا نُزعج الـ UI بلا سبب.
	}

	FocusedActor = NewFocus;

	if (NewFocus)
	{
		// يُستدعى مرة واحدة عند دخول الهدف في المرمى (Outline / نص التلميح).
		IBPI_Interactable::Execute_OnFocusGet(NewFocus);
	}

	BP_OnFocusChanged(NewFocus);
}

void ABRPlayerCharacter::ClearFocus()
{
	if (!FocusedActor.IsValid())
	{
		return;
	}

	FocusedActor = nullptr;
	BP_OnFocusChanged(nullptr);
}

// -----------------------------------------------------------------------------
// Grab / Release
// -----------------------------------------------------------------------------

bool ABRPlayerCharacter::TryGrab()
{
	if (!PhysicsHandle || PhysicsHandle->GetGrabbedComponent())
	{
		return false;
	}

	FHitResult Hit;
	if (!TraceFromView(Hit, InteractDistance))
	{
		return false;
	}

	UPrimitiveComponent* Component = Hit.GetComponent();
	if (!Component)
	{
		return false;
	}

	// 1) يجب أن يكون قابلاً للحركة (الجدران الثابتة لا تُحمل).
	if (Component->Mobility != EComponentMobility::Movable)
	{
		return false;
	}

	// 2) يجب أن يحاكي الفيزياء فعلياً، وإلا فالمقبض لا يجد ما يحرّكه.
	if (!Component->IsSimulatingPhysics())
	{
		return false;
	}

	// 3) حدّ الكتلة: منع حمل الجرار بيد واحدة.
	const float Mass = Component->GetMass();
	if (Mass > MaxGrabMass)
	{
		UE_LOG(LogTemp, Verbose, TEXT("[BaranRamo] Object too heavy: %.1f kg (max %.1f kg)."), Mass, MaxGrabMass);
		return false;
	}

	// عند بدء الـ Trace داخل مجسم (التصاق) لا يكون ImpactPoint صالحاً.
	const FVector GrabLocation = Hit.bStartPenetrating ? Hit.Location : Hit.ImpactPoint;

	PhysicsHandle->GrabComponentAtLocationWithRotation(
		Component,
		NAME_None, // بلا Bone: نمسك المجسم كاملاً
		GrabLocation,
		Component->GetComponentRotation());

	return true;
}

void ABRPlayerCharacter::ReleaseGrab()
{
	if (!PhysicsHandle)
	{
		return;
	}

	if (PhysicsHandle->GetGrabbedComponent())
	{
		// لإضافة رمي: طبّق Impulse قبل الإفلات:
		//   Grabbed->AddImpulse(FirstPersonCamera->GetForwardVector() * ThrowStrength, NAME_None, true);
		PhysicsHandle->ReleaseComponent();
	}
}

void ABRPlayerCharacter::ApplyHoldTarget()
{
	if (!PhysicsHandle || !HoldPoint || !FirstPersonCamera)
	{
		return;
	}

	if (!PhysicsHandle->GetGrabbedComponent())
	{
		return;
	}

	// الهدف = نقطة أمام الكاميرا، والدوران = دوران الكاميرا (كي لا يدور الجسم عشوائياً).
	PhysicsHandle->SetTargetLocationAndRotation(
		HoldPoint->GetComponentLocation(),
		FirstPersonCamera->GetComponentRotation());
}

bool ABRPlayerCharacter::IsGrabbing() const
{
	return PhysicsHandle && PhysicsHandle->GetGrabbedComponent() != nullptr;
}
