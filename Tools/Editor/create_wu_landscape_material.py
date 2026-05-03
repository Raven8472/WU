import unreal


OUT_DIR = "/Game/World/Landscape"
MAT_NAME = "M_WU_Landscape_Master"
MI_NAME = "MI_WU_Landscape_Starter"

TEXTURE_ROOT = "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape"

LAYERS = [
    ("Grass", "T_ground_grass_01_diffuse", 1.0),
    ("DirtPath", "T_ground_dirt_01_diffuse", 0.0),
    ("ForestFloor", "T_ground_forest_diffuse", 0.0),
    ("Rock", "T_ground_rock_01_diffuse", 0.0),
    ("Mud", "T_ground_dirt_01_diffuse", 0.0),
]


def asset_path(asset_name):
    return f"{TEXTURE_ROOT}/{asset_name}.{asset_name}"


def load_texture(asset_name):
    texture = unreal.load_asset(asset_path(asset_name))
    if not texture:
        raise RuntimeError(f"Missing texture asset: {asset_path(asset_name)}")
    return texture


def create_expression(material, expression_class, x, y):
    return unreal.MaterialEditingLibrary.create_material_expression(
        material,
        expression_class,
        node_pos_x=x,
        node_pos_y=y,
    )


def set_property_if_present(obj, prop_name, value):
    try:
        obj.set_editor_property(prop_name, value)
        return True
    except Exception:
        return False


def connect_expression(source, output_name, target, input_name):
    if not unreal.MaterialEditingLibrary.connect_material_expressions(source, output_name, target, input_name):
        raise RuntimeError(
            f"Failed to connect {source.get_name()}[{output_name}] -> {target.get_name()}[{input_name}]"
        )


def connect_property(source, output_name, material_property):
    if not unreal.MaterialEditingLibrary.connect_material_property(source, output_name, material_property):
        raise RuntimeError(f"Failed to connect {source.get_name()}[{output_name}] to {material_property}")


def make_texture_sample(material, layer_name, texture_name, x, y):
    sample = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    sample.set_editor_property("parameter_name", f"{layer_name}_BaseColor")
    sample.set_editor_property("texture", load_texture(texture_name))
    return sample


def make_layer_weight(material, layer_name, preview_weight, x, y):
    weight = create_expression(material, unreal.MaterialExpressionLandscapeLayerWeight, x, y)
    weight.set_editor_property("parameter_name", layer_name)
    weight.set_editor_property("preview_weight", preview_weight)
    return weight


def main():
    unreal.EditorAssetLibrary.make_directory(OUT_DIR)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
    material_path = f"{OUT_DIR}/{MAT_NAME}.{MAT_NAME}"
    material = unreal.load_asset(material_path)

    if not material:
        material = asset_tools.create_asset(MAT_NAME, OUT_DIR, unreal.Material, unreal.MaterialFactoryNew())
        if not material:
            raise RuntimeError(f"Failed to create material {OUT_DIR}/{MAT_NAME}")
    else:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(material)

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("two_sided", False)

    previous = None
    for index, (layer_name, texture_name, preview_weight) in enumerate(LAYERS):
        y = -420 + index * 180
        sample = make_texture_sample(material, layer_name, texture_name, -900, y)
        weight = make_layer_weight(material, layer_name, preview_weight, -560, y)

        if previous:
            connect_expression(previous, "", weight, "Base")
        else:
            connect_expression(sample, "RGB", weight, "Base")

        connect_expression(sample, "RGB", weight, "Layer")
        previous = weight

    zone_color = create_expression(material, unreal.MaterialExpressionVectorParameter, -900, 520)
    zone_color.set_editor_property("parameter_name", "ZoneMarker_Color")
    zone_color.set_editor_property("default_value", unreal.LinearColor(0.95, 0.68, 0.12, 1.0))

    zone_weight = make_layer_weight(material, "ZoneMarker", 0.0, -560, 520)
    connect_expression(previous, "", zone_weight, "Base")
    connect_expression(zone_color, "", zone_weight, "Layer")
    previous = zone_weight

    roughness = create_expression(material, unreal.MaterialExpressionScalarParameter, -260, 700)
    roughness.set_editor_property("parameter_name", "Roughness")
    roughness.set_editor_property("default_value", 0.82)

    connect_property(previous, "", unreal.MaterialProperty.MP_BASE_COLOR)
    connect_property(roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)

    instance_path = f"{OUT_DIR}/{MI_NAME}.{MI_NAME}"
    instance = unreal.load_asset(instance_path)
    if not instance:
        instance = asset_tools.create_asset(
            MI_NAME,
            OUT_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        if not instance:
            raise RuntimeError(f"Failed to create material instance {OUT_DIR}/{MI_NAME}")

    instance.set_editor_property("parent", material)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)

    print(f"Created/updated {OUT_DIR}/{MAT_NAME} and {OUT_DIR}/{MI_NAME}")


if __name__ == "__main__":
    main()
