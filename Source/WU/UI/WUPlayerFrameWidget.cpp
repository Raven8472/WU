// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUPlayerFrameWidget.h"
#include "WUCharacter.h"
#include "Engine/Texture2D.h"
#include "GameFramework/PlayerController.h"
#include "UObject/ConstructorHelpers.h"

UWUPlayerFrameWidget::UWUPlayerFrameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<UTexture2D> PortraitFrameAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_PortraitFrame_Player.T_HUD_PortraitFrame_Player"));
	if (PortraitFrameAsset.Succeeded())
	{
		PortraitFrameTexture = PortraitFrameAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> BarFrameAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_BarFrame_Player.T_HUD_BarFrame_Player"));
	if (BarFrameAsset.Succeeded())
	{
		BarFrameTexture = BarFrameAsset.Object;
	}
}

AWUCharacter* UWUPlayerFrameWidget::GetTargetCharacter() const
{
	if (const APlayerController* PC = GetOwningPlayer())
	{
		return Cast<AWUCharacter>(PC->GetPawn());
	}

	return nullptr;
}
