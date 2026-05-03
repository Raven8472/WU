import unreal


OUT_DIR = "/Game/World/Landscape"
MAT_NAME = "M_WU_Landscape_Master"
MI_NAME = "MI_WU_Landscape_Starter"
HOLE_MAT_NAME = "M_WU_Landscape_Hole"
HOLE_MI_NAME = "MI_WU_Landscape_Hole"

TEXTURE_ROOT = "/Game/AssetPacks/STF/Pack03-LandscapePro/Environment/Landscape/Landscape"

LAYERS = [
    ("Grass", "T_ground_grass_01_diffuse", 1.0, 140.0),
    ("DirtPath", "T_ground_dirt_01_diffuse", 0.0, 160.0),
    (
        "RoughBrickGround",
        "/Game/AssetPacks/MurdocMaterials/RoughBrickGround/Textures/RoughBrickGround_basecolor",
        0.0,
        1.0,
    ),
    ("ForestFloor", "T_ground_forest_diffuse", 0.0, 90.0),
    ("Rock", "T_ground_rock_01_diffuse", 0.0, 130.0),
    ("Mud", "T_ground_dirt_01_diffuse", 0.0, 160.0),
]


def asset_path(asset_name):
    if asset_name.startswith("/Game/"):
        return f"{asset_name}.{asset_name.rsplit('/', 1)[-1]}"

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


def connect_expression_any(source, output_name, target, input_names):
    for input_name in input_names:
        if unreal.MaterialEditingLibrary.connect_material_expressions(source, output_name, target, input_name):
            return input_name

    raise RuntimeError(
        f"Failed to connect {source.get_name()}[{output_name}] -> "
        f"{target.get_name()}[{', '.join(input_names)}]"
    )


def connect_property(source, output_name, material_property):
    if not unreal.MaterialEditingLibrary.connect_material_property(source, output_name, material_property):
        raise RuntimeError(f"Failed to connect {source.get_name()}[{output_name}] to {material_property}")


def make_texture_sample(material, layer_name, texture_name, x, y, uv_source=None, parameter_suffix="BaseColor"):
    sample = create_expression(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    sample.set_editor_property("parameter_name", f"{layer_name}_{parameter_suffix}")
    sample.set_editor_property("texture", load_texture(texture_name))

    if uv_source:
        connect_expression_any(uv_source, "", sample, ["Coordinates", "UVs"])

    return sample


def make_layer_weight(material, layer_name, preview_weight, x, y):
    weight = create_expression(material, unreal.MaterialExpressionLandscapeLayerWeight, x, y)
    weight.set_editor_property("parameter_name", layer_name)
    weight.set_editor_property("preview_weight", preview_weight)
    return weight


def make_scalar_parameter(material, parameter_name, default_value, x, y, slider_min=None, slider_max=None):
    parameter = create_expression(material, unreal.MaterialExpressionScalarParameter, x, y)
    parameter.set_editor_property("parameter_name", parameter_name)
    parameter.set_editor_property("default_value", default_value)

    if slider_min is not None:
        set_property_if_present(parameter, "slider_min", slider_min)

    if slider_max is not None:
        set_property_if_present(parameter, "slider_max", slider_max)

    return parameter


def make_custom_input(input_name):
    custom_input = unreal.CustomInput()
    custom_input.set_editor_property("input_name", input_name)
    return custom_input


def make_world_uv(material, parameter_name, default_scale, x, y):
    coords = create_expression(material, unreal.MaterialExpressionLandscapeLayerCoords, x, y)
    coords.set_editor_property("desc", parameter_name)
    if not set_property_if_present(coords, "mapping_scale", default_scale):
        set_property_if_present(coords, "MappingScale", default_scale)
    return coords


def make_macro_variation(material, base_sample, macro_uv, x, y):
    strength = make_scalar_parameter(material, "MacroVariation_Strength", 0.12, x, y + 160, 0.0, 0.5)

    factor = create_expression(material, unreal.MaterialExpressionCustom, x + 260, y)
    factor.set_editor_property("description", "MacroVariation_NoiseFactor")
    factor.set_editor_property("inputs", [make_custom_input("UV"), make_custom_input("Strength")])
    factor.set_editor_property(
        "code",
        "\n".join([
            "float2 p = UV;",
            "float2 i = floor(p);",
            "float2 f = frac(p);",
            "float2 u = f * f * (3.0 - 2.0 * f);",
            "float a = frac(sin(dot(i + float2(0.0, 0.0), float2(127.1, 311.7))) * 43758.5453);",
            "float b = frac(sin(dot(i + float2(1.0, 0.0), float2(127.1, 311.7))) * 43758.5453);",
            "float c = frac(sin(dot(i + float2(0.0, 1.0), float2(127.1, 311.7))) * 43758.5453);",
            "float d = frac(sin(dot(i + float2(1.0, 1.0), float2(127.1, 311.7))) * 43758.5453);",
            "float n = lerp(lerp(a, b, u.x), lerp(c, d, u.x), u.y);",
            "return lerp(1.0 - Strength, 1.0 + Strength, n);",
        ]),
    )
    connect_expression(macro_uv, "", factor, "UV")
    connect_expression(strength, "", factor, "Strength")

    varied = create_expression(material, unreal.MaterialExpressionMultiply, x + 820, y)
    connect_expression(base_sample, "RGB", varied, "A")
    connect_expression(factor, "", varied, "B")
    return varied


def make_auto_slope_blend(material, painted_color, rock_sample):
    slope_start = make_scalar_parameter(material, "AutoSlope_Start", 0.34, -720, 760, 0.0, 1.0)
    slope_end = make_scalar_parameter(material, "AutoSlope_End", 0.58, -720, 820, 0.0, 1.0)
    slope_strength = make_scalar_parameter(material, "AutoSlope_Strength", 1.0, -720, 880, 0.0, 2.0)
    slope_enabled = make_scalar_parameter(material, "AutoSlope_Enabled", 1.0, -720, 940, 0.0, 1.0)

    slope_mask = create_expression(material, unreal.MaterialExpressionCustom, -360, 760)
    slope_mask.set_editor_property("description", "AutoSlope_RockMask")
    slope_mask.set_editor_property(
        "inputs",
        [
            make_custom_input("SlopeStart"),
            make_custom_input("SlopeEnd"),
            make_custom_input("SlopeStrength"),
            make_custom_input("SlopeEnabled"),
        ],
    )
    slope_mask.set_editor_property(
        "code",
        "\n".join([
            "float Steepness = 1.0 - saturate(abs(Parameters.WorldNormal.z));",
            "float SafeSlopeEnd = max(SlopeStart + 0.001, SlopeEnd);",
            "return saturate(smoothstep(SlopeStart, SafeSlopeEnd, Steepness) * SlopeStrength * SlopeEnabled);",
        ]),
    )
    connect_expression(slope_start, "", slope_mask, "SlopeStart")
    connect_expression(slope_end, "", slope_mask, "SlopeEnd")
    connect_expression(slope_strength, "", slope_mask, "SlopeStrength")
    connect_expression(slope_enabled, "", slope_mask, "SlopeEnabled")

    blend = create_expression(material, unreal.MaterialExpressionLinearInterpolate, 140, 520)
    connect_expression(painted_color, "", blend, "A")
    connect_expression(rock_sample, "", blend, "B")
    connect_expression(slope_mask, "", blend, "Alpha")
    return blend


def create_or_clear_material(asset_tools, material_name):
    material_path = f"{OUT_DIR}/{material_name}.{material_name}"
    material = unreal.load_asset(material_path)

    if material:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
        return material

    material = asset_tools.create_asset(material_name, OUT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError(f"Failed to create material {OUT_DIR}/{material_name}")

    return material


def create_or_load_instance(asset_tools, instance_name, parent_material):
    instance_path = f"{OUT_DIR}/{instance_name}.{instance_name}"
    instance = unreal.load_asset(instance_path)
    if not instance:
        instance = asset_tools.create_asset(
            instance_name,
            OUT_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        if not instance:
            raise RuntimeError(f"Failed to create material instance {OUT_DIR}/{instance_name}")

    instance.set_editor_property("parent", parent_material)
    unreal.EditorAssetLibrary.save_loaded_asset(instance)
    return instance


def build_landscape_material(material, enable_holes):
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED if enable_holes else unreal.BlendMode.BLEND_OPAQUE)
    material.set_editor_property("two_sided", False)
    set_property_if_present(material, "opacity_mask_clip_value", 0.333)

    macro_uv = make_world_uv(material, "Macro_Texture_World_Scale", 2600.0, -1500, -240)

    previous = None
    rock_sample = None
    for index, (layer_name, texture_name, preview_weight, detail_scale) in enumerate(LAYERS):
        y = -420 + index * 320
        detail_uv = make_world_uv(material, f"{layer_name}_Detail_Texture_Scale", detail_scale, -1500, y)
        sample = make_texture_sample(material, layer_name, texture_name, -900, y, detail_uv)
        varied_sample = make_macro_variation(material, sample, macro_uv, -900, y + 130)
        weight = make_layer_weight(material, layer_name, preview_weight, 180, y)

        if layer_name == "Rock":
            rock_sample = varied_sample

        if previous:
            connect_expression(previous, "", weight, "Base")
        else:
            connect_expression(varied_sample, "", weight, "Base")

        connect_expression(varied_sample, "", weight, "Layer")
        previous = weight

    if not previous or not rock_sample:
        raise RuntimeError("Landscape material requires painted layer output and a rock layer for auto-slope blending.")

    previous = make_auto_slope_blend(material, previous, rock_sample)

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

    if enable_holes:
        visibility_mask = create_expression(material, unreal.MaterialExpressionLandscapeVisibilityMask, -260, 860)
        connect_property(visibility_mask, "", unreal.MaterialProperty.MP_OPACITY_MASK)

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def main():
    unreal.EditorAssetLibrary.make_directory(OUT_DIR)

    asset_tools = unreal.AssetToolsHelpers.get_asset_tools()

    material = create_or_clear_material(asset_tools, MAT_NAME)
    build_landscape_material(material, enable_holes=False)
    create_or_load_instance(asset_tools, MI_NAME, material)

    hole_material = create_or_clear_material(asset_tools, HOLE_MAT_NAME)
    build_landscape_material(hole_material, enable_holes=True)
    create_or_load_instance(asset_tools, HOLE_MI_NAME, hole_material)

    print(
        "Created/updated "
        f"{OUT_DIR}/{MAT_NAME}, {OUT_DIR}/{MI_NAME}, "
        f"{OUT_DIR}/{HOLE_MAT_NAME}, and {OUT_DIR}/{HOLE_MI_NAME}"
    )


if __name__ == "__main__":
    main()
