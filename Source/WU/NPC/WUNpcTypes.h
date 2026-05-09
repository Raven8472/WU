// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WUNpcTypes.generated.h"

UENUM(BlueprintType)
enum class EWUNpcRole : uint8
{
	Ambient,
	QuestGiver,
	Vendor,
	Banker
};

USTRUCT(BlueprintType)
struct WU_API FWUNpcProfile
{
	GENERATED_BODY()

	FWUNpcProfile();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName NpcId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	EWUNpcRole Role = EWUNpcRole::Ambient;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName QuestId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FName VendorTableId = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
	FText InteractionPrompt;
};
