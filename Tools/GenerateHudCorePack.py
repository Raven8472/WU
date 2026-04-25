from __future__ import annotations

import json
import math
import random
import zlib
from dataclasses import dataclass
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageEnhance, ImageFilter, ImageOps


ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "Content" / "UI" / "HUD" / "CorePack"
DOC_OUT = ROOT / "Tools" / "HUDCorePack"
SOURCE_ASSETS = DOC_OUT / "SourceAssets"
PREVIEW = DOC_OUT / "Preview"


SCALE = 3


def rgba(hex_color: str, alpha: int = 255) -> tuple[int, int, int, int]:
    hex_color = hex_color.strip("#")
    return (
        int(hex_color[0:2], 16),
        int(hex_color[2:4], 16),
        int(hex_color[4:6], 16),
        alpha,
    )


PALETTE = {
    "void": rgba("050505", 0),
    "ink": rgba("050607", 232),
    "ink_soft": rgba("07090a", 190),
    "carbon": rgba("121313", 245),
    "carbon_2": rgba("1b1b1a", 245),
    "steel_dark": rgba("272725", 255),
    "steel": rgba("4b4a43", 255),
    "steel_light": rgba("878173", 255),
    "brass_dark": rgba("6d4a13", 255),
    "brass": rgba("b98a27", 255),
    "brass_light": rgba("f0cc61", 255),
    "shadow": rgba("000000", 180),
    "health": rgba("a22018", 255),
    "health_hi": rgba("f05237", 255),
    "stamina": rgba("18943b", 255),
    "stamina_hi": rgba("61e37a", 255),
    "mana": rgba("1769c8", 255),
    "mana_hi": rgba("65d7ff", 255),
    "xp": rgba("ba861d", 255),
    "xp_hi": rgba("ffcf4a", 255),
}


@dataclass
class AssetMeta:
    file: str
    size: tuple[int, int]
    use: str
    umg_draw_as: str
    margin: str = ""
    umg_margin: str = ""
    notes: str = ""


manifest: list[AssetMeta] = []


def stable_seed(text: str) -> int:
    return zlib.crc32(text.encode("utf-8")) & 0xFFFFFFFF


def canvas(w: int, h: int) -> Image.Image:
    return Image.new("RGBA", (w * SCALE, h * SCALE), (0, 0, 0, 0))


def downsample(img: Image.Image) -> Image.Image:
    return img.resize((img.width // SCALE, img.height // SCALE), Image.Resampling.LANCZOS)


def trim_alpha(img: Image.Image, padding: int = 0) -> Image.Image:
    alpha = img.getchannel("A")
    bbox = alpha.getbbox()
    if not bbox:
        return img
    left, top, right, bottom = bbox
    left = max(0, left - padding)
    top = max(0, top - padding)
    right = min(img.width, right + padding)
    bottom = min(img.height, bottom + padding)
    return img.crop((left, top, right, bottom))


def fit_image(img: Image.Image, max_w: int, max_h: int) -> Image.Image:
    scale = min(max_w / img.width, max_h / img.height)
    size = (max(1, int(img.width * scale)), max(1, int(img.height * scale)))
    return img.resize(size, Image.Resampling.LANCZOS)


def color_grade_stone(img: Image.Image) -> Image.Image:
    alpha = img.getchannel("A")
    gray = ImageOps.grayscale(img)
    gray = ImageOps.autocontrast(gray, cutoff=1)
    gray = ImageEnhance.Contrast(gray).enhance(1.22)
    gray = ImageEnhance.Sharpness(gray).enhance(1.35)
    colorized = ImageOps.colorize(gray, black="#222723", mid="#687064", white="#c2bfa9")
    graded = Image.merge("RGBA", (*colorized.split(), alpha))
    return graded.filter(ImageFilter.UnsharpMask(radius=1.1, percent=155, threshold=2))


def add_alpha_stroke(img: Image.Image, color: tuple[int, int, int, int], radius: int, blur: float = 0.0) -> Image.Image:
    alpha = img.getchannel("A")
    grown = alpha.filter(ImageFilter.MaxFilter(radius * 2 + 1))
    stroke_mask = ImageChops.subtract(grown, alpha)
    if blur:
        stroke_mask = stroke_mask.filter(ImageFilter.GaussianBlur(blur))
    stroke = Image.new("RGBA", img.size, color)
    stroke.putalpha(stroke_mask)
    out = Image.new("RGBA", img.size, (0, 0, 0, 0))
    out.alpha_composite(stroke)
    out.alpha_composite(img)
    return out


def drop_shadow(img: Image.Image, offset: tuple[int, int], blur: float, alpha: int) -> Image.Image:
    shadow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    mask = img.getchannel("A").filter(ImageFilter.GaussianBlur(blur))
    shadow_color = Image.new("RGBA", img.size, (0, 0, 0, alpha))
    shadow_color.putalpha(mask.point(lambda p: min(alpha, p)))
    shadow.alpha_composite(shadow_color, offset)
    out = Image.new("RGBA", img.size, (0, 0, 0, 0))
    out.alpha_composite(shadow)
    out.alpha_composite(img)
    return out


def draw_texture(draw: ImageDraw.ImageDraw, box: tuple[int, int, int, int], seed: int, opacity: int = 26) -> None:
    rnd = random.Random(seed)
    x1, y1, x2, y2 = box
    for _ in range(max(18, (x2 - x1) * (y2 - y1) // 2600)):
        x = rnd.randint(x1, max(x1, x2 - 1))
        y = rnd.randint(y1, max(y1, y2 - 1))
        ln = rnd.randint(4 * SCALE, 24 * SCALE)
        col = rnd.choice(
            [
                (255, 231, 160, opacity),
                (0, 0, 0, opacity + 10),
                (130, 120, 96, opacity),
            ]
        )
        draw.line((x, y, min(x2, x + ln), y + rnd.randint(-2 * SCALE, 2 * SCALE)), fill=col, width=1 * SCALE)


def inset(box: tuple[int, int, int, int], n: int) -> tuple[int, int, int, int]:
    n *= SCALE
    return (box[0] + n, box[1] + n, box[2] - n, box[3] - n)


def rect_scaled(x: int, y: int, w: int, h: int) -> tuple[int, int, int, int]:
    return (x * SCALE, y * SCALE, (x + w) * SCALE, (y + h) * SCALE)


def rounded_panel(name: str, w: int, h: int, radius: int, margin: int, fill_alpha: int, seed: int, accent: bool = True) -> None:
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    outer = rect_scaled(2, 2, w - 4, h - 4)
    r = radius * SCALE

    shadow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow, "RGBA")
    sd.rounded_rectangle((outer[0] + 5 * SCALE, outer[1] + 6 * SCALE, outer[2] + 5 * SCALE, outer[3] + 8 * SCALE), r, fill=(0, 0, 0, 130))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(5 * SCALE)))

    d.rounded_rectangle(outer, r, fill=PALETTE["ink"][:3] + (fill_alpha,), outline=PALETTE["shadow"], width=2 * SCALE)
    draw_texture(d, outer, seed, 20)

    d.rounded_rectangle(inset(outer, 3), max(1, r - 3 * SCALE), outline=PALETTE["steel_dark"], width=3 * SCALE)
    d.rounded_rectangle(inset(outer, 7), max(1, r - 7 * SCALE), outline=PALETTE["steel"], width=1 * SCALE)
    d.rounded_rectangle(inset(outer, 10), max(1, r - 10 * SCALE), outline=(0, 0, 0, 160), width=2 * SCALE)

    if accent:
        for x in (14, w - 22):
            d.polygon(
                [
                    ((x + 4) * SCALE, 9 * SCALE),
                    ((x + 10) * SCALE, 15 * SCALE),
                    ((x + 4) * SCALE, 21 * SCALE),
                    ((x - 2) * SCALE, 15 * SCALE),
                ],
                fill=PALETTE["brass"],
                outline=PALETTE["brass_light"],
            )
        d.line((24 * SCALE, 14 * SCALE, (w - 24) * SCALE, 14 * SCALE), fill=PALETTE["brass_dark"], width=1 * SCALE)
        d.line((24 * SCALE, (h - 15) * SCALE, (w - 24) * SCALE, (h - 15) * SCALE), fill=(80, 58, 22, 180), width=1 * SCALE)

    out = OUT / f"{name}.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Resizable dark fantasy panel/backplate",
            umg_draw_as="Box",
            margin=f"{margin}px all sides",
            umg_margin=f"Left/Right {margin / w:.4f}, Top/Bottom {margin / h:.4f}",
            notes="Transparent corners; set texture group to UI and compression to UserInterface2D.",
        )
    )


def action_slot(name: str, state: str, glow: tuple[int, int, int, int] | None = None) -> None:
    w = h = 96
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    outer = rect_scaled(4, 4, 88, 88)

    if glow:
        glow_layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
        gd = ImageDraw.Draw(glow_layer, "RGBA")
        gd.rounded_rectangle(rect_scaled(2, 2, 92, 92), 8 * SCALE, outline=glow, width=5 * SCALE)
        img.alpha_composite(glow_layer.filter(ImageFilter.GaussianBlur(3 * SCALE)))

    inner_alpha = 238 if state != "Disabled" else 180
    d.rounded_rectangle(outer, 5 * SCALE, fill=(4, 4, 4, inner_alpha), outline=(0, 0, 0, 230), width=2 * SCALE)
    draw_texture(d, outer, stable_seed(name), 18)

    if state == "Pressed":
        border = PALETTE["steel_dark"]
        hi = rgba("332513", 255)
    elif state == "Disabled":
        border = rgba("3d3d3b", 210)
        hi = rgba("727066", 140)
    elif state == "Hover":
        border = PALETTE["brass"]
        hi = PALETTE["brass_light"]
    else:
        border = PALETTE["steel"]
        hi = PALETTE["steel_light"]

    d.rounded_rectangle(inset(outer, 3), 4 * SCALE, outline=border, width=4 * SCALE)
    d.rounded_rectangle(inset(outer, 9), 2 * SCALE, outline=(0, 0, 0, 190), width=2 * SCALE)
    d.line((15 * SCALE, 14 * SCALE, 80 * SCALE, 14 * SCALE), fill=hi, width=1 * SCALE)
    d.line((14 * SCALE, 15 * SCALE, 14 * SCALE, 80 * SCALE), fill=(255, 255, 255, 28), width=1 * SCALE)

    for cx, cy in ((15, 15), (81, 15), (15, 81), (81, 81)):
        d.ellipse(rect_scaled(cx - 4, cy - 4, 8, 8), fill=PALETTE["carbon_2"], outline=border, width=1 * SCALE)

    if state == "Disabled":
        d.rounded_rectangle(inset(outer, 12), 1 * SCALE, fill=(0, 0, 0, 105))
        d.line((22 * SCALE, 22 * SCALE, 74 * SCALE, 74 * SCALE), fill=(145, 145, 135, 95), width=4 * SCALE)

    out = OUT / f"{name}.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use=f"Action slot {state.lower()} state",
            umg_draw_as="Image",
            notes="Use as an Overlay frame above ability/item icon.",
        )
    )


def cooldown_overlay() -> None:
    w = h = 96
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    cx = cy = w * SCALE // 2
    r = 39 * SCALE
    d.pieslice((cx - r, cy - r, cx + r, cy + r), start=-90, end=135, fill=(0, 0, 0, 160))
    d.rounded_rectangle(rect_scaled(6, 6, 84, 84), 4 * SCALE, outline=(255, 255, 255, 26), width=1 * SCALE)
    out = OUT / "T_HUD_ActionSlot_CooldownOverlay.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Static cooldown/dim overlay",
            umg_draw_as="Image",
            notes="For a true radial cooldown, use this as a material reference or mask source.",
        )
    )


def bar_fill(name: str, w: int, h: int, base: tuple[int, int, int, int], hi: tuple[int, int, int, int]) -> None:
    img = canvas(w, h)
    pix = img.load()
    rnd = random.Random(stable_seed(name))
    for y in range(h * SCALE):
        t = y / max(1, h * SCALE - 1)
        for x in range(w * SCALE):
            band = 0.5 + 0.5 * math.sin((x / SCALE) * 0.08 + (y / SCALE) * 0.17)
            gleam = max(0.0, 1.0 - abs(t - 0.28) * 6.0)
            noise = rnd.randint(-8, 8)
            r = int(base[0] * (1 - gleam * 0.35) + hi[0] * gleam * 0.35 + band * 9 + noise)
            g = int(base[1] * (1 - gleam * 0.35) + hi[1] * gleam * 0.35 + band * 9 + noise)
            b = int(base[2] * (1 - gleam * 0.35) + hi[2] * gleam * 0.35 + band * 9 + noise)
            pix[x, y] = (max(0, min(255, r)), max(0, min(255, g)), max(0, min(255, b)), 255)
    d = ImageDraw.Draw(img, "RGBA")
    d.rectangle((0, 0, w * SCALE, 2 * SCALE), fill=(255, 255, 255, 70))
    d.rectangle((0, (h - 4) * SCALE, w * SCALE, h * SCALE), fill=(0, 0, 0, 75))
    out = OUT / f"{name}.png"
    final = downsample(img)
    final.putalpha(255)
    final.save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Progress bar fill texture",
            umg_draw_as="Image",
            notes="Tile horizontally or use as fill image in a Progress Bar style.",
        )
    )


def bar_frame(name: str, w: int, h: int, accent: tuple[int, int, int, int]) -> None:
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    outer = rect_scaled(2, 4, w - 4, h - 8)
    d.rounded_rectangle(outer, 8 * SCALE, fill=(0, 0, 0, 105), outline=(0, 0, 0, 230), width=2 * SCALE)
    d.rounded_rectangle(inset(outer, 4), 6 * SCALE, outline=PALETTE["steel_dark"], width=4 * SCALE)
    d.rounded_rectangle(inset(outer, 9), 4 * SCALE, outline=accent, width=2 * SCALE)
    d.rounded_rectangle(inset(outer, 14), 2 * SCALE, fill=(0, 0, 0, 190), outline=(255, 255, 255, 26), width=1 * SCALE)
    for x in (16, w - 28):
        d.polygon(
            [
                ((x + 6) * SCALE, 7 * SCALE),
                ((x + 15) * SCALE, h * SCALE // 2),
                ((x + 6) * SCALE, (h - 7) * SCALE),
                ((x - 2) * SCALE, h * SCALE // 2),
            ],
            fill=PALETTE["carbon_2"],
            outline=accent,
        )
    out = OUT / f"{name}.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Fixed health/resource bar frame",
            umg_draw_as="Image",
            notes="Place progress fill beneath this frame, inset about 18px left/right and 17px top/bottom.",
        )
    )


def portrait_frame(name: str, size: int, accent: tuple[int, int, int, int]) -> None:
    img = canvas(size, size)
    d = ImageDraw.Draw(img, "RGBA")
    cx = cy = size * SCALE // 2
    shadow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow, "RGBA")
    sd.ellipse((12 * SCALE, 13 * SCALE, (size - 8) * SCALE, (size - 7) * SCALE), fill=(0, 0, 0, 130))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(4 * SCALE)))
    for radius, width, col in (
        (size // 2 - 8, 6, PALETTE["steel_dark"]),
        (size // 2 - 15, 4, PALETTE["steel"]),
        (size // 2 - 21, 3, accent),
        (size // 2 - 28, 2, (0, 0, 0, 210)),
    ):
        d.ellipse((cx - radius * SCALE, cy - radius * SCALE, cx + radius * SCALE, cy + radius * SCALE), outline=col, width=width * SCALE)
    for angle in (45, 135, 225, 315):
        a = math.radians(angle)
        x = cx + int(math.cos(a) * (size // 2 - 14) * SCALE)
        y = cy + int(math.sin(a) * (size // 2 - 14) * SCALE)
        d.polygon(
            [
                (x, y - 8 * SCALE),
                (x + 7 * SCALE, y),
                (x, y + 8 * SCALE),
                (x - 7 * SCALE, y),
            ],
            fill=PALETTE["carbon_2"],
            outline=accent,
        )
    out = OUT / f"{name}.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(size, size),
            use="Circular player/target portrait frame",
            umg_draw_as="Image",
            notes="Place portrait image under the frame using a circular material mask.",
        )
    )


def minimap_frame() -> None:
    w, h = 360, 244
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    outer = rect_scaled(3, 3, w - 6, h - 6)
    d.rounded_rectangle(outer, 5 * SCALE, fill=(0, 0, 0, 95), outline=(0, 0, 0, 240), width=3 * SCALE)
    d.rounded_rectangle(inset(outer, 5), 2 * SCALE, outline=PALETTE["steel_dark"], width=5 * SCALE)
    d.rounded_rectangle(inset(outer, 12), 1 * SCALE, outline=PALETTE["brass_dark"], width=2 * SCALE)
    d.rectangle(rect_scaled(16, 17, w - 32, 27), fill=(0, 0, 0, 120))
    d.line((20 * SCALE, 28 * SCALE, (w - 20) * SCALE, 28 * SCALE), fill=PALETTE["brass"], width=1 * SCALE)
    for x, y in ((20, 18), (w - 30, 18), (20, h - 30), (w - 30, h - 30)):
        d.rectangle(rect_scaled(x, y, 10, 10), fill=PALETTE["carbon_2"], outline=PALETTE["brass"], width=1 * SCALE)
    out = OUT / "T_HUD_MinimapFrame_Rect_9Slice.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Resizable minimap frame",
            umg_draw_as="Box",
            margin="24px all sides",
            umg_margin=f"Left/Right {24 / w:.4f}, Top/Bottom {24 / h:.4f}",
            notes="Header band expects map name/time as separate UMG text.",
        )
    )


def minimap_mask() -> None:
    size = 256
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    d = ImageDraw.Draw(img, "RGBA")
    d.rounded_rectangle((8, 8, size - 8, size - 8), 6, fill=(255, 255, 255, 255))
    img.save(OUT / "T_HUD_MinimapMask_RoundedSquare.png")
    manifest.append(
        AssetMeta(
            file="T_HUD_MinimapMask_RoundedSquare.png",
            size=(size, size),
            use="Minimap material alpha mask",
            umg_draw_as="Image",
            notes="Use as mask input if your minimap render target needs clipped corners.",
        )
    )


def tab(name: str, active: bool) -> None:
    w, h = 160, 48
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    outer = rect_scaled(3, 7, w - 6, h - 10)
    fill = rgba("19160d", 235) if active else rgba("0b0c0d", 210)
    edge = PALETTE["brass"] if active else PALETTE["steel"]
    d.rounded_rectangle(outer, 5 * SCALE, fill=fill, outline=(0, 0, 0, 220), width=2 * SCALE)
    d.rounded_rectangle(inset(outer, 4), 3 * SCALE, outline=edge, width=2 * SCALE)
    d.line((16 * SCALE, 14 * SCALE, (w - 16) * SCALE, 14 * SCALE), fill=PALETTE["brass_light"] if active else (255, 255, 255, 35), width=1 * SCALE)
    out = OUT / f"{name}.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Chat/log tab state",
            umg_draw_as="Box",
            margin="18px left/right, 12px top/bottom",
            umg_margin=f"Left/Right {18 / w:.4f}, Top/Bottom {12 / h:.4f}",
        )
    )


def separator(name: str, horizontal: bool = True) -> None:
    w, h = (512, 12) if horizontal else (12, 512)
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    if horizontal:
        y = h * SCALE // 2
        d.line((12 * SCALE, y, (w - 12) * SCALE, y), fill=(0, 0, 0, 150), width=3 * SCALE)
        d.line((12 * SCALE, y - 2 * SCALE, (w - 12) * SCALE, y - 2 * SCALE), fill=PALETTE["steel"], width=1 * SCALE)
        d.line((12 * SCALE, y + 2 * SCALE, (w - 12) * SCALE, y + 2 * SCALE), fill=PALETTE["brass_dark"], width=1 * SCALE)
    else:
        x = w * SCALE // 2
        d.line((x, 12 * SCALE, x, (h - 12) * SCALE), fill=(0, 0, 0, 150), width=3 * SCALE)
        d.line((x - 2 * SCALE, 12 * SCALE, x - 2 * SCALE, (h - 12) * SCALE), fill=PALETTE["steel"], width=1 * SCALE)
        d.line((x + 2 * SCALE, 12 * SCALE, x + 2 * SCALE, (h - 12) * SCALE), fill=PALETTE["brass_dark"], width=1 * SCALE)
    out = OUT / f"{name}.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Thin divider/separator",
            umg_draw_as="Image",
        )
    )


def quest_marker() -> None:
    size = 48
    img = canvas(size, size)
    d = ImageDraw.Draw(img, "RGBA")
    shadow = Image.new("RGBA", img.size, (0, 0, 0, 0))
    sd = ImageDraw.Draw(shadow, "RGBA")
    sd.polygon([(24 * SCALE, 6 * SCALE), (42 * SCALE, 24 * SCALE), (24 * SCALE, 42 * SCALE), (6 * SCALE, 24 * SCALE)], fill=(0, 0, 0, 120))
    img.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(2 * SCALE)))
    d.polygon([(24 * SCALE, 5 * SCALE), (43 * SCALE, 24 * SCALE), (24 * SCALE, 43 * SCALE), (5 * SCALE, 24 * SCALE)], fill=PALETTE["brass"], outline=PALETTE["brass_light"])
    d.polygon([(24 * SCALE, 13 * SCALE), (35 * SCALE, 24 * SCALE), (24 * SCALE, 35 * SCALE), (13 * SCALE, 24 * SCALE)], fill=rgba("3a2508", 255), outline=PALETTE["brass_light"])
    out = OUT / "T_HUD_QuestMarker_Diamond.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(size, size),
            use="Quest bullet/marker",
            umg_draw_as="Image",
        )
    )


def bottom_filigree() -> None:
    w, h = 960, 180
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    base_y = 132 * SCALE
    d.line((80 * SCALE, base_y, (w - 80) * SCALE, base_y), fill=(0, 0, 0, 180), width=5 * SCALE)
    d.line((100 * SCALE, (base_y - 6 * SCALE), (w - 100) * SCALE, (base_y - 6 * SCALE)), fill=PALETTE["steel"], width=2 * SCALE)
    d.line((132 * SCALE, (base_y + 7 * SCALE), (w - 132) * SCALE, (base_y + 7 * SCALE)), fill=PALETTE["brass_dark"], width=2 * SCALE)
    for side in (-1, 1):
        origin_x = (w // 2 + side * 385) * SCALE
        for i in range(7):
            length = (126 - i * 12) * SCALE
            y = (126 - i * 11) * SCALE
            x2 = origin_x + side * length
            d.line((origin_x, base_y - i * 5 * SCALE, x2, y), fill=PALETTE["steel_dark"], width=(10 - i) * SCALE)
            d.line((origin_x, base_y - i * 5 * SCALE, x2, y), fill=PALETTE["steel_light"], width=max(1, (3 - i // 3)) * SCALE)
        d.ellipse((origin_x - 28 * SCALE, base_y - 28 * SCALE, origin_x + 28 * SCALE, base_y + 28 * SCALE), fill=PALETTE["carbon_2"], outline=PALETTE["steel_light"], width=3 * SCALE)
        d.polygon(
            [
                (origin_x + side * 18 * SCALE, base_y),
                (origin_x + side * 55 * SCALE, base_y - 18 * SCALE),
                (origin_x + side * 52 * SCALE, base_y + 17 * SCALE),
            ],
            fill=PALETTE["steel_dark"],
            outline=PALETTE["steel_light"],
        )
    out = OUT / "T_HUD_BottomBar_Filigree.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Decorative bottom action-bar flourish",
            umg_draw_as="Image",
            notes="Optional decoration behind the center hotbar; keep separate from interactive slots.",
        )
    )


def load_boar_asset(name: str) -> Image.Image | None:
    path = SOURCE_ASSETS / name
    if not path.exists():
        return None
    return Image.open(path).convert("RGBA")


def process_boar(name: str, out_name: str, max_size: tuple[int, int], canvas_size: tuple[int, int] = (256, 256)) -> Image.Image | None:
    source = load_boar_asset(name)
    if source is None:
        return None

    boar = trim_alpha(source, padding=4)
    boar = color_grade_stone(boar)
    boar = fit_image(boar, max_size[0], max_size[1])
    boar = add_alpha_stroke(boar, PALETTE["steel_dark"], radius=2, blur=0.4)
    boar = add_alpha_stroke(boar, (0, 0, 0, 190), radius=4, blur=1.2)

    ornament = Image.new("RGBA", canvas_size, (0, 0, 0, 0))
    x = (ornament.width - boar.width) // 2
    y = ornament.height - boar.height - max(4, ornament.height // 32)
    ornament.alpha_composite(drop_shadow(boar, (3, 6), 5, 120), (x, y))

    out = OUT / out_name
    ornament.save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=ornament.size,
            use="Winged boar HUD ornament",
            umg_draw_as="Image",
            notes="Fixed-size ornament. Use Size To Content or keep width and height equal when scaling.",
        )
    )
    return ornament


def bottom_winged_boars() -> None:
    left = process_boar("LeftBoar.PNG", "T_HUD_Ornament_WingedBoar_Left.png", (102, 108), (120, 120))
    right = process_boar("RightBoar.png", "T_HUD_Ornament_WingedBoar_Right.png", (102, 108), (120, 120))
    if left is None or right is None:
        return

    bottom_bar_rail()

    w, h = 1280, 120
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    base_y = 82 * SCALE

    # Compact rails designed to sit just behind the hotbar baseline, like WoW's gryphon bookends.
    d.line((116 * SCALE, base_y, (w - 116) * SCALE, base_y), fill=(0, 0, 0, 205), width=7 * SCALE)
    d.line((136 * SCALE, (base_y - 7 * SCALE), (w - 136) * SCALE, (base_y - 7 * SCALE)), fill=PALETTE["steel"], width=3 * SCALE)
    d.line((136 * SCALE, (base_y + 7 * SCALE), (w - 136) * SCALE, (base_y + 7 * SCALE)), fill=PALETTE["brass_dark"], width=3 * SCALE)
    d.line((168 * SCALE, (base_y - 13 * SCALE), (w - 168) * SCALE, (base_y - 13 * SCALE)), fill=(245, 213, 111, 115), width=1 * SCALE)
    d.line((168 * SCALE, (base_y + 13 * SCALE), (w - 168) * SCALE, (base_y + 13 * SCALE)), fill=(0, 0, 0, 145), width=2 * SCALE)

    for x in (128, w - 148):
        d.ellipse(rect_scaled(x - 8, 74, 16, 16), fill=PALETTE["carbon_2"], outline=PALETTE["steel_light"], width=2 * SCALE)
        d.polygon(
            [
                ((x + 8) * SCALE, base_y),
                ((x + 28) * SCALE, (base_y - 13 * SCALE)),
                ((x + 28) * SCALE, (base_y + 13 * SCALE)),
            ],
            fill=PALETTE["steel_dark"],
            outline=PALETTE["steel_light"],
        )

    left_large = left.resize((120 * SCALE, 120 * SCALE), Image.Resampling.LANCZOS)
    right_large = right.resize((120 * SCALE, 120 * SCALE), Image.Resampling.LANCZOS)
    img.alpha_composite(left_large, (0, 0))
    img.alpha_composite(right_large, ((w - 120) * SCALE, 0))

    out = OUT / "T_HUD_BottomBar_WingedBoars.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Decorative bottom action-bar flourish with winged boar ornaments",
            umg_draw_as="Image",
            notes="Anchor bottom center behind the hotbar. Scale uniformly; do not stretch X/Y independently.",
        )
    )


def bottom_bar_rail() -> None:
    w, h = 1040, 40
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    base_y = 21 * SCALE

    # Rail-only version so the boar ornaments can be positioned independently.
    d.line((12 * SCALE, base_y, (w - 12) * SCALE, base_y), fill=(0, 0, 0, 210), width=8 * SCALE)
    d.line((22 * SCALE, (base_y - 8 * SCALE), (w - 22) * SCALE, (base_y - 8 * SCALE)), fill=PALETTE["steel"], width=3 * SCALE)
    d.line((22 * SCALE, (base_y + 8 * SCALE), (w - 22) * SCALE, (base_y + 8 * SCALE)), fill=PALETTE["brass_dark"], width=3 * SCALE)
    d.line((48 * SCALE, (base_y - 14 * SCALE), (w - 48) * SCALE, (base_y - 14 * SCALE)), fill=(245, 213, 111, 120), width=1 * SCALE)
    d.line((48 * SCALE, (base_y + 14 * SCALE), (w - 48) * SCALE, (base_y + 14 * SCALE)), fill=(0, 0, 0, 145), width=2 * SCALE)

    for x, direction in ((26, -1), (w - 26, 1)):
        d.ellipse(rect_scaled(x - 8, 13, 16, 16), fill=PALETTE["carbon_2"], outline=PALETTE["steel_light"], width=2 * SCALE)
        d.polygon(
            [
                ((x - direction * 8) * SCALE, base_y),
                ((x - direction * 28) * SCALE, (base_y - 13 * SCALE)),
                ((x - direction * 28) * SCALE, (base_y + 13 * SCALE)),
            ],
            fill=PALETTE["steel_dark"],
            outline=PALETTE["steel_light"],
        )

    out = OUT / "T_HUD_BottomBar_Rail.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Rail-only bottom action-bar ornament",
            umg_draw_as="Image",
            notes="Use separately from the winged boars. Anchor bottom center behind the hotbar and scale X only if needed.",
        )
    )


def side_column_frame() -> None:
    w, h = 96, 512
    img = canvas(w, h)
    d = ImageDraw.Draw(img, "RGBA")
    outer = rect_scaled(4, 3, w - 8, h - 6)
    d.rounded_rectangle(outer, 5 * SCALE, fill=(0, 0, 0, 112), outline=(0, 0, 0, 235), width=2 * SCALE)
    d.rounded_rectangle(inset(outer, 5), 3 * SCALE, outline=PALETTE["steel_dark"], width=4 * SCALE)
    d.rounded_rectangle(inset(outer, 12), 1 * SCALE, outline=PALETTE["brass_dark"], width=1 * SCALE)
    for y in range(64, h - 24, 64):
        d.line((14 * SCALE, y * SCALE, (w - 14) * SCALE, y * SCALE), fill=(255, 255, 255, 25), width=1 * SCALE)
    out = OUT / "T_HUD_SideSlotColumn_Frame_9Slice.png"
    downsample(img).save(out)
    manifest.append(
        AssetMeta(
            file=out.name,
            size=(w, h),
            use="Resizable side hotbar/inventory column backplate",
            umg_draw_as="Box",
            margin="24px all sides",
            umg_margin=f"Left/Right {24 / w:.4f}, Top/Bottom {24 / h:.4f}",
        )
    )


def preview_sheet() -> None:
    files = [m.file for m in manifest if m.file.endswith(".png") and not m.file.startswith("T_HUD_Bar_Fill")]
    thumbs: list[tuple[str, Image.Image]] = []
    for file in files:
        im = Image.open(OUT / file).convert("RGBA")
        im.thumbnail((220, 130), Image.Resampling.LANCZOS)
        thumbs.append((file, im))

    cell_w, cell_h = 260, 178
    cols = 4
    rows = math.ceil(len(thumbs) / cols)
    sheet = Image.new("RGBA", (cols * cell_w, rows * cell_h), rgba("111315", 255))
    d = ImageDraw.Draw(sheet, "RGBA")
    for idx, (file, im) in enumerate(thumbs):
        c = idx % cols
        r = idx // cols
        x = c * cell_w
        y = r * cell_h
        d.rectangle((x + 8, y + 8, x + cell_w - 8, y + cell_h - 8), fill=rgba("181a1c", 255), outline=rgba("3a3325", 255))
        checker = Image.new("RGBA", (cell_w - 28, cell_h - 54), rgba("262626", 255))
        cd = ImageDraw.Draw(checker, "RGBA")
        for yy in range(0, checker.height, 16):
            for xx in range(0, checker.width, 16):
                if (xx // 16 + yy // 16) % 2 == 0:
                    cd.rectangle((xx, yy, xx + 15, yy + 15), fill=rgba("303030", 255))
        px = x + 14 + (checker.width - im.width) // 2
        py = y + 14 + (checker.height - im.height) // 2
        sheet.alpha_composite(checker, (x + 14, y + 14))
        sheet.alpha_composite(im, (px, py))
        d.text((x + 14, y + cell_h - 34), file.replace("T_HUD_", "").replace(".png", ""), fill=rgba("d9c78f", 255))
    PREVIEW.mkdir(parents=True, exist_ok=True)
    sheet.convert("RGB").save(PREVIEW / "HUD_CorePack_Preview.png")


def write_manifest() -> None:
    data = [
        {
            "file": item.file,
            "size": item.size,
            "use": item.use,
            "umg_draw_as": item.umg_draw_as,
            "margin": item.margin,
            "umg_margin": item.umg_margin,
            "notes": item.notes,
        }
        for item in manifest
    ]
    DOC_OUT.mkdir(parents=True, exist_ok=True)
    (DOC_OUT / "HUD_CorePack_Manifest.json").write_text(json.dumps(data, indent=2), encoding="utf-8")


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    DOC_OUT.mkdir(parents=True, exist_ok=True)
    PREVIEW.mkdir(parents=True, exist_ok=True)

    rounded_panel("T_HUD_Panel_Large_9Slice", 512, 256, 10, 32, 215, 101)
    rounded_panel("T_HUD_Panel_Compact_9Slice", 320, 160, 8, 24, 222, 102)
    rounded_panel("T_HUD_ChatPanel_9Slice", 512, 256, 8, 28, 178, 103)
    rounded_panel("T_HUD_Tooltip_9Slice", 360, 180, 8, 24, 232, 104)

    action_slot("T_HUD_ActionSlot_Normal", "Normal")
    action_slot("T_HUD_ActionSlot_Hover", "Hover", glow=rgba("edb83e", 130))
    action_slot("T_HUD_ActionSlot_Pressed", "Pressed")
    action_slot("T_HUD_ActionSlot_Disabled", "Disabled")
    cooldown_overlay()

    bar_frame("T_HUD_BarFrame_Player", 392, 70, PALETTE["brass_dark"])
    bar_frame("T_HUD_BarFrame_Target", 360, 58, rgba("8f231e", 255))
    bar_fill("T_HUD_Bar_Fill_Health", 512, 32, PALETTE["health"], PALETTE["health_hi"])
    bar_fill("T_HUD_Bar_Fill_Stamina", 512, 32, PALETTE["stamina"], PALETTE["stamina_hi"])
    bar_fill("T_HUD_Bar_Fill_Mana", 512, 32, PALETTE["mana"], PALETTE["mana_hi"])
    bar_fill("T_HUD_Bar_Fill_XP", 512, 18, PALETTE["xp"], PALETTE["xp_hi"])

    portrait_frame("T_HUD_PortraitFrame_Player", 160, PALETTE["brass"])
    portrait_frame("T_HUD_PortraitFrame_Target", 144, rgba("9c2a22", 255))
    minimap_frame()
    minimap_mask()
    side_column_frame()

    tab("T_HUD_Tab_Normal_9Slice", False)
    tab("T_HUD_Tab_Active_9Slice", True)
    separator("T_HUD_Separator_Horizontal")
    separator("T_HUD_Separator_Vertical", horizontal=False)
    quest_marker()
    bottom_filigree()
    bottom_winged_boars()

    write_manifest()
    preview_sheet()


if __name__ == "__main__":
    main()
