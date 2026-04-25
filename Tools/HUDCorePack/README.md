# WU HUD Core Pack

Dark fantasy HUD texture sources for UMG widgets. These are transparent PNGs intended to be imported as UI textures.

## Unreal Import Settings

- Compression Settings: `UserInterface2D (RGBA)`
- Texture Group: `UI`
- Mip Gen Settings: `NoMipmaps`
- sRGB: enabled for color textures
- Use UMG `Draw As: Box` for files ending in `_9Slice`
- Use UMG `Draw As: Image` for fixed frames, fills, masks, and markers

## Suggested 9-Slice Margins

UMG Brush margins are normalized, not pixel values. Type the normalized values below into the Brush Margin fields.

| Texture | UMG Draw As | Source Margin | UMG Margin |
| --- | --- | --- |
| `T_HUD_Panel_Large_9Slice.png` | Box | 32 px all sides | L/R `0.0625`, T/B `0.1250` |
| `T_HUD_Panel_Compact_9Slice.png` | Box | 24 px all sides | L/R `0.0750`, T/B `0.1500` |
| `T_HUD_ChatPanel_9Slice.png` | Box | 28 px all sides | L/R `0.0547`, T/B `0.1094` |
| `T_HUD_Tooltip_9Slice.png` | Box | 24 px all sides | L/R `0.0667`, T/B `0.1333` |
| `T_HUD_MinimapFrame_Rect_9Slice.png` | Box | 24 px all sides | L/R `0.0667`, T/B `0.0984` |
| `T_HUD_SideSlotColumn_Frame_9Slice.png` | Box | 24 px all sides | L/R `0.2500`, T/B `0.0469` |
| `T_HUD_Tab_Normal_9Slice.png` | Box | 18 px left/right, 12 px top/bottom | L/R `0.1125`, T/B `0.2500` |
| `T_HUD_Tab_Active_9Slice.png` | Box | 18 px left/right, 12 px top/bottom | L/R `0.1125`, T/B `0.2500` |

## Widget Assembly Notes

- Action buttons: put the ability or item icon at the bottom of an Overlay, then place the slot frame on top.
- Cooldowns: `T_HUD_ActionSlot_CooldownOverlay.png` is a static dim overlay. For a real radial cooldown, use it as visual reference and drive a UI material.
- Bars: place the fill texture under `T_HUD_BarFrame_Player.png` or `T_HUD_BarFrame_Target.png`. The fill inset is roughly 18 px from the left/right and 17 px from the top/bottom.
- Portraits: place the portrait image under the circular frame. Use a circular mask material if the portrait source is square.
- Minimap: use `T_HUD_MinimapMask_RoundedSquare.png` as a mask if your minimap render target needs clipped corners.
- Winged boars: prefer the separated pieces for layout work. Use `T_HUD_BottomBar_Rail.png` as the center rail, then place `T_HUD_Ornament_WingedBoar_Left.png` and `T_HUD_Ornament_WingedBoar_Right.png` independently at the hotbar ends. Scale the boars uniformly; do not stretch only one axis. `T_HUD_BottomBar_WingedBoars.png` is kept as an older combined comparison asset.

`HUD_CorePack_Manifest.json` contains the same data in machine-readable form.
`Preview/HUD_CorePack_Preview.png` is a quick contact sheet for browsing the pack outside Unreal.
