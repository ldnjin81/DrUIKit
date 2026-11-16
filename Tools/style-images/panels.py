"""DrUIKit 판타지 계열 패널 9분할 이미지 생성기(절차적, 외부 에셋 없음).

사용: python panels.py <출력 폴더>
각 이미지는 512x512(2배 해상도), 9분할 여백 96px. style.json에서 margin 96, scale 0.5로 쓴다.
"""
import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFilter

S = 512
M = 96
rng = np.random.default_rng(7)


def fbm(size, octaves=6, base=4, seed=0):
    r = np.random.default_rng(seed)
    out = np.zeros((size, size), np.float32)
    amp, total = 1.0, 0.0
    for o in range(octaves):
        n = base * 2 ** o
        grid = (r.random((n, n)) * 255).astype(np.uint8)
        layer = np.asarray(Image.fromarray(grid).resize((size, size), Image.BICUBIC), np.float32) / 255.0
        out += layer * amp
        total += amp
        amp *= 0.5
    return out / total


def edge_distance(size):
    y, x = np.mgrid[0:size, 0:size].astype(np.float32)
    return np.minimum(np.minimum(x, size - 1 - x), np.minimum(y, size - 1 - y))


def lerp(a, b, t):
    a, b = np.asarray(a, np.float32), np.asarray(b, np.float32)
    return a + (b - a) * t[..., None]


def to_image(rgb, alpha):
    arr = np.dstack([np.clip(rgb, 0, 255), np.clip(alpha, 0, 255)]).astype(np.uint8)
    return Image.fromarray(arr, "RGBA")


def paper(light, dark, seed, burn=60.0, torn=True, jag_amp=22.0, jag_freq=16):
    """얼룩·결·가장자리 그을림이 있는 종이. torn이면 가장자리를 찢긴 모양으로 투명하게."""
    n = fbm(S, 7, 4, seed)
    fiber = fbm(S, 3, 64, seed + 1)
    stains = np.clip((fbm(S, 4, 3, seed + 2) - 0.55) * 4, 0, 1)
    # 9분할 가운데는 늘어나므로 큰 얼룩은 옅게(늘어난 줄무늬가 덜 보이게).
    t = np.clip((n - 0.3) * 1.2, 0, 1) * 0.4 + fiber * 0.12 + stains * 0.18
    d = edge_distance(S)
    jag = (fbm(S, 5, jag_freq, seed + 3) - 0.5) * jag_amp
    burn_t = np.clip(1 - (d + jag) / burn, 0, 1) ** 1.6
    rgb = lerp(light, dark, np.clip(t + burn_t * 0.9, 0, 1))
    if torn:
        alpha = np.clip((d + jag - 6) * 60, 0, 255)
    else:
        alpha = np.full((S, S), 255, np.float32)
    return rgb, alpha


def leather(base, seed):
    n = fbm(S, 6, 8, seed)
    grain = fbm(S, 2, 128, seed + 1)
    d = edge_distance(S)
    vign = np.clip(d / 140, 0, 1)
    shade = 0.55 + n * 0.35 + (grain - 0.5) * 0.18
    shade *= 0.6 + 0.4 * vign
    return np.asarray(base, np.float32) * shade[..., None]


def gold_line(draw, box, width, color, highlight):
    draw.rectangle(box, outline=color, width=width)
    x0, y0, x1, y1 = box
    draw.line([(x0 + width, y0 + 1), (x1 - width, y0 + 1)], fill=highlight, width=1)
    draw.line([(x0 + 1, y0 + width), (x0 + 1, y1 - width)], fill=highlight, width=1)


def corner_ornament(draw, cx, cy, sx, sy, color, size=34):
    """모서리 장식: 마름모 + 두 방향으로 뻗는 덩굴 곡선. sx, sy는 모서리에서 안쪽 방향(+1/-1)."""
    r = 9
    draw.polygon([(cx, cy - r * sy), (cx + r * sx, cy), (cx, cy + r * sy), (cx - r * sx, cy)], fill=color)
    for axis in (0, 1):
        pts = []
        for i in range(24):
            t = i / 23
            along = 14 + t * size
            wave = math.sin(t * math.pi * 1.5) * 7 * (1 - t * 0.4)
            px, py = (cx + along * sx, cy + wave * sy) if axis == 0 else (cx + wave * sx, cy + along * sy)
            pts.append((px, py))
        draw.line(pts, fill=color, width=3)
        ex, ey = pts[-1]
        draw.ellipse([ex - 4, ey - 4, ex + 4, ey + 4], fill=color)


def frame(img, outer, inner, color, highlight, ornament=True, inset_outer=14, inset_inner=28):
    draw = ImageDraw.Draw(img)
    gold_line(draw, (inset_outer, inset_outer, S - 1 - inset_outer, S - 1 - inset_outer), outer, color, highlight)
    if inner:
        draw.rectangle((inset_inner, inset_inner, S - 1 - inset_inner, S - 1 - inset_inner), outline=color, width=inner)
    if ornament:
        c = inset_outer + outer // 2
        for sx, sy, x, y in ((1, 1, c, c), (-1, 1, S - 1 - c, c), (1, -1, c, S - 1 - c), (-1, -1, S - 1 - c, S - 1 - c)):
            corner_ornament(draw, x, y, sx, sy, color)


def bracket_corners(img, color, inset=10, arm=70, width=3, dot=True):
    """다크 판타지: 얇은 사각선 + 모서리 꺾쇠와 끝점."""
    draw = ImageDraw.Draw(img)
    thin = tuple(list(color[:3]) + [110])
    draw.rectangle((inset + 8, inset + 8, S - 1 - inset - 8, S - 1 - inset - 8), outline=thin, width=2)
    for sx, sy, x, y in ((1, 1, inset, inset), (-1, 1, S - 1 - inset, inset), (1, -1, inset, S - 1 - inset), (-1, -1, S - 1 - inset, S - 1 - inset)):
        draw.line([(x, y + arm * sy), (x, y), (x + arm * sx, y)], fill=color, width=width)
        draw.line([(x + 8 * sx, y + 30 * sy), (x + 8 * sx, y + 8 * sy), (x + 30 * sx, y + 8 * sy)], fill=color, width=2)
        if dot:
            for ex, ey in ((x, y + arm * sy), (x + arm * sx, y)):
                draw.polygon([(ex, ey - 5), (ex + 5, ey), (ex, ey + 5), (ex - 5, ey)], fill=color)


def shadow_inside(img, strength=90, width=40):
    """테두리 안쪽으로 살짝 어두운 그림자(입체감)."""
    a = np.asarray(img, np.float32)
    d = edge_distance(S)
    k = np.clip(1 - (d - 16) / width, 0, 1) * (strength / 255)
    a[..., :3] *= (1 - k)[..., None]
    return Image.fromarray(a.astype(np.uint8), "RGBA")


def main(out):
    os.makedirs(out, exist_ok=True)

    # 판타지 밝게: 양피지 + 청동 테두리
    rgb, alpha = paper((236, 219, 178), (150, 104, 58), 11, burn=50, torn=False)
    img = shadow_inside(to_image(rgb, alpha), 70)
    frame(img, 6, 2, (122, 84, 38, 255), (214, 170, 96, 255))
    img.save(f"{out}/fantasy-panel-light.png")

    # 판타지 어둡게: 가죽 + 금 테두리
    rgb = leather((92, 56, 36), 21)
    img = shadow_inside(to_image(rgb, np.full((S, S), 255, np.float32)), 60)
    frame(img, 6, 2, (196, 154, 74, 255), (250, 224, 150, 255))
    img.save(f"{out}/fantasy-panel-dark.png")

    # 다크 판타지 어둡게: 거의 검은 반투명 + 옅은 금 꺾쇠
    y = np.linspace(0, 1, S, dtype=np.float32)[:, None].repeat(S, 1)
    n = fbm(S, 5, 6, 31)
    rgb = lerp((20, 18, 16), (8, 7, 7), y * 0.7 + n * 0.3)
    img = to_image(rgb, np.full((S, S), 232, np.float32))
    bracket_corners(img, (176, 150, 100, 255))
    img.save(f"{out}/darkfantasy-panel-dark.png")

    # 다크 판타지 밝게: 잿빛 반투명 + 쇠색 꺾쇠
    rgb = lerp((222, 219, 212), (196, 192, 184), y * 0.6 + n * 0.4)
    img = to_image(rgb, np.full((S, S), 236, np.float32))
    bracket_corners(img, (62, 58, 54, 255))
    img.save(f"{out}/darkfantasy-panel-light.png")

    # 양피지 밝게: 찢긴 가장자리 종이
    rgb, alpha = paper((241, 226, 190), (128, 84, 44), 41, burn=70)
    to_image(rgb, alpha).save(f"{out}/parchment-panel-light.png")

    # 양피지 어둡게: 촛불 아래 종이(어둡고 따뜻하게)
    rgb, alpha = paper((198, 168, 120), (70, 44, 22), 51, burn=90)
    to_image(rgb, alpha).save(f"{out}/parchment-panel-dark.png")

    # 카드(glass-strong): 패널과 같은 질감·같은 선(같은 화면에서 따로 떠 보이지 않게), 덩굴 장식만 뺀다(작은 칸에도 쓰인다).
    # 패널과 같은 512px, 여백 96px, 배율 .5. 선은 패널보다 안쪽 여유를 줄여 작은 칸에서도 내용 자리를 남긴다.
    rgb, alpha = paper((236, 219, 178), (150, 104, 58), 11, burn=50, torn=False)
    img = shadow_inside(to_image(rgb, alpha), 70)
    frame(img, 5, 2, (122, 84, 38, 255), (214, 170, 96, 255), ornament=False, inset_outer=10, inset_inner=20)
    img.save(f"{out}/fantasy-card-light.png")
    rgb = leather((92, 56, 36), 21)
    img = shadow_inside(to_image(rgb, np.full((S, S), 255, np.float32)), 60)
    frame(img, 5, 2, (196, 154, 74, 255), (250, 224, 150, 255), ornament=False, inset_outer=10, inset_inner=20)
    img.save(f"{out}/fantasy-card-dark.png")
    y = np.linspace(0, 1, S, dtype=np.float32)[:, None].repeat(S, 1)
    n = fbm(S, 5, 6, 31)
    img = to_image(lerp((20, 18, 16), (8, 7, 7), y * 0.7 + n * 0.3), np.full((S, S), 232, np.float32))
    bracket_corners(img, (176, 150, 100, 255), arm=44, dot=False)
    img.save(f"{out}/darkfantasy-card-dark.png")
    img = to_image(lerp((222, 219, 212), (196, 192, 184), y * 0.6 + n * 0.4), np.full((S, S), 236, np.float32))
    bracket_corners(img, (62, 58, 54, 255), arm=44, dot=False)
    img.save(f"{out}/darkfantasy-card-light.png")
    # 양피지 카드: 패널과 같은 종이, 선 없이(찢긴 가장자리는 얕게).
    rgb, alpha = paper((241, 226, 190), (128, 84, 44), 41, burn=55, torn=True, jag_amp=14)
    to_image(rgb, alpha).save(f"{out}/parchment-card-light.png")
    rgb, alpha = paper((198, 168, 120), (70, 44, 22), 51, burn=70, torn=True, jag_amp=14)
    to_image(rgb, alpha).save(f"{out}/parchment-card-dark.png")

if __name__ == "__main__":
    main(sys.argv[1])
