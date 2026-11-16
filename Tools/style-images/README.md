# 스타일 그림 만들기

판타지 RPG(`UISample/Fantasy`), 양피지(`UISample/Parchment`), 다크 판타지(`UISample/DarkFantasy`)의 `Images/`는 이 저장소에서 직접 만든 그림이다. 외부 에셋은 쓰지 않았다.

## 패널·카드(9분할)

`panels.py`가 numpy·Pillow로 절차적으로 그린다(종이 얼룩·결·그을음, 가죽 결, 금·청동 테두리와 모서리 장식, 꺾쇠).

```
python panels.py <출력 폴더>
```

- `<스타일>-panel-<light|dark>.png`: 512px, 9분할 여백 96px → `Images/panel-<mode>.png`(style.json `glass`, `margin: 96`, `scale: 0.5`)
- `<스타일>-card-<light|dark>.png`: 128px, 여백 16px → `Images/card-<mode>.png`(style.json `glass-strong`, `margin: 16`, `scale: 0.5`)

## 바탕(ground)

FLUX.1 [schnell](Apache 2.0)로 1344×768을 만들고 JPG(품질 82)로 줄였다. 모두 끝에 `no text, no letters, no characters, no people`을 붙였다.

| 파일 | 프롬프트 |
| --- | --- |
| Fantasy `ground-light.jpg` | epic fantasy RPG main menu background painting, sunlit green valley with a distant white stone castle on a cliff, waterfalls, soft clouds, golden hour, painterly concept art, wide establishing shot, calm empty center area |
| Fantasy `ground-dark.jpg` | fantasy RPG main menu background painting, interior of a medieval tavern hall at night, warm candlelight, wooden beams, hanging lanterns, banners, stone fireplace, painterly concept art, dark moody vignette |
| DarkFantasy `ground-light.jpg` | dark fantasy game menu background, bleak misty moor at dawn with a ruined castle silhouette and dead trees, pale overcast sky, ash grey and muted tones, painterly concept art, soft fog |
| DarkFantasy `ground-dark.jpg` | dark fantasy game main menu background, ruined gothic cathedral interior, broken pointed arches, thick fog, drifting embers, cold moonlight through shattered windows, desaturated, grim atmosphere, painterly concept art, heavy dark vignette |
| Parchment `ground-light.jpg` | top-down view of an old wooden scholar desk with an unrolled blank aged parchment map area, quill and inkwell, wax seal, brass compass, scattered old papers at the edges, warm daylight, empty center, painterly illustration |
| Parchment `ground-dark.jpg` | top-down view of a dark wooden desk at night lit by a single candle, old leather-bound books, blank parchment scrolls, quill and inkwell, wax seals, deep shadows at the edges, empty center, painterly illustration |

같은 프롬프트라도 시드가 달라 다른 그림이 나온다. 글자처럼 보이는 것이 섞이거나 위아래 검은 띠가 생긴 결과는 버렸다.
