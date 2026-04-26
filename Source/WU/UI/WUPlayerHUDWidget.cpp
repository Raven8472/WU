// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUPlayerHUDWidget.h"
#include "WUCharacter.h"
#include "WUPlayerController.h"
#include "GameFramework/PlayerController.h"

AWUCharacter* UWUPlayerHUDWidget::GetWUCharacter() const
{
	if (const APlayerController* PC = GetOwningPlayer())
	{
		return Cast<AWUCharacter>(PC->GetPawn());
	}

	return nullptr;
}

AWUPlayerController* UWUPlayerHUDWidget::GetWUPlayerController() const
{
	return Cast<AWUPlayerController>(GetOwningPlayer());
}

float UWUPlayerHUDWidget::GetHealthPercent() const
{
	if (const AWUCharacter* Character = GetWUCharacter())
	{
		return Character->GetHealthPercent();
	}

	return 0.0f;
}

int32 UWUPlayerHUDWidget::GetHealthRounded() const
{
	if (const AWUCharacter* Character = GetWUCharacter())
	{
		return FMath::RoundToInt(Character->GetCurrentHealth());
	}

	return 0;
}

int32 UWUPlayerHUDWidget::GetMaxHealthRounded() const
{
	if (const AWUCharacter* Character = GetWUCharacter())
	{
		return FMath::RoundToInt(Character->GetMaxHealth());
	}

	return 0;
}

bool UWUPlayerHUDWidget::IsPlayerDead() const
{
	if (const AWUCharacter* Character = GetWUCharacter())
	{
		return Character->IsDead();
	}

	return false;
}

bool UWUPlayerHUDWidget::IsPlayerReleasedSpirit() const
{
	if (const AWUCharacter* Character = GetWUCharacter())
	{
		return Character->IsReleasedSpirit();
	}

	return false;
}

FText UWUPlayerHUDWidget::GetDeathPromptText() const
{
	if (const AWUCharacter* Character = GetWUCharacter())
	{
		if (!Character->IsDead())
		{
			return FText::GetEmpty();
		}

		if (!Character->HasReleased())
		{
			return FText::FromString(TEXT("Release to graveyard"));
		}

		if (Character->CanReviveAtCorpse())
		{
			return FText::FromString(TEXT("Revive at corpse"));
		}

		return FText::FromString(TEXT("Return to your corpse"));
	}

	return FText::GetEmpty();
}

AWUCharacter* UWUPlayerHUDWidget::GetTargetCharacter() const
{
	if (const AWUPlayerController* PC = GetWUPlayerController())
	{
		return PC->GetCurrentTarget();
	}

	return nullptr;
}

bool UWUPlayerHUDWidget::HasTarget() const
{
	return GetTargetCharacter() != nullptr;
}

float UWUPlayerHUDWidget::GetTargetHealthPercent() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return Target->GetHealthPercent();
	}

	return 0.0f;
}

int32 UWUPlayerHUDWidget::GetTargetHealthRounded() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return FMath::RoundToInt(Target->GetCurrentHealth());
	}

	return 0;
}

int32 UWUPlayerHUDWidget::GetTargetMaxHealthRounded() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return FMath::RoundToInt(Target->GetMaxHealth());
	}

	return 0;
}

FText UWUPlayerHUDWidget::GetTargetDisplayName() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return Target->GetDisplayName();
	}

	return FText::GetEmpty();
}

UTexture2D* UWUPlayerHUDWidget::GetTargetPortraitTexture() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return Target->GetPortraitTexture();
	}

	return nullptr;
}
