import json
import os
from collections import Counter, defaultdict

import unreal


MAP_PATH = "/Game/ThirdPerson/Lvl_MagicalBritain_Persistent"
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
    unreal.log("[WU Building Audit] {}".format(message))


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


def get_editor_prop(obj, prop_name, default=None):
    try:
        return obj.get_editor_property(prop_name)
    except Exception:
        return default


def get_material_parent(material):
    if not material:
        return None
    try:
        parent = material.get_editor_property("parent")
        return asset_path(parent)
    except Exception:
        return None


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


def get_material_details(material):
    details = {
        "path": asset_path(material),
        "class": class_name(material),
        "parent": get_material_parent(material),
    }
    base = material
    parent_path = details["parent"]
    if parent_path:
        loaded_parent = unreal.EditorAssetLibrary.load_asset(parent_path.split(".")[0])
        if loaded_parent:
            base = loaded_parent

    for prop in (
        "blend_mode",
        "shading_model",
        "two_sided",
        "dithered_lod_transition",
        "use_material_attributes",
        "enable_tessellation",
    ):
        value = get_editor_prop(base, prop)
        if value is not None:
            details[prop] = str(value)
    return details


def looks_like_building(actor, components):
    label = actor.get_actor_label() if hasattr(actor, "get_actor_label") else actor.get_name()
    tokens = [label, class_name(actor), asset_path(actor)]
    for comp in components:
        mesh = comp.get_editor_property("static_mesh")
        tokens.append(asset_path(mesh))
    haystack = " ".join([t for t in tokens if t]).lower()
    return any(hint in haystack for hint in BUILDING_HINTS)


def load_level():
    log("Loading {}".format(MAP_PATH))
    loaded = False
    try:
        subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        loaded = bool(subsystem.load_level(MAP_PATH))
    except Exception as exc:
        log("LevelEditorSubsystem.load_level failed: {}".format(exc))
    if not loaded:
        try:
            loaded = bool(unreal.EditorLevelLibrary.load_level(MAP_PATH))
        except Exception as exc:
            log("EditorLevelLibrary.load_level failed: {}".format(exc))
    return loaded


def try_load_world_partition_actors():
    method_names = []
    try:
        wp = unreal.get_editor_subsystem(unreal.WorldPartitionEditorSubsystem)
        method_names = [name for name in dir(wp) if "load" in name.lower() or "actor" in name.lower()]
        for name in (
            "load_all_cells",
            "load_all_actors",
            "load_actors",
        ):
            method = getattr(wp, name, None)
            if method:
                try:
                    method()
                    log("Called WorldPartitionEditorSubsystem.{}()".format(name))
                    break
                except Exception as exc:
                    log("World partition {}() failed: {}".format(name, exc))
    except Exception as exc:
        log("World partition subsystem unavailable: {}".format(exc))
    return method_names


def get_all_actors():
    try:
        subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        return list(subsystem.get_all_level_actors())
    except Exception:
        return list(unreal.EditorLevelLibrary.get_all_level_actors())


def main():
    report_dir = os.path.join(unreal.Paths.project_saved_dir(), "Reports")
    os.makedirs(report_dir, exist_ok=True)

    loaded = load_level()
    wp_methods = try_load_world_partition_actors()
    actors = get_all_actors()
    log("Loaded level: {}; actors visible to editor: {}".format(loaded, len(actors)))

    material_counts = Counter()
    mesh_counts = Counter()
    nanite_counts = Counter()
    material_details = {}
    building_actors = []
    all_static_mesh_actor_count = 0

    for actor in actors:
        try:
            components = list(actor.get_components_by_class(unreal.StaticMeshComponent))
        except Exception:
            components = []
        if not components:
            continue

        all_static_mesh_actor_count += 1
        if not looks_like_building(actor, components):
            continue

        actor_entry = {
            "label": actor.get_actor_label() if hasattr(actor, "get_actor_label") else actor.get_name(),
            "name": actor.get_name(),
            "class": class_name(actor),
            "path": asset_path(actor),
            "components": [],
        }

        for comp in components:
            mesh = comp.get_editor_property("static_mesh")
            mesh_path = asset_path(mesh)
            nanite_enabled = get_nanite_enabled(mesh)
            mesh_counts[mesh_path or "<none>"] += 1
            nanite_counts[str(nanite_enabled)] += 1

            comp_entry = {
                "name": comp.get_name(),
                "class": class_name(comp),
                "mesh": mesh_path,
                "nanite_enabled": nanite_enabled,
                "materials": [],
            }

            try:
                material_count = comp.get_num_materials()
            except Exception:
                material_count = 0
            for index in range(material_count):
                material = comp.get_material(index)
                mat_path = asset_path(material) or "<none>"
                material_counts[mat_path] += 1
                if mat_path not in material_details and material:
                    material_details[mat_path] = get_material_details(material)
                comp_entry["materials"].append(
                    {
                        "slot": index,
                        "path": mat_path,
                        "class": class_name(material),
                    }
                )

            actor_entry["components"].append(comp_entry)

        building_actors.append(actor_entry)

    report = {
        "map": MAP_PATH,
        "level_loaded": loaded,
        "visible_actor_count": len(actors),
        "static_mesh_actor_count": all_static_mesh_actor_count,
        "building_actor_count": len(building_actors),
        "world_partition_load_related_methods": wp_methods,
        "nanite_counts_by_component_mesh": dict(nanite_counts),
        "top_meshes": mesh_counts.most_common(50),
        "top_materials": material_counts.most_common(80),
        "material_details": material_details,
        "building_actors": building_actors,
    }

    json_path = os.path.join(report_dir, "building_material_audit.json")
    txt_path = os.path.join(report_dir, "building_material_audit.txt")

    with open(json_path, "w", encoding="utf-8") as f:
        json.dump(report, f, indent=2, sort_keys=True)

    lines = []
    lines.append("WU Building Material Audit")
    lines.append("Map: {}".format(MAP_PATH))
    lines.append("Visible actors: {}".format(len(actors)))
    lines.append("Static mesh actors: {}".format(all_static_mesh_actor_count))
    lines.append("Building-like actors: {}".format(len(building_actors)))
    lines.append("")
    lines.append("Nanite counts by component mesh:")
    for key, count in nanite_counts.most_common():
        lines.append("  {}: {}".format(key, count))
    lines.append("")
    lines.append("Top materials:")
    for mat_path, count in material_counts.most_common(30):
        detail = material_details.get(mat_path, {})
        parent = detail.get("parent")
        lines.append("  {} uses | {}".format(count, mat_path))
        if parent:
            lines.append("    parent: {}".format(parent))
    lines.append("")
    lines.append("Top meshes:")
    for mesh_path, count in mesh_counts.most_common(30):
        lines.append("  {} uses | {}".format(count, mesh_path))

    with open(txt_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines))

    log("Wrote {}".format(json_path))
    log("Wrote {}".format(txt_path))


if __name__ == "__main__":
    main()
