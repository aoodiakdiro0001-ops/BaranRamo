// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BaranRamo.Build.cs
// ملف تعريف الوحدة (Module). مكانه:
//     Source/BaranRamo/BaranRamo.Build.cs
//
// لماذا نحتاجه؟ كل ملفات .h/.cpp في هذا المجلد تحتاج وحدة تعرف:
//   - الوحدات التي تعتمد عليها (Engine, EnhancedInput, ...).
//   - مسار الـ PCH.
// بدونه لن يُبنى المشروع أصلاً.
// =============================================================================

using UnrealBuildTool;

public class BaranRamo : ModuleRules
{
	public BaranRamo(ReadOnlyTargetRules Target) : base(Target)
	{
		// PCH مشترك: بناء أسرع بكثير في المشاريع الصغيرة.
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",             // FString, TArray, FMath, Logging
			"CoreUObject",      // UObject, UPROPERTY, Reflection
			"Engine",           // AActor, ACharacter, UPhysicsHandleComponent, UCameraComponent
			"InputCore",        // EKeys (لو أضفت Input قديم)
			"EnhancedInput",    // UInputAction, UInputMappingContext
			"UMG",              // الـ HUD (Widgets) لاحقاً
			"AIModule",         // Behavior Trees للحيوانات (المرحلة 3)
			"NavigationSystem"  // NavMesh لحركة الحيوانات
		});

		// وحدات لا تظهر في الواجهات العامة (headers) — تبقى خاصة.
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore"
		});

		// لو أضفت Chaos Vehicles لاحقاً، أضف:
		//     "ChaosVehicles",
		//     "PhysXVehicles"   // (للمركبات القديمة فقط)
	}
}
