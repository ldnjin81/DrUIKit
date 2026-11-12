#include "Core/DrUIState.h"

EDrUIState DrUIState::Resolve(const FDrUIStateInputs& In) {
  if (In.bDisabled) return EDrUIState::Disabled;
  if (In.bLoading) return EDrUIState::Loading;
  if (In.bSuccess) return EDrUIState::Success;
  if (In.bError) return EDrUIState::Error;
  if (In.bPressed) return EDrUIState::Pressed;
  if (In.bHovered) return EDrUIState::Hover;
  if (In.bSelected) return EDrUIState::Selected;
  if (In.bFocused) return EDrUIState::Focus;
  return EDrUIState::Default;
}

bool DrUIState::IsFocusRingVisible(const FDrUIStateInputs& In) { return In.bFocused && !In.bDisabled; }
