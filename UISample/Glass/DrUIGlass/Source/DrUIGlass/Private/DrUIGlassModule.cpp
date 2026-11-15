#include "Modules/ModuleManager.h"

// 테마는 코어 스타일 로더(DrUIStyle)가 Tokens/tokens.json·style.json에서 읽는다. 이 모듈은 갤러리·바탕 위젯만 갖는다.
IMPLEMENT_MODULE(FDefaultModuleImpl, DrUIGlass)
