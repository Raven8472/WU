// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "WULoginScreenWidget.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWULoginFlowReadySignature);

UCLASS(Blueprintable)
class WU_API UWULoginScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWULoginScreenWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "WU|Login")
	FWULoginFlowReadySignature OnLoginFlowReady;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	FString BackgroundSourcePath = TEXT("UI/Login/Login_Background.png");

private:
	UFUNCTION()
	void HandleLoginSucceeded();

	UFUNCTION()
	void HandleCharactersLoaded(const TArray<FWUBackendCharacterSummary>& Characters);

	UFUNCTION()
	void HandleRequestFailed(const FString& ErrorMessage);

	FReply HandleDevLoginClicked();
	FText GetStatusText() const;
	UTexture2D* ResolveBackgroundTexture();

private:
	FSlateBrush BackgroundBrush;
	FSlateBrush PanelBrush;
	FText StatusText;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LoadedBackgroundTexture;
};
