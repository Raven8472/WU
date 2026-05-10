import json
import os
from collections import Counter

import unreal


EXTERNAL_ACTOR_PATH = "/Game/__ExternalActors__/ThirdPerson/Lvl_MagicalBritain_Persistent"
BUILDING_HINTS = (
    "building",
    "architecture",
    "architectural",
    "castle",
    "house",
    "wall",
    "floor",
    "roof",
    "stairs",
    "stair",
    "pillar",
    "railing",
    "window",
    "door",
    "medieval",
    "school_of_magic",
)


def log(message):
    unreal.log("[WU External Actor Audit] {}".format(message))


def as_string(value):
    try:
        return str(value)
    except Exception:
        return None


def asset_path(obj):
    if not obj:
        return None
    try:
        return obj.get_path_name()
    except Exception:
        return as_string(obj)


def class_name(obj):
    if not obj:
        return None
    try:
        return obj.get_class().get_name()
    except Exception:
        return type(obj).__name__


def get_editor_prop(obj, prop_name, default=None):
    try:
        return obj.get_editor_property(prop_name)
    except Exception:
        return default


def get_nanite_enabled(static_mesh):
    if not static_mesh:
        return None
    settings = get_editor_prop(static_mesh, "nanite_settings")
    if not settings:
        return None
    for prop in ("enabled", "b_enabled"):
        try:
            return bool(settings.get_editor_property(prop))
        except Exception:
            pass
    for prop in ("enabled", "b_enabled"):
        if hasattr(settings, prop):
            return bool(getattr(settings, prop))
    return None


def get_material_parent(material):
    if not material:
        return None
    try:
        parent = material.get_editor_property("parent")
        return asset_path(parent)
    except Exception:
        return None


def asset_data_object_path(asset_data):
    for attr_name in ("object_path", "object_path_string"):
        value = getattr(asset_data, attr_name, None)
        if value:
            return as_string(value)
    for method_name in ("get_soft_object_path", "get_export_text_name"):
        method = getattr(asset_data, method_name, None)
        if method:
            try:
                return as_string(method())
            except Exception:
                pass
    package_name = as_string(getattr(asset_data, "package_name", ""))
    asset_name = as_string(getattr(asset_data, "asset_name", ""))
    if package_name and asset_name:
        return "{}.{}".format(package_name, asset_name)
    return package_name or asset_name


def tag_map(asset_data):
    tags = {}
    raw = getattr(asset_data, "tags_and_values", None)
    if not raw:
        return tags
    for method_name in ("to_map", "to_dict"):
        method = getattr(raw, method_name, None)
        if method:
            try:
                return {as_string(k): as_string(v) for k, v in method().items()}
            except Exception:
                pass
    try:
        return {as_string(k): as_string(v) for k, v in dict(raw).items()}
    except Exception:
        return tags


def get_dependencies(asset_registry, package_name):
    dependency_paths = []
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
    for dep in deps:
        dependency_paths.append(as_string(dep))
    return sorted(set([path for path in dependency_paths if path]))


def looks_like_building(tokens):
    haystack = " ".join([token for token in tokens if token]).lower()
    return any(hint in haystack for hint in BUILDING_HINTS)


def inspect_loaded_actor(obj):
    result = {
        "loaded_class": class_name(obj),
        "loaded_path": asset_path(obj),
        "components": [],
    }
    try:
        if hasattr(obj, "get_actor_label"):
            result["actor_label"] = obj.get_actor_label()
    except Exception:
        pass

    try:
        components = list(obj.get_components_by_class(unreal.StaticMeshComponent))
    except Exception:
        components = []

    for comp in components:
        comp_entry = {
            "name": comp.get_name(),
            "class": class_name(comp),
            "mesh": None,
            "materials": [],
        }
        try:
            mesh = comp.get_editor_property("static_mesh")
            comp_entry["mesh"] = asset_path(mesh)
            comp_entry["nanite_enabled"] = get_nanite_enabled(mesh)
        except Exception:
            pass
        try:
            material_count = comp.get_num_materials()
        except Exception:
            material_count = 0
        for index in range(material_count):
            try:
                material = comp.get_material(index)
            except Exception:
                material = None
            comp_entry["materials"].append(
                {
                    "slot": index,
                    "path": asset_path(material),
                    "class": class_name(material),
                    "parent": get_material_parent(material),
                }
            )
        result["components"].append(comp_entry)
    return result


def main():
    report_dir = os.path.join(unreal.Paths.project_saved_dir(), "Reports")
    os.makedirs(report_dir, exist_ok=True)

    asset_registry = unreal.AssetRegistryHelpers.get_asset_registry()
    try:
        asset_registry.search_all_assets(True)
    except Exception:
        pass
    try:
        asset_registry.wait_for_completion()
    except Exception:
        pass

    assets = list(asset_registry.get_assets_by_path(EXTERNAL_ACTOR_PATH, True))
    log("External actor assets: {}".format(len(assets)))

    class_counts = Counter()
    dep_counts = Counter()
    candidate_packages = []
    all_packages = []

    for asset_data in assets:
        package_name = as_string(asset_data.package_name)
        object_path = asset_data_object_path(asset_data)
        asset_name = as_string(asset_data.asset_name)
        asset_class = as_string(getattr(asset_data, "asset_class_path", "")) or as_string(
            getattr(asset_data, "asset_class", "")
        )
        tags = tag_map(asset_data)
        deps = get_dependencies(asset_registry, asset_data.package_name)
        for dep in deps:
            dep_counts[dep] += 1

        loaded = None
        try:
            loaded = asset_data.get_asset()
        except Exception:
            loaded = None

        loaded_info = inspect_loaded_actor(loaded) if loaded else None
        loaded_class = loaded_info.get("loaded_class") if loaded_info else None
        class_counts[loaded_class or asset_class or "<unknown>"] += 1

        tokens = [
            package_name,
            object_path,
            asset_name,
            asset_class,
            loaded_class,
            json.dumps(tags, sort_keys=True),
        ]
        tokens.extend(deps)
        if loaded_info:
            tokens.append(json.dumps(loaded_info, sort_keys=True))

        entry = {
            "package_name": package_name,
            "object_path": object_path,
            "asset_name": asset_name,
            "asset_class": asset_class,
            "tags": tags,
            "dependencies": deps,
            "loaded_info": loaded_info,
            "looks_like_building": looks_like_building(tokens),
        }
        all_packages.append(entry)
        if entry["looks_like_building"]:
            candidate_packages.append(entry)

    report = {
        "external_actor_path": EXTERNAL_ACTOR_PATH,
        "external_actor_count": len(assets),
        "candidate_building_actor_count": len(candidate_packages),
        "class_counts": class_counts.most_common(),
        "top_dependencies": dep_counts.most_common(100),
        "candidate_packages": candidate_packages,
        "all_packages": all_packages,
    }

    json_path = os.path.join(report_dir, "external_building_actor_audit.json")
    txt_path = os.path.join(report_dir, "external_building_actor_audit.txt")

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2, sort_keys=True)

    lines = []
    lines.append("WU External Building Actor Audit")
    lines.append("Path: {}".format(EXTERNAL_ACTOR_PATH))
    lines.append("External actors: {}".format(len(assets)))
    lines.append("Building-like candidates: {}".format(len(candidate_packages)))
    lines.append("")
    lines.append("Classes:")
    for name, count in class_counts.most_common(30):
        lines.append("  {} | {}".format(count, name))
    lines.append("")
    lines.append("Top dependencies:")
    for dep, count in dep_counts.most_common(40):
        lines.append("  {} | {}".format(count, dep))
    lines.append("")
    lines.append("Candidate actor packages:")
    for entry in candidate_packages[:80]:
        lines.append("  {}".format(entry["package_name"]))
        loaded_info = entry.get("loaded_info") or {}
        label = loaded_info.get("actor_label")
        loaded_class = loaded_info.get("loaded_class")
        if label or loaded_class:
            lines.append("    label/class: {} / {}".format(label, loaded_class))
        deps = [
            dep
            for dep in entry["dependencies"]
            if looks_like_building([dep])
        ][:8]
        for dep in deps:
            lines.append("    dep: {}".format(dep))

    with open(txt_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    log("Wrote {}".format(json_path))
    log("Wrote {}".format(txt_path))


if __name__ == "__main__":
    main()
