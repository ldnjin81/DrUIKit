// 갤러리가 DrUIKit의 모든 위젯 클래스를 담는지 검사한다(새 UI를 갤러리에 빠뜨리면 실패).
#if WITH_DEV_AUTOMATION_TESTS
#include "Components/Widget.h"
#include "DrGlassGallery.h"
#include "Misc/AutomationTest.h"
#include "UObject/UObjectIterator.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FDrGlassGalleryCoverageTest, "DrUIGlass.Gallery.CoversAllKitWidgets", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FDrGlassGalleryCoverageTest::RunTest(const FString&) {
  const TArray<UClass*> Covered = UDrGlassGallery::CoveredClasses();
  int32 KitWidgets = 0;
  for (TObjectIterator<UClass> It; It; ++It) {
    UClass* Class = *It;
    if (!Class->IsChildOf(UWidget::StaticClass()) || Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists)) continue;
    if (Class->GetOutermost()->GetName() != TEXT("/Script/DrUIKit")) continue;
    ++KitWidgets;
    TestTrue(*FString::Printf(TEXT("갤러리에 %s가 있어야 함(UDrGlassGallery 페이지와 CoveredClasses에 추가)"), *Class->GetName()), Covered.Contains(Class));
  }
  TestTrue(TEXT("DrUIKit 위젯 클래스를 찾음"), KitWidgets > 0);
  return true;
}
#endif
