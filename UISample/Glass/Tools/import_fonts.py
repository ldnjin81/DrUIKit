"""Pretendard(OFL) TTF 5개를 글꼴 페이스로 가져오고 복합 글꼴 /DrUIGlass/Fonts/F_Pretendard로 묶는다.
서체 이름 = 굵기 이름(Light, Regular, Medium, SemiBold, Bold) — DrUIKit이 굵기로 서체를 고를 때 쓰는 이름.

UnrealEditor-Cmd Host/DrUIKitHost.uproject -run=pythonscript
-script=<절대경로>/UISample/Glass/Tools/import_fonts.py -EnablePlugins=PythonScriptPlugin -nullrhi -unattended
"""
import os
import unreal as u

here = os.path.dirname(os.path.abspath(__file__))
source = os.path.normpath(os.path.join(here, '..', 'DrUIGlass', 'Fonts'))
dest = '/DrUIGlass/Fonts'
weights = ['Light', 'Regular', 'Medium', 'SemiBold', 'Bold']
lib = u.EditorAssetLibrary
tools = u.AssetToolsHelpers.get_asset_tools()

tasks = []
for w in weights:
    task = u.AssetImportTask()
    task.filename = os.path.join(source, 'Pretendard-%s.ttf' % w)
    task.destination_path = dest
    task.destination_name = 'FF_Pretendard_%s' % w
    task.automated = True
    task.replace_existing = True
    task.save = True
    tasks.append(task)
tools.import_asset_tasks(tasks)

entries = []
for w in weights:
    face = u.load_asset('%s/FF_Pretendard_%s' % (dest, w))
    if not face:
        raise RuntimeError('글꼴 페이스 가져오기 실패: ' + w)
    face.set_editor_property('loading_policy', u.FontLoadingPolicy.LAZY_LOAD)
    lib.save_loaded_asset(face, False)
    data = u.FontData()
    data.set_editor_property('font_face_asset', face)
    entry = u.TypefaceEntry()
    entry.set_editor_property('name', w)
    entry.set_editor_property('font', data)
    entries.append(entry)

font_path = dest + '/F_Pretendard'
font = u.load_asset(font_path) if lib.does_asset_exist(font_path) else tools.create_asset('F_Pretendard', dest, u.Font, u.FontFactory())
if not font:
    raise RuntimeError('복합 글꼴 생성 실패')
font.set_editor_property('font_cache_type', u.FontCacheType.RUNTIME)
composite = u.CompositeFont()
typeface = u.Typeface()
typeface.set_editor_property('fonts', entries)
composite.set_editor_property('default_typeface', typeface)
font.set_editor_property('composite_font', composite)
if not lib.save_loaded_asset(font, False):
    raise RuntimeError('복합 글꼴 저장 실패')
u.log('DRUIGLASS_FONTS_READY')
