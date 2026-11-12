"""갤러리 테스트 레벨 /DrUIGlass/Maps/L_DrUIGallery를 만든다(게임 모드 ADrGlassGalleryGameMode).

UnrealEditor-Cmd Host/DrUIKitHost.uproject -run=pythonscript
-script=<절대경로>/UISample/Glass/Tools/build_gallery_level.py -EnablePlugins=PythonScriptPlugin -nullrhi -unattended
"""
import unreal as u

level = '/DrUIGlass/Maps/L_DrUIGallery'
levels = u.get_editor_subsystem(u.LevelEditorSubsystem)
# 커맨드릿에서는 플러그인 콘텐츠가 아직 검색되지 않아 does_asset_exist가 거짓일 수 있다. 먼저 열어 보고 없으면 만든다.
if not (levels.load_level(level) or levels.new_level(level)):
    raise RuntimeError('갤러리 레벨 생성 실패')
world = u.get_editor_subsystem(u.UnrealEditorSubsystem).get_editor_world()
world.get_world_settings().set_editor_property('default_game_mode', u.DrGlassGalleryGameMode)
if not levels.save_current_level():
    raise RuntimeError('갤러리 레벨 저장 실패')
u.log('DRUIGLASS_GALLERY_READY')
