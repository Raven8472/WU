r"""
Run this inside the Unreal Editor Python console to apply UI texture settings
to every texture in /Game/UI/HUD/CorePack.

Unreal Python console command:
exec(open(r"C:\Users\raven\Documents\Unreal Projects\WU\Tools\SetHudCorePackTextureSettings.py").read())
"""

import unreal


CORE_PACK_PATH = "/Game/UI/HUD/CorePack"


def set_hud_core_pack_texture_settings() -> None:
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    assets = registry.get_assets_by_path(CORE_PACK_PATH, recursive=False)

    textures = []
    for asset_data in assets:
        asset = asset_data.get_asset()
        if isinstance(asset, unreal.Texture2D):
            textures.append(asset)

    if not textures:
        unreal.log_warning(f"No Texture2D assets found in {CORE_PACK_PATH}")
        return

    for texture in textures:
        texture.set_editor_property(
            "compression_settings",
            unreal.TextureCompressionSettings.TC_EDITOR_ICON,
        )
        texture.set_editor_property("lod_group", unreal.TextureGroup.TEXTUREGROUP_UI)
        texture.set_editor_property(
            "mip_gen_settings",
            unreal.TextureMipGenSettings.TMGS_NO_MIPMAPS,
        )
        texture.set_editor_property("srgb", True)
        texture.modify()
        unreal.EditorAssetLibrary.save_loaded_asset(texture)

    unreal.log(f"Applied UI texture settings to {len(textures)} textures in {CORE_PACK_PATH}")


set_hud_core_pack_texture_settings()
