import json
import os
from collections import Counter

import unreal


EXTERNAL_ACTOR_PATH = "/Game/__ExternalActors__/ThirdPerson/Lvl_MagicalBritain_Persistent"
MATERIAL_CANDIDATES = [
    "/Game/World/Landscape/MI_WU_Landscape_Starter.MI_WU_Landscape_Starter",
    "/Game/World/Landscape/MI_WU_Landscape_Hole.MI_WU_Landscape_Hole",
    "/Game/World/Landscape/M_WU_Landscape_Master.M_WU_Landscape_Master",
    "/Game/World/Landscape/M_WU_Landscape_Hole.M_WU_Landscape_Hole",
    "/Game/AssetPacks/Brushify/Materials/Landscape/MI_Landscape.MI_Landscape",
    "/Game/AssetPacks/Brushify/Materials/Landscape/M_Landscape.M_Landscape",
    "/Game/AssetPacks/Brushify/Maps/Medieval/MaterialOverrides/MI_Landscape.MI_Landscape",
    "/Game/AssetPacks/School_Of_Magic/Materials/Landscape/MI_Landscape.MI_Landscape",
    "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape/MI_landscapeGround_ajustabel_inst.MI_landscapeGround_ajustabel_inst",
    "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape/v2/MI_landscape_pro_v2_inst.MI_landscape_pro_v2_inst",
    "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape/v2/MI_landscape_pro_v2_no_tesselation_Inst.MI_landscape_pro_v2_no_tesselation_Inst",
    "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape/v2/MI_landscape_pro_v2_no_tesselation_default_triplanar_mapping_Inst.MI_landscape_pro_v2_no_tesselation_default_triplanar_mapping_Inst",
    "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape/v2/WU_Landscape.WU_Landscape",
]


def log(message):
    unreal.log("[WU Landscape Audit] {}".format(message))


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


def get_prop(obj, prop_name):
    try:
        return obj.get_editor_property(prop_name)
    except Exception:
        return None


def try_call(fn, *args):
    try:
        return fn(*args)
    except Exception as exc:
        return "__ERROR__ {}".format(exc)


def normalize_name(name):
    try:
        return str(name)
    except Exception:
        return repr(name)


def get_material_parent(material):
    if not material:
        return None
    return asset_path(get_prop(material, "parent"))


def get_scalar_params(material):
    results = {
        "names": [],
        "values": {},
        "overrides": {},
    }
    fn = getattr(unreal.MaterialEditingLibrary, "get_scalar_parameter_names", None)
    if fn:
        names = try_call(fn, material)
        if isinstance(names, str):
            results["names_error"] = names
            names = []
        results["names"] = [normalize_name(name) for name in names or []]
        get_value = getattr(unreal.MaterialEditingLibrary, "get_material_instance_scalar_parameter_value", None)
        if get_value:
            for name in names or []:
                value = try_call(get_value, material, name)
                results["values"][normalize_name(name)] = value

    for prop_name in ("scalar_parameter_values", "static_parameters"):
        raw = get_prop(material, prop_name)
        if raw:
            results["overrides"][prop_name] = str(raw)
    return results


def get_vector_params(material):
    results = {
        "names": [],
        "values": {},
    }
    fn = getattr(unreal.MaterialEditingLibrary, "get_vector_parameter_names", None)
    if not fn:
        return results
    names = try_call(fn, material)
    if isinstance(names, str):
        results["names_error"] = names
        return results
    results["names"] = [normalize_name(name) for name in names or []]
    get_value = getattr(unreal.MaterialEditingLibrary, "get_material_instance_vector_parameter_value", None)
    if get_value:
        for name in names or []:
            results["values"][normalize_name(name)] = str(try_call(get_value, material, name))
    return results


def get_texture_params(material):
    results = {
        "names": [],
        "values": {},
    }
    fn = getattr(unreal.MaterialEditingLibrary, "get_texture_parameter_names", None)
    if not fn:
        return results
    names = try_call(fn, material)
    if isinstance(names, str):
        results["names_error"] = names
        return results
    results["names"] = [normalize_name(name) for name in names or []]
    get_value = getattr(unreal.MaterialEditingLibrary, "get_material_instance_texture_parameter_value", None)
    if get_value:
        for name in names or []:
            results["values"][normalize_name(name)] = asset_path(try_call(get_value, material, name))
    return results


def get_material_expressions(material):
    expressions = get_prop(material, "expressions")
    if expressions:
        try:
            return list(expressions)
        except Exception:
            return expressions

    editor_data = get_prop(material, "editor_only_data")
    expression_collection = get_prop(editor_data, "expression_collection") if editor_data else None
    expressions = get_prop(expression_collection, "expressions") if expression_collection else None
    if expressions:
        try:
            return list(expressions)
        except Exception:
            return expressions

    return []


def get_landscape_coord_nodes(material):
    nodes = []
    expressions = get_material_expressions(material)
    for expression in expressions:
        if class_name(expression) != "MaterialExpressionLandscapeLayerCoords":
            continue
        entry = {
            "desc": get_prop(expression, "desc"),
            "mapping_scale": None,
            "mapping_rotation": get_prop(expression, "mapping_rotation"),
            "mapping_pan_u": get_prop(expression, "mapping_pan_u"),
            "mapping_pan_v": get_prop(expression, "mapping_pan_v"),
        }
        for prop_name in ("mapping_scale", "MappingScale"):
            value = get_prop(expression, prop_name)
            if value is not None:
                entry["mapping_scale"] = value
                break
        nodes.append(entry)
    return nodes


def inspect_material(path):
    material = unreal.load_asset(path)
    if not material:
        return {"path": path, "error": "failed to load"}
    return {
        "path": asset_path(material),
        "class": class_name(material),
        "parent": get_material_parent(material),
        "scalars": get_scalar_params(material),
        "vectors": get_vector_params(material),
        "textures": get_texture_params(material),
        "landscape_coord_nodes": get_landscape_coord_nodes(material),
    }


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
        try:
            deps = asset_registry.get_dependencies(package_name)
        except Exception:
            deps = []
    return sorted(set(str(dep) for dep in deps if dep))


def inspect_actor(actor, package_name, dependencies):
    actor_class = class_name(actor)
    entry = {
        "package_name": str(package_name),
        "class": actor_class,
        "label": None,
        "path": asset_path(actor),
        "dependencies": dependencies,
        "landscape_material": None,
        "landscape_hole_material": None,
        "override_materials": [],
    }
    try:
        entry["label"] = actor.get_actor_label()
    except Exception:
        pass

    for prop_name in ("landscape_material", "landscape_hole_material"):
        entry[prop_name] = asset_path(get_prop(actor, prop_name))

    for prop_name in (
        "override_materials",
        "material_instances",
        "material_instance_dynamic",
        "material_instance_constants",
    ):
        raw = get_prop(actor, prop_name)
        if raw:
            try:
                entry["override_materials"].extend([asset_path(item) for item in list(raw)])
            except Exception:
                entry["override_materials"].append(str(raw))
    return entry


def main():
    report_dir = os.path.join(unreal.Paths.project_saved_dir(), "Reports")
    os.makedirs(report_dir, exist_ok=True)

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        asset_registry.search_all_assets(True)
        asset_registry.wait_for_completion()
    except Exception:
        pass

    actors = []
    dep_counts = Counter()
    for asset_data in list(asset_registry.get_assets_by_path(EXTERNAL_ACTOR_PATH, True)):
        package_name = asset_data.package_name
        dependencies = get_dependencies(asset_registry, package_name)
        for dep in dependencies:
            dep_counts[dep] += 1
        haystack = " ".join([str(package_name)] + dependencies).lower()
        if "landscape" not in haystack:
            continue
        try:
            actor = asset_data.get_asset()
        except Exception:
            actor = None
        if actor:
            actors.append(inspect_actor(actor, package_name, dependencies))

    material_paths = set(MATERIAL_CANDIDATES)
    for actor in actors:
        for key in ("landscape_material", "landscape_hole_material"):
            if actor.get(key):
                material_paths.add(actor[key])
        for material_path in actor.get("override_materials") or []:
            if material_path:
                material_paths.add(material_path)

    report = {
        "landscape_like_actor_count": len(actors),
        "landscape_like_actors": actors,
        "landscape_dependency_counts": [
            [path, count]
            for path, count in dep_counts.most_common(100)
            if "landscape" in path.lower()
        ],
        "materials": [inspect_material(path) for path in sorted(material_paths)],
    }

    json_path = os.path.join(report_dir, "landscape_material_audit.json")
    txt_path = os.path.join(report_dir, "landscape_material_audit.txt")
    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2, sort_keys=True)

    lines = [
        "WU Landscape Material Audit",
        "Landscape-like actors: {}".format(len(actors)),
        "",
        "Actors:",
    ]
    for actor in actors:
        lines.append("  {} | {} | {}".format(actor["class"], actor["label"], actor["package_name"]))
        lines.append("    material: {}".format(actor["landscape_material"]))
        lines.append("    hole: {}".format(actor["landscape_hole_material"]))
    lines.append("")
    lines.append("Materials and scalar values:")
    for mat in report["materials"]:
        lines.append("  {} | {}".format(mat.get("class"), mat.get("path")))
        if mat.get("parent"):
            lines.append("    parent: {}".format(mat["parent"]))
        values = mat.get("scalars", {}).get("values", {})
        for name in sorted(values):
            lines.append("    {} = {}".format(name, values[name]))
        coord_nodes = mat.get("landscape_coord_nodes") or []
        for node in sorted(coord_nodes, key=lambda item: str(item.get("desc"))):
            lines.append("    coord {} = {}".format(node.get("desc"), node.get("mapping_scale")))

    with open(txt_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    log("Wrote {}".format(json_path))
    log("Wrote {}".format(txt_path))


if __name__ == "__main__":
    main()
