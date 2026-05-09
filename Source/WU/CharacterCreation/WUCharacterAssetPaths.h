// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"

struct WU_API FWUCharacterAssetPaths
{
	static const TCHAR* BodyMesh(EWUCharacterSex Sex);
	static const TCHAR* HeadMesh(EWUCharacterSex Sex);
	static const TCHAR* HairMesh(EWUCharacterSex Sex, int32 HairStyleIndex);
	static const TCHAR* BrowsMesh(EWUCharacterSex Sex, int32 BrowStyleIndex);
	static const TCHAR* BeardMesh(EWUCharacterSex Sex, int32 BeardStyleIndex);
	static const TCHAR* PantsMesh(EWUCharacterSex Sex);
	static const TCHAR* HandsMesh(EWUCharacterSex Sex);
	static const TCHAR* BracersMesh(EWUCharacterSex Sex);
	static const TCHAR* StarterChestOutfitMesh(EWUCharacterSex Sex);
	static const TCHAR* StarterChestAddOutfitMesh(EWUCharacterSex Sex);
	static const TCHAR* StarterBeltOutfitMesh(EWUCharacterSex Sex);
	static const TCHAR* StarterBootsOutfitMesh(EWUCharacterSex Sex);
	static const TCHAR* BodyMaterial(EWUCharacterSex Sex, int32 SkinPresetIndex);
	static const TCHAR* HeadMaterial(EWUCharacterSex Sex, int32 HeadPresetIndex);
	static const TCHAR* EyeMaterial(int32 EyeColorIndex);
	static const TCHAR* HairMaterial(int32 HairColorIndex);
	static const TCHAR* UnderhairTexture(EWUCharacterSex Sex, int32 HairColorIndex);
	static const TCHAR* AnimationBlueprint(EWUCharacterSex Sex);
	static const TCHAR* BackpedalAnimation(EWUCharacterSex Sex, float Right);
	static const TCHAR* TurnInPlaceAnimation(EWUCharacterSex Sex, float YawDeltaDegrees);

private:

	static int32 NormalizeIndex(int32 Index, int32 Count);
};
