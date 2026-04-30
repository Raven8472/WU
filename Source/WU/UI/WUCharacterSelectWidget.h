// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "Styling/SlateBrush.h"
#include "WUCharacterSelectWidget.generated.h"

class SVerticalBox;
class UWUCharacterCreatorWidget;
class UTexture;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWUEnterGameRequestedSignature);

UCLASS(Blueprintable)
class WU_API UWUCharacterSelectWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWUCharacterSelectWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "WU|Character Select")
	FWUEnterGameRequestedSignature OnEnterGameRequested;

	void SetStatusText(const FText& NewStatusText);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> BackgroundTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	FString BackgroundSourcePath = TEXT("UI/Login/Login_Background.png");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Creation")
	TSubclassOf<UWUCharacterCreatorWidget> CharacterCreatorWidgetClass;

private:
	UFUNCTION()
	void HandleCharactersLoaded(const TArray<FWUBackendCharacterSummary>& LoadedCharacters);

	UFUNCTION()
	void HandleCharacterCreated(const FWUBackendCharacterSummary& Character);

	UFUNCTION()
	void HandleRequestFailed(const FString& ErrorMessage);

	UFUNCTION()
	void HandleCreatorCreateRequested(const FWUCharacterCreateRequest& Request);

	FReply HandleCreateClicked();
	FReply HandleRefreshClicked();
	FReply HandleSelectClicked(FString CharacterId);
	FReply HandleEnterGameClicked();
	FText GetStatusText() const;

	TSharedRef<SWidget> CreateCharacterCreatorPanel();
	TSharedRef<SWidget> CreateCharacterPreviewPanel();
	void RefreshCharacterRows();
	TSharedRef<SWidget> CreateCharacterRow(const FWUBackendCharacterSummary& Character);
	UTexture* ResolvePreviewTexture() const;
	UTexture2D* ResolveBackgroundTexture();

private:
	FSlateBrush BackgroundBrush;
	FSlateBrush PanelBrush;
	FSlateBrush PreviewBrush;
	FText StatusText;
	TSharedPtr<SVerticalBox> CharacterListBox;

	UPROPERTY(Transient)
	TObjectPtr<UWUCharacterCreatorWidget> CharacterCreatorWidget;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LoadedBackgroundTexture;
};
