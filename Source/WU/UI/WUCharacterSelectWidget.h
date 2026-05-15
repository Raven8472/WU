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
	void HandleCharacterDeleted(const FString& CharacterId);

	UFUNCTION()
	void HandleRequestFailed(const FString& ErrorMessage);

	UFUNCTION()
	void HandleCreatorCreateRequested(const FWUCharacterCreateRequest& Request);

	UFUNCTION()
	void HandleCreatorClosed();

	FReply HandleCreateClicked();
	FReply HandleRefreshClicked();
	FReply HandleSelectClicked(FString CharacterId);
	FReply HandleDeleteClicked(FString CharacterId, FString CharacterName);
	FReply HandleDeleteSelectedClicked();
	FReply HandleChangeRealmClicked();
	FReply HandleEnterGameClicked();
	FText GetStatusText() const;
	FText GetSelectedRealmText() const;
	FText GetSelectedCharacterNameText() const;
	FText GetSelectedCharacterDetailText() const;
	FText GetSelectedCharacterLocationText() const;
	EVisibility GetSelectPanelVisibility() const;
	EVisibility GetCreatorModeVisibility() const;

	TSharedRef<SWidget> CreateCharacterCreatorPanel();
	TSharedRef<SWidget> CreateCharacterListPanel();
	TSharedRef<SWidget> CreateCreationContextPanel();
	TSharedRef<SWidget> CreateBottomCommandBar();
	TSharedRef<SWidget> CreateActionButton(const FText& Text, FReply(UWUCharacterSelectWidget::*Handler)());
	void RefreshCharacterRows();
	TSharedRef<SWidget> CreateCharacterRow(const FWUBackendCharacterSummary& Character);
	void PreviewSelectedCharacter();
	void PreviewCharacter(const FWUBackendCharacterSummary& Character);
	bool ShouldShowCharacterPreview() const;
	const FWUBackendCharacterSummary* GetSelectedCharacter() const;
	UTexture* ResolvePreviewTexture() const;
	UTexture2D* ResolveBackgroundTexture();
	FText GetRaceDisplayText(EWUCharacterRace Race) const;
	FText GetRaceDescriptionText(EWUCharacterRace Race) const;
	FText GetPathDisplayText(FName PathId) const;
	FText GetPathDescriptionText(FName PathId) const;

private:
	FSlateBrush BackgroundBrush;
	FSlateBrush PanelBrush;
	FSlateBrush PreviewBrush;
	FText StatusText;
	TSharedPtr<SVerticalBox> CharacterListBox;
	bool bHasSelectedCharacterPreview = false;

	UPROPERTY(Transient)
	TObjectPtr<UWUCharacterCreatorWidget> CharacterCreatorWidget;

	UPROPERTY(Transient)
	TObjectPtr<UTexture2D> LoadedBackgroundTexture;
};
