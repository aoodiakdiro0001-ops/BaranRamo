// BaranRamo © 2026 Baran Avisat — All Rights Reserved.
// =============================================================================
// BPI_Interactable.h
// واجهة (Interface) موحّدة لكل شيء يمكن للاعب التفاعل معه:
// صناديق، أبواب، محاصيل، حيوانات، آلات، ونقاط البيع (Sell Zone).
//
// الاستخدام من C++:
//     if (Actor->Implements<UBPI_Interactable>())
//     {
//         IBPI_Interactable::Execute_OnInteract(Actor, this);
//     }
// الاستخدام من Blueprint:
//     Class Settings → Interfaces → Add → BR Interactable،
//     ثم نفّذ الحدثين On Interact و On Focus Get.
//
// ملاحظة مهمة: الحدثان BlueprintNativeEvent، لذا أي صنف C++ يطبّق الواجهة
// يجب أن يعرّف الدالتين باسم  <FunctionName>_Implementation .
// =============================================================================

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BPI_Interactable.generated.h"

/**
 * UInterface: الغلاف الذي تحتاجه Unreal للتعامل مع الواجهة كنوع UObject.
 * MinimalAPI: لا نصدّر هذه الفئة لأن التطبيق يحدث في C++/BP داخل نفس الوحدة.
 */
UINTERFACE(MinimalAPI, Blueprintable, meta = (DisplayName = "BR Interactable"))
class UBPI_Interactable : public UInterface
{
	GENERATED_BODY()
};

/**
 * الواجهة الفعلية. لاحظ أنها لا ترث من UObject ولا تحتوي مكوّنات —
 * مجرد عقد (contract) بين اللاعب والعالم.
 */
class BARANRAMO_API IBPI_Interactable
{
	GENERATED_BODY()

public:
	/**
	 * يُستدعى عندما يضغط اللاعب زر التفاعل (E) والشيء داخل مرمى الـ Line Trace.
	 *
	 * @param Instigator  الفاعل (عادة ABRPlayerCharacter). قد يكون nullptr في حال
	 *                    استُدعيت الواجهة من نظام آخر (مثل NPC).
	 *
	 * تنفيذ مقترح حسب النوع:
	 *   - صندوق   : افتح الـ Inventory، أو انقل محتواه إلى اللاعب.
	 *   - محصول   : احصد، أضف للـ Amount، واحذف الـ Actor.
	 *   - باب     : اعكس حالة الفتح بتدوير الـ Mesh 90 درجة.
	 *   - حيوان   : غذّه / احلبه / اجمعه.
	 *   - ماكينة  : شغّل الإنتاج بعد خصم المواد.
	 *
	 * يُنفَّذ على السيرفر فقط عندما يكون الـ Actor مملوكاً للسيرفر (Replicated).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaranRamo|Interaction")
	void OnInteract(AActor* Instigator);

	/**
	 * يُستدعى مرة واحدة عندما يدخل الشيء في مرمى النظر (يصبح الهدف الحالي)،
	 * وليس كل إطار. استخدمه لإظهار:
	 *   - Outline / Custom Depth Stencil
	 *   - نص «اضغط E للحصاد»
	 *   - أيقونة عائمة
	 *
	 * لا يوجد OnFocusLost في هذه النسخة: اللاعب يمسح التركيز بنفسه
	 * عبر ABRPlayerCharacter::ClearFocus() (انظر BRPlayerCharacter.h).
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "BaranRamo|Interaction")
	void OnFocusGet();
};
