// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WULoginPlayerController.generated.h"

class UWUCharacterSelectWidget;
class UWULoginScreenWidget;

UCLASS()
class WU_API AWULoginPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWULoginPlayerController();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "WU|Login")
	TSubclassOf<UWULoginScreenWidget> LoginWidgetClass;

	UPROPERTY(EditAnywhere, Category = "WU|Login")
	TSubclassOf<UWUCharacterSelectWidget> CharacterSelectWidgetClass;

	UPROPERTY()
	TObjectPtr<UWULoginScreenWidget> LoginWidget;

	UPROPERTY()
	TObjectPtr<UWUCharacterSelectWidget> CharacterSelectWidget;

private:
	UFUNCTION()
	void ShowCharacterSelect();

	void ApplyMenuInputMode();
};
