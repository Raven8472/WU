// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "Engine/DataAsset.h"
#include "NPC/WUNpcTypes.h"
#include "WUNpcDefinition.generated.h"

UCLASS(BlueprintType)
class WU_API UWUNpcDefinition : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FWUNpcProfile Profile;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "NPC")
	FWUCharacterAppearance Appearance;
};
