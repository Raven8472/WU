import json
import os

import unreal


MATERIAL_PATHS = [
    "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Geometry/Churches/Materials/MI_Stone_Wall_UVless.MI_Stone_Wall_UVless",
    "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Geometry/Churches/Materials/MI_Stone_Brown.MI_Stone_Brown",
    "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Geometry/Churches/Materials/MI_RoofTiles.MI_RoofTiles",
    "/Game/AssetPacks/Brushify/DesignPacks/Medieval/Buildings/Geometry/Churches/Materials/MI_SpireYellow.MI_SpireYellow",
    "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Presets/Wood/MI_Wood_LightPlain.MI_Wood_LightPlain",
    "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Presets/Wood/MI_Wood_LightBrown.MI_Wood_LightBrown",
    "/Game/AssetPacks/Brushify/DesignPacks/MasterMaterial/Presets/Metal/MI_MetalRough.MI_MetalRough",
]


def asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return str(obj)


def try_call(method, *args):
    try:
        return method(*args)
    except Exception as exc:
        return "__ERROR__ {}".format(exc)


def normalize_name(name):
    try:
        return str(name)
    except Exception:
        return repr(name)


def get_texture_params(material):
    results = {}
    names = None
    for fn_name in (
        "get_texture_parameter_names",
        "get_material_instance_texture_parameter_names",
    ):
        fn = getattr(unreal.MaterialEditingLibrary, fn_name, None)
        if fn:
            names = try_call(fn, material)
            if not isinstance(names, str):
                break
    if isinstance(names, str):
        results["names_error"] = names
        names = []
    results["names"] = [normalize_name(name) for name in (names or [])]
    values = {}
    for name in names or []:
        for fn_name in (
            "get_material_instance_texture_parameter_value",
            "get_texture_parameter_value",
        ):
            fn = getattr(unreal.MaterialEditingLibrary, fn_name, None)
            if not fn:
                continue
            value = try_call(fn, material, name)
            if isinstance(value, str) and value.startswith("__ERROR__"):
                values.setdefault(normalize_name(name), []).append(value)
                continue
            values[normalize_name(name)] = asset_path(value)
            break
    results["values"] = values
    return results


def get_vector_params(material):
    results = {}
    names = None
    for fn_name in (
        "get_vector_parameter_names",
        "get_material_instance_vector_parameter_names",
    ):
        fn = getattr(unreal.MaterialEditingLibrary, fn_name, None)
        if fn:
            names = try_call(fn, material)
            if not isinstance(names, str):
                break
    if isinstance(names, str):
        results["names_error"] = names
        names = []
    results["names"] = [normalize_name(name) for name in (names or [])]
    values = {}
    for name in names or []:
        fn = getattr(unreal.MaterialEditingLibrary, "get_material_instance_vector_parameter_value", None)
        if not fn:
            continue
        value = try_call(fn, material, name)
        values[normalize_name(name)] = str(value)
    results["values"] = values
    return results


def get_dependencies(asset_registry, package_name):
    options = unreal.AssetRegistryDependencyOptions(
        include_soft_package_references=True,
        include_hard_package_references=True,
        include_searchable_names=True,
        include_soft_management_references=True,
        include_hard_management_references=True,
    )
    try:
        deps = asset_registry.get_dependencies(package_name, options)
    except Exception:
        deps = asset_registry.get_dependencies(package_name)
    return sorted(set([str(dep) for dep in deps]))


def main():
    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        asset_registry.search_all_assets(True)
        asset_registry.wait_for_completion()
    except Exception:
        pass

    report = {}
    for path in MATERIAL_PATHS:
        material = unreal.load_asset(path)
        if not material:
            report[path] = {"error": "failed to load"}
            continue
        package_name = path.split(".", 1)[0]
        parent = None
        try:
            parent = asset_path(material.get_editor_property("parent"))
        except Exception:
            pass
        report[path] = {
            "parent": parent,
            "texture_params": get_texture_params(material),
            "vector_params": get_vector_params(material),
            "dependencies": get_dependencies(asset_registry, package_name),
        }

    report_dir = os.path.join(unreal.Paths.project_saved_dir(), "Reports")
    os.makedirs(report_dir, exist_ok=True)
    out_path = os.path.join(report_dir, "church_material_source_params.json")
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2, sort_keys=True)
    unreal.log("[WU Church Material Inspect] Wrote {}".format(out_path))


if __name__ == "__main__":
    main()
