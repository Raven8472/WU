import json
import os
from collections import Counter

import unreal


EXTERNAL_ACTOR_PATH = "/Game/__ExternalActors__/ThirdPerson/Lvl_MagicalBritain_Persistent"
OUT_DIR = "/Game/World/Buildings/Materials"
MASTER_NAME = "M_WU_Building_StylizedLite"
GLASS_MASTER_NAME = "M_WU_Building_Glass_TwoSidedLite"

ORIGINAL_TO_REPLACEMENT = {
    "MI_Wood_DarkBrown": "MI_WU_Building_Wood_Dark",
    "MI_Wood_LightBrown": "MI_WU_Building_Wood_Light",
    "MI_Wood_LightPlain": "MI_WU_Building_Wood_Plain",
    "MI_MetalRough": "MI_WU_Building_Metal_Dull",
    "MI_Plaster_Cream": "MI_WU_Building_Plaster_Cream",
    "MI_Plaster_White": "MI_WU_Building_Plaster_White",
    "MI_RoofTiles_House": "MI_WU_Building_Roof_Tile",
    "MI_RoofTiles": "MI_WU_Building_Roof_Tile",
    "MI_RoofThatched": "MI_WU_Building_Roof_Thatch",
    "MI_Stone_02": "MI_WU_Building_Stone",
    "MI_Stone_Brown": "MI_WU_Building_Stone_Brown",
    "MI_Stone_Wall_2xTiling": "MI_WU_Building_Stone_Wall",
    "MI_Stone_Wall_UVless": "MI_WU_Building_Stone_Wall",
    "MI_Stone_Ground": "MI_WU_Building_Stone_Ground",
    "MI_SpireYellow": "MI_WU_Building_Spire_Warm",
    "MI_Glass_Inst": "MI_WU_Building_Glass_Opaque",
}

REPLACEMENT_SETTINGS = {
    "MI_WU_Building_Wood_Dark": {
        "color": (0.24, 0.13, 0.075, 1.0),
        "roughness": 0.82,
    },
    "MI_WU_Building_Wood_Light": {
        "color": (0.56, 0.36, 0.19, 1.0),
        "roughness": 0.86,
    },
    "MI_WU_Building_Wood_Plain": {
        "color": (0.66, 0.48, 0.28, 1.0),
        "roughness": 0.88,
    },
    "MI_WU_Building_Metal_Dull": {
        "color": (0.18, 0.19, 0.19, 1.0),
        "roughness": 0.91,
    },
    "MI_WU_Building_Plaster_Cream": {
        "color": (0.78, 0.70, 0.55, 1.0),
        "roughness": 0.94,
    },
    "MI_WU_Building_Plaster_White": {
        "color": (0.84, 0.80, 0.70, 1.0),
        "roughness": 0.94,
    },
    "MI_WU_Building_Roof_Tile": {
        "color": (0.34, 0.13, 0.095, 1.0),
        "roughness": 0.93,
    },
    "MI_WU_Building_Roof_Thatch": {
        "color": (0.47, 0.37, 0.20, 1.0),
        "roughness": 0.96,
    },
    "MI_WU_Building_Stone": {
        "color": (0.38, 0.38, 0.34, 1.0),
        "roughness": 0.95,
    },
    "MI_WU_Building_Stone_Brown": {
        "color": (0.35, 0.28, 0.21, 1.0),
        "roughness": 0.96,
    },
    "MI_WU_Building_Stone_Wall": {
        "color": (0.32, 0.31, 0.28, 1.0),
        "roughness": 0.96,
    },
    "MI_WU_Building_Stone_Ground": {
        "color": (0.28, 0.27, 0.24, 1.0),
        "roughness": 0.97,
    },
    "MI_WU_Building_Glass_Opaque": {
        "color": (0.24, 0.36, 0.42, 1.0),
        "roughness": 0.72,
    },
    "MI_WU_Building_Spire_Warm": {
        "color": (0.72, 0.58, 0.26, 1.0),
        "roughness": 0.9,
    },
}

GLASS_INSTANCE_NAMES = {
    "MI_WU_Building_Glass_Opaque",
}


def log(message):
    unreal.log("[WU Building Stylizer] {}".format(message))


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


def connect_property(source, output_name, material_property):
    if not unreal.MaterialEditingLibrary.connect_material_property(source, output_name, material_property):
        raise RuntimeError("Failed to connect {} to {}".format(source.get_name(), material_property))


def create_or_clear_master(asset_tools, material_name=MASTER_NAME, two_sided=False, specular_value=0.12):
    material_path = "{}/{}.{}".format(OUT_DIR, material_name, material_name)
    material = unreal.load_asset(material_path)
    if material:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
    else:
        material = asset_tools.create_asset(material_name, OUT_DIR, unreal.Material, unreal.MaterialFactoryNew())
        if not material:
            raise RuntimeError("Failed to create {}".format(material_path))

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("two_sided", two_sided)

    color = create_expression(material, unreal.MaterialExpressionVectorParameter, -520, -160)
    color.set_editor_property("parameter_name", "BaseColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.5, 0.5, 0.5, 1.0))

    roughness = create_expression(material, unreal.MaterialExpressionScalarParameter, -520, 20)
    roughness.set_editor_property("parameter_name", "Roughness")
    roughness.set_editor_property("default_value", 0.9)
    set_property_if_present(roughness, "slider_min", 0.0)
    set_property_if_present(roughness, "slider_max", 1.0)

    metallic = create_expression(material, unreal.MaterialExpressionConstant, -520, 180)
    metallic.set_editor_property("r", 0.0)

    specular = create_expression(material, unreal.MaterialExpressionConstant, -520, 340)
    specular.set_editor_property("r", specular_value)

    connect_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
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

    instance.set_editor_property("parent", parent)
    settings = REPLACEMENT_SETTINGS[instance_name]
    color = settings["color"]
    unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
        instance,
        "BaseColor",
        unreal.LinearColor(color[0], color[1], color[2], color[3]),
    )
    unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
        instance,
        "Roughness",
        settings["roughness"],
    )
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def get_building_actor_assets():
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


def is_building_actor(actor):
    actor_class_name = class_name(actor) or ""
    return (
        actor_class_name.startswith("BP_House")
        or actor_class_name.startswith("BP_Hovel")
        or actor_class_name.startswith("BP_Church")
    )


def replacement_for_material(material, replacements):
    original_path = asset_path(material)
    original_name = get_asset_base_name(original_path)
    replacement_name = ORIGINAL_TO_REPLACEMENT.get(original_name)
    if not replacement_name:
        return None, original_path, original_name
    return replacements[replacement_name], original_path, original_name


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


def apply_overrides(replacements):
    building_actor_count = 0
    changed_actor_count = 0
    changed_slots = 0
    unchanged_slots = 0
    unmapped_slots = Counter()
    replacement_counts = Counter()
    changed_actors = []
    failed_saves = []

    for asset_data in get_building_actor_assets():
        try:
            actor = asset_data.get_asset()
        except Exception:
            actor = None
        if not actor or not is_building_actor(actor):
            continue

        building_actor_count += 1
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
                replacement, original_path, original_name = replacement_for_material(current_material, replacements)
                if not replacement:
                    unmapped_slots[original_path or "<none>"] += 1
                    continue

                replacement_path = asset_path(replacement)
                if original_path == replacement_path:
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
        "building_actor_count": building_actor_count,
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
    json_path = os.path.join(report_dir, "building_material_stylize_report.json")
    txt_path = os.path.join(report_dir, "building_material_stylize_report.txt")

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(result, f, indent=2, sort_keys=True)

    lines = [
        "WU Building Material Stylize Report",
        "Building actors scanned: {}".format(result["building_actor_count"]),
        "Building actors changed: {}".format(result["changed_actor_count"]),
        "Material slots changed: {}".format(result["changed_slots"]),
        "Material slots already stylized: {}".format(result["unchanged_slots"]),
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
    glass_master = create_or_clear_master(
        asset_tools,
        material_name=GLASS_MASTER_NAME,
        two_sided=True,
        specular_value=0.18,
    )
    replacements = {
        name: create_or_load_instance(
            asset_tools,
            name,
            glass_master if name in GLASS_INSTANCE_NAMES else master,
        )
        for name in sorted(REPLACEMENT_SETTINGS.keys())
    }
    result = apply_overrides(replacements)
    write_report(result)
    log(
        "Changed {} slots across {} building actors.".format(
            result["changed_slots"],
            result["changed_actor_count"],
        )
    )


if __name__ == "__main__":
    main()
