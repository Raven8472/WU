import json
import os
from collections import Counter

import unreal


EXTERNAL_ACTOR_PATH = "/Game/__ExternalActors__/ThirdPerson/Lvl_MagicalBritain_Persistent"
OUT_DIR = "/Game/World/Buildings/Materials"
MASTER_NAME = "M_WU_Church_TexturedLite"
GLASS_MASTER_PATH = "/Game/World/Buildings/Materials/M_WU_Building_Glass_TwoSidedLite.M_WU_Building_Glass_TwoSidedLite"
CHURCH_GLASS_NAME = "MI_WU_Church_Glass_Dark"

CHURCH_CLASS_NAME = "BP_Church_03_Interior_C"

WHITE_TEXTURE = "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Textures/Generic/T_White_D.T_White_D"

CHURCH_MATERIALS = {
    "MI_WU_Church_StoneWall_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Textures/Stone/Stone_Wall_04/T_Stone_Wall_04_D.T_Stone_Wall_04_D",
        "tint": (0.78, 0.73, 0.64, 1.0),
        "brightness": 0.92,
        "roughness": 0.96,
    },
    "MI_WU_Church_StoneBrown_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Textures/Stone/Stone_Wall_01/T_Stone_Wall_01_D.T_Stone_Wall_01_D",
        "tint": (0.62, 0.46, 0.31, 1.0),
        "brightness": 0.86,
        "roughness": 0.96,
    },
    "MI_WU_Church_RoofTile_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Textures/Roof/T_Tiles_03_D.T_Tiles_03_D",
        "tint": (0.78, 0.42, 0.30, 1.0),
        "brightness": 0.92,
        "roughness": 0.94,
    },
    "MI_WU_Church_WoodPlain_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Textures/Wood/Wood_04/T_Wood_04_D.T_Wood_04_D",
        "tint": (0.72, 0.55, 0.36, 1.0),
        "brightness": 0.9,
        "roughness": 0.9,
    },
    "MI_WU_Church_WoodLight_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Textures/Wood/Wood_03/T_Wood_03_D.T_Wood_03_D",
        "tint": (0.78, 0.56, 0.36, 1.0),
        "brightness": 0.9,
        "roughness": 0.88,
    },
    "MI_WU_Church_MetalDull_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Textures/Metal/Metal_04/T_Metal_04_D.T_Metal_04_D",
        "tint": (0.48, 0.48, 0.46, 1.0),
        "brightness": 0.72,
        "roughness": 0.93,
    },
    "MI_WU_Church_SpireWarm_TexturedLite": {
        "texture": "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Textures/Stone/Stone_Ground_01/T_Stone_Ground_01_D.T_Stone_Ground_01_D",
        "tint": (1.0, 0.78, 0.34, 1.0),
        "brightness": 0.82,
        "roughness": 0.92,
    },
}

CHURCH_GLASS_SETTINGS = {
    "color": (0.055, 0.052, 0.044, 1.0),
    "roughness": 0.96,
}

CURRENT_TO_CHURCH = {
    "MI_WU_Building_Stone_Wall": "MI_WU_Church_StoneWall_TexturedLite",
    "MI_Stone_Wall_UVless": "MI_WU_Church_StoneWall_TexturedLite",
    "MI_WU_Building_Stone_Brown": "MI_WU_Church_StoneBrown_TexturedLite",
    "MI_Stone_Brown": "MI_WU_Church_StoneBrown_TexturedLite",
    "MI_WU_Building_Roof_Tile": "MI_WU_Church_RoofTile_TexturedLite",
    "MI_RoofTiles": "MI_WU_Church_RoofTile_TexturedLite",
    "MI_WU_Building_Wood_Plain": "MI_WU_Church_WoodPlain_TexturedLite",
    "MI_Wood_LightPlain": "MI_WU_Church_WoodPlain_TexturedLite",
    "MI_WU_Building_Wood_Light": "MI_WU_Church_WoodLight_TexturedLite",
    "MI_Wood_LightBrown": "MI_WU_Church_WoodLight_TexturedLite",
    "MI_WU_Building_Metal_Dull": "MI_WU_Church_MetalDull_TexturedLite",
    "MI_MetalRough": "MI_WU_Church_MetalDull_TexturedLite",
    "MI_WU_Building_Spire_Warm": "MI_WU_Church_SpireWarm_TexturedLite",
    "MI_SpireYellow": "MI_WU_Church_SpireWarm_TexturedLite",
    "MI_WU_Building_Glass_Opaque": CHURCH_GLASS_NAME,
    "MI_WU_Church_Glass_Dark": CHURCH_GLASS_NAME,
    "MI_Glass_Inst": CHURCH_GLASS_NAME,
}


def log(message):
    unreal.log("[WU Church Textured Lite] {}".format(message))


def asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return str(obj)


def class_name(obj):
    if not obj:
        return None
    try:
        return obj.get_class().get_name()
    except Exception:
        return type(obj).__name__


def get_asset_base_name(path):
    if not path:
        return None
    package = str(path).rsplit("/", 1)[-1]
    return package.split(".", 1)[0]


def set_property_if_present(obj, prop_name, value):
    try:
        obj.set_editor_property(prop_name, value)
        return True
    except Exception:
        return False


def create_expression(material, expression_class, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(
        material,
        expression_class,
        node_pos_x=x,
        node_pos_y=y,
    )


def connect_expression(source, output_name, target, input_name):
    if not unreal.MaterialEditingLibrary.connect_material_expressions(
        source,
        output_name,
        target,
        input_name,
    ):
        raise RuntimeError(
            "Failed to connect {}.{} to {}.{}".format(
                source.get_name(),
                output_name,
                target.get_name(),
                input_name,
            )
        )


def connect_property(source, output_name, material_property):
    if not unreal.MaterialEditingLibrary.connect_material_property(source, output_name, material_property):
        raise RuntimeError("Failed to connect {} to {}".format(source.get_name(), material_property))


def create_or_clear_master(asset_tools):
    material_path = "{}/{}.{}".format(OUT_DIR, MASTER_NAME, MASTER_NAME)
    material = unreal.load_asset(material_path)
    if material:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    else:
        material = asset_tools.create_asset(MASTER_NAME, OUT_DIR, unreal.Material, unreal.MaterialFactoryNew())
        if not material:
            raise RuntimeError("Failed to create {}".format(material_path))

    white_texture = unreal.load_asset(WHITE_TEXTURE)
    if not white_texture:
        raise RuntimeError("Failed to load default texture {}".format(WHITE_TEXTURE))

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("two_sided", False)

    texture = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, -920, -220)
    texture.set_editor_property("parameter_name", "BaseColorTexture")
    texture.set_editor_property("texture", white_texture)
    set_property_if_present(texture, "sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_COLOR)

    tint = create_expression(material, unreal.MaterialExpressionVectorParameter, -920, -20)
    tint.set_editor_property("parameter_name", "Tint")
    tint.set_editor_property("default_value", unreal.LinearColor(1.0, 1.0, 1.0, 1.0))

    tint_multiply = create_expression(material, unreal.MaterialExpressionMultiply, -560, -150)
    connect_expression(texture, "RGB", tint_multiply, "A")
    connect_expression(tint, "", tint_multiply, "B")

    brightness = create_expression(material, unreal.MaterialExpressionScalarParameter, -920, 170)
    brightness.set_editor_property("parameter_name", "Brightness")
    brightness.set_editor_property("default_value", 1.0)
    set_property_if_present(brightness, "slider_min", 0.0)
    set_property_if_present(brightness, "slider_max", 2.0)

    brightness_multiply = create_expression(material, unreal.MaterialExpressionMultiply, -260, -120)
    connect_expression(tint_multiply, "", brightness_multiply, "A")
    connect_expression(brightness, "", brightness_multiply, "B")

    roughness = create_expression(material, unreal.MaterialExpressionScalarParameter, -520, 120)
    roughness.set_editor_property("parameter_name", "Roughness")
    roughness.set_editor_property("default_value", 0.94)
    set_property_if_present(roughness, "slider_min", 0.0)
    set_property_if_present(roughness, "slider_max", 1.0)

    metallic = create_expression(material, unreal.MaterialExpressionConstant, -520, 280)
    metallic.set_editor_property("r", 0.0)

    specular = create_expression(material, unreal.MaterialExpressionConstant, -520, 430)
    specular.set_editor_property("r", 0.12)

    connect_property(brightness_multiply, "", unreal.MaterialProperty.MP_BASE_COLOR)
    connect_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    connect_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)
    connect_property(specular, "", unreal.MaterialProperty.MP_SPECULAR)

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def create_or_load_instance(asset_tools, instance_name, parent):
    instance_path = "{}/{}.{}".format(OUT_DIR, instance_name, instance_name)
    instance = unreal.load_asset(instance_path)
    if not instance:
        instance = asset_tools.create_asset(
            instance_name,
            OUT_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        if not instance:
            raise RuntimeError("Failed to create {}".format(instance_path))

    settings = CHURCH_MATERIALS[instance_name]
    texture = unreal.load_asset(settings["texture"])
    if not texture:
        raise RuntimeError("Failed to load {}".format(settings["texture"]))

    instance.set_editor_property("parent", parent)
    unreal.MaterialEditingLibrary.set_material_instance_texture_parameter_value(
        instance,
        "BaseColorTexture",
        texture,
    )
    tint = settings["tint"]
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        instance,
        "Tint",
        unreal.LinearColor(tint[0], tint[1], tint[2], tint[3]),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        instance,
        "Brightness",
        settings["brightness"],
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        instance,
        "Roughness",
        settings["roughness"],
    )
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def create_or_load_glass_instance(asset_tools):
    parent = unreal.load_asset(GLASS_MASTER_PATH)
    if not parent:
        raise RuntimeError("Failed to load glass master {}".format(GLASS_MASTER_PATH))

    instance_path = "{}/{}.{}".format(OUT_DIR, CHURCH_GLASS_NAME, CHURCH_GLASS_NAME)
    instance = unreal.load_asset(instance_path)
    if not instance:
        instance = asset_tools.create_asset(
            CHURCH_GLASS_NAME,
            OUT_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        if not instance:
            raise RuntimeError("Failed to create {}".format(instance_path))

    instance.set_editor_property("parent", parent)
    color = CHURCH_GLASS_SETTINGS["color"]
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        instance,
        "BaseColor",
        unreal.LinearColor(color[0], color[1], color[2], color[3]),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        instance,
        "Roughness",
        CHURCH_GLASS_SETTINGS["roughness"],
    )
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def get_external_actor_assets():
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        asset_registry.search_all_assets(True)
        asset_registry.wait_for_completion()
    except Exception:
        pass
    return list(asset_registry.get_assets_by_path(EXTERNAL_ACTOR_PATH, True))


def get_static_mesh_components(actor):
    try:
        return list(actor.get_components_by_class(unreal.StaticMeshComponent))
    except Exception:
        return []


def save_actor(actor, package_name):
    try:
        actor.mark_package_dirty()
    except Exception:
        pass
    for save_call in (
        lambda: unreal.EditorAssetLibrary.save_loaded_asset(actor, False),
        lambda: unreal.EditorAssetLibrary.save_asset(package_name, False),
    ):
        try:
            if save_call():
                return True
        except Exception:
            pass
    return False


def replacement_for_material(material, replacements):
    current_path = asset_path(material)
    current_name = get_asset_base_name(current_path)
    replacement_name = CURRENT_TO_CHURCH.get(current_name)
    if not replacement_name:
        return None, current_path, current_name
    return replacements[replacement_name], current_path, current_name


def apply_church_overrides(replacements):
    scanned_actor_count = 0
    changed_actor_count = 0
    changed_slots = 0
    unchanged_slots = 0
    unmapped_slots = Counter()
    replacement_counts = Counter()
    changed_actors = []
    failed_saves = []

    for asset_data in get_external_actor_assets():
        try:
            actor = asset_data.get_asset()
        except Exception:
            actor = None
        if not actor or class_name(actor) != CHURCH_CLASS_NAME:
            continue

        scanned_actor_count += 1
        package_name = str(asset_data.package_name)
        label = actor.get_actor_label() if hasattr(actor, "get_actor_label") else actor.get_name()
        actor_changed_slots = 0

        for component in get_static_mesh_components(actor):
            try:
                material_count = component.get_num_materials()
            except Exception:
                material_count = 0
            if material_count <= 0:
                continue

            try:
                component.modify()
            except Exception:
                pass

            for slot_index in range(material_count):
                current_material = component.get_material(slot_index)
                replacement, current_path, current_name = replacement_for_material(current_material, replacements)
                if not replacement:
                    unmapped_slots[current_path or "<none>"] += 1
                    continue

                replacement_path = asset_path(replacement)
                if current_path == replacement_path:
                    unchanged_slots += 1
                    replacement_counts[replacement_path] += 1
                    continue

                component.set_material(slot_index, replacement)
                changed_slots += 1
                actor_changed_slots += 1
                replacement_counts[replacement_path] += 1

        if actor_changed_slots:
            changed_actor_count += 1
            changed_actors.append(
                {
                    "label": label,
                    "class": class_name(actor),
                    "package": package_name,
                    "changed_slots": actor_changed_slots,
                }
            )
            if not save_actor(actor, package_name):
                failed_saves.append(package_name)

    try:
        unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    except Exception as exc:
        log("save_dirty_packages failed: {}".format(exc))

    return {
        "scanned_actor_count": scanned_actor_count,
        "changed_actor_count": changed_actor_count,
        "changed_slots": changed_slots,
        "unchanged_slots": unchanged_slots,
        "unmapped_slots": unmapped_slots.most_common(),
        "replacement_counts": replacement_counts.most_common(),
        "changed_actors": changed_actors,
        "failed_saves": failed_saves,
    }


def write_report(result):
    report_dir = os.path.join(unreal.Paths.project_saved_dir(), "Reports")
    os.makedirs(report_dir, exist_ok=True)
    json_path = os.path.join(report_dir, "church_textured_lite_report.json")
    txt_path = os.path.join(report_dir, "church_textured_lite_report.txt")

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2, sort_keys=True)

    lines = [
        "WU Church Textured Lite Report",
        "Church actors scanned: {}".format(result["scanned_actor_count"]),
        "Church actors changed: {}".format(result["changed_actor_count"]),
        "Material slots changed: {}".format(result["changed_slots"]),
        "Material slots already textured-lite: {}".format(result["unchanged_slots"]),
        "",
        "Replacement material slot counts:",
    ]
    for path, count in result["replacement_counts"]:
        lines.append("  {} | {}".format(count, path))

    if result["unmapped_slots"]:
        lines.append("")
        lines.append("Unmapped slots:")
        for path, count in result["unmapped_slots"]:
            lines.append("  {} | {}".format(count, path))

    if result["failed_saves"]:
        lines.append("")
        lines.append("Failed saves:")
        for package_name in result["failed_saves"]:
            lines.append("  {}".format(package_name))

    lines.append("")
    lines.append("Changed actors:")
    for actor in result["changed_actors"]:
        lines.append("  {} | {} slots | {}".format(actor["label"], actor["changed_slots"], actor["package"]))

    with open(txt_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    log("Wrote {}".format(json_path))
    log("Wrote {}".format(txt_path))


def main():
    unreal.EditorAssetLibrary.make_directory(OUT_DIR)
    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    master = create_or_clear_master(asset_tools)
    replacements = {
        name: create_or_load_instance(asset_tools, name, master)
        for name in sorted(CHURCH_MATERIALS.keys())
    }
    replacements[CHURCH_GLASS_NAME] = create_or_load_glass_instance(asset_tools)
    result = apply_church_overrides(replacements)
    write_report(result)
    log(
        "Changed {} slots across {} church actors.".format(
            result["changed_slots"],
            result["changed_actor_count"],
        )
    )


if __name__ == "__main__":
    main()
