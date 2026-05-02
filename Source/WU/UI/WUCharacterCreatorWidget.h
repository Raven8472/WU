// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "Styling/SlateBrush.h"
#include "WUCharacterCreatorWidget.generated.h"

class SEditableTextBox;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUCharacterCreateRequestedSignature, const FWUCharacterCreateRequest&, Request);

/**
 * Native character creator shell.
 * Builds a future-backend-safe FWUCharacterCreateRequest and drives the local preview actor.
 */
UCLASS(Blueprintable)
class WU_API UWUCharacterCreatorWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUCharacterCreatorWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "Character Creation")
	FWUCharacterCreateRequestedSignature OnCreateRequested;

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void ShowCreator();

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void HideCreator();

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void ToggleCreator();

	UFUNCTION(BlueprintPure, Category = "Character Creation")
	bool IsCreatorOpen() const;

	UFUNCTION(BlueprintPure, Category = "Character Creation")
	FWUCharacterCreateRequest GetCurrentRequest() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D CreatorSize = FVector2D(430.0f, 690.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(1.0f, 0.96f, 0.9f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MutedValueColor = FSlateColor(FLinearColor(0.56f, 0.52f, 0.44f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	EVisibility GetCreatorVisibility() const;
	void RefreshPreview() const;
	void SetRace(EWUCharacterRace NewRace);
	void SetSex(EWUCharacterSex NewSex);
	void CycleSkinPreset(int32 Delta);
	void CycleHeadPreset(int32 Delta);
	void CycleHairStyle(int32 Delta);
	void CycleHairColor(int32 Delta);
	void CycleEyeColor(int32 Delta);
	void CycleBrowStyle(int32 Delta);
	void CycleBeardStyle(int32 Delta);
	void RotatePreview(float YawDelta) const;
	void SubmitCreateRequest();

	FReply HandleCreateClicked();
	void HandleNameChanged(const FText& Text);

	TSharedRef<SWidget> CreateHeaderText(const FText& Text) const;
	TSharedRef<SWidget> CreateValueText(TAttribute<FText> Text) const;
	TSharedRef<SWidget> CreateButton(const FText& Text, TFunction<FReply()> Handler) const;
	TSharedRef<SWidget> CreateStepperRow(const FText& Label, TAttribute<FText> ValueText, TFunction<void()> PreviousHandler, TFunction<void()> NextHandler) const;

	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));

private:

	bool bCreatorOpen = false;
	FWUCharacterCreateRequest CurrentRequest;

	TSharedPtr<SEditableTextBox> NameInputBox;
	FSlateBrush PanelBrush;
};
