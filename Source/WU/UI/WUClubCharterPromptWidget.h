// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/WUInventoryTypes.h"
#include "WUClubCharterPromptWidget.generated.h"

class SEditableTextBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWUClubCharterPromptSubmitSignature, const FString&, ClubName, int32, SlotIndex, FWUInventoryItem, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWUClubCharterPromptCancelledSignature);

/**
 * Temporary dev flow for using a Club Charter item to name and found a club.
 */
UCLASS(Blueprintable)
class WU_API UWUClubCharterPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UWUClubCharterPromptWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Club Charter")
	void ShowPrompt(int32 SlotIndex, const FWUInventoryItem& Item);

	UFUNCTION(BlueprintCallable, Category = "Club Charter")
	void HidePrompt();

	UFUNCTION(BlueprintCallable, Category = "Club Charter")
	void SetStatusText(const FText& NewStatusText);

	UFUNCTION(BlueprintPure, Category = "Club Charter")
	bool IsPromptOpen() const;

	UPROPERTY(BlueprintAssignable, Category = "Club Charter")
	FWUClubCharterPromptSubmitSignature OnClubNameSubmitted;

	UPROPERTY(BlueprintAssignable, Category = "Club Charter")
	FWUClubCharterPromptCancelledSignature OnPromptCancelled;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FText GetStatusText() const;
	EVisibility GetPromptVisibility() const;
	EVisibility GetStatusVisibility() const;
	FReply HandleCreateClicked();
	FReply HandleCancelClicked();
	void HandleNameCommitted(const FText& Text, ETextCommit::Type CommitType);
	FString GetTrimmedClubName() const;

	bool bPromptOpen = false;
	int32 ActiveSlotIndex = INDEX_NONE;
	FWUInventoryItem ActiveItem;
	FText StatusText;
	TSharedPtr<SEditableTextBox> ClubNameTextBox;
};
