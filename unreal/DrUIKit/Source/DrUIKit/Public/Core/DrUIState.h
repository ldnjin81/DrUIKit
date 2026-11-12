#pragma once
#include "CoreMinimal.h"
#include "DrUIState.generated.h"

/** 부품의 보이는 상태(디자인 시스템 '버튼 상태 9가지'). 한 순간에는 하나만 보인다. */
UENUM(BlueprintType)
enum class EDrUIState : uint8 {
  Default, Hover, Pressed, Focus, Disabled, Loading, Success, Error, Selected
};

/** 상태를 정하는 입력. 여러 개가 동시에 참일 수 있다. */
struct FDrUIStateInputs {
  bool bHovered = false, bPressed = false, bFocused = false, bDisabled = false;
  bool bLoading = false, bSuccess = false, bError = false, bSelected = false;
};

namespace DrUIState {
/** 우선순위: 비활성 > 로딩 > 성공 > 오류 > 눌림 > 호버 > 선택됨 > 포커스 > 기본.
    포커스 링은 상태와 별개로 그리므로(IsFocusRingVisible) 다른 상태가 이긴다. */
DRUIKIT_API EDrUIState Resolve(const FDrUIStateInputs& In);
/** 키보드·패드 포커스 링을 그릴지. 비활성일 때만 숨긴다. */
DRUIKIT_API bool IsFocusRingVisible(const FDrUIStateInputs& In);
}
