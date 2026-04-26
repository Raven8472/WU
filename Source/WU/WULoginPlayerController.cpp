// Copyright Epic Games, Inc. All Rights Reserved.

#include "WULoginPlayerController.h"
#include "UI/WUCharacterSelectWidget.h"
#include "UI/WULoginScreenWidget.h"

AWULoginPlayerController::AWULoginPlayerController()
{
	LoginWidgetClass = UWULoginScreenWidget::StaticClass();
	CharacterSelectWidgetClass = UWUCharacterSelectWidget::StaticClass();
}

void AWULoginPlayerController::BeginPlay()
{
	Super::BeginPlay();

	ApplyMenuInputMode();

	if (IsLocalPlayerController() && LoginWidgetClass)
	{
		LoginWidget = CreateWidget<UWULoginScreenWidget>(this, LoginWidgetClass);
		if (LoginWidget)
		{
			LoginWidget->OnLoginFlowReady.AddDynamic(this, &AWULoginPlayerController::ShowCharacterSelect);
			LoginWidget->AddToPlayerScreen(10);
		}
	}
}

void AWULoginPlayerController::ShowCharacterSelect()
{
	if (!IsLocalPlayerController() || !CharacterSelectWidgetClass)
	{
		return;
	}

	if (LoginWidget)
	{
		LoginWidget->RemoveFromParent();
		LoginWidget = nullptr;
	}

	if (!CharacterSelectWidget)
	{
		CharacterSelectWidget = CreateWidget<UWUCharacterSelectWidget>(this, CharacterSelectWidgetClass);
	}

	if (CharacterSelectWidget && !CharacterSelectWidget->IsInViewport())
	{
		CharacterSelectWidget->AddToPlayerScreen(10);
	}

	ApplyMenuInputMode();
}

void AWULoginPlayerController::ApplyMenuInputMode()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}
