// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Identity/WUIdentityTypes.h"
#include "Styling/SlateBrush.h"
#include "WUSocialWidget.generated.h"

class SEditableTextBox;
class SScrollBox;
class SVerticalBox;
class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUSocialIncludeOfflineChangedSignature, bool, bIncludeOffline);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWUSocialRefreshRequestedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUSocialInviteRequestedSignature, const FString&, CharacterNameOrId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWUSocialKickRequestedSignature, const FString&, CharacterId);

/**
 * Native first-pass social window focused on the selected character's club.
 */
UCLASS(Blueprintable)
class WU_API UWUSocialWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUSocialWidget(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable, Category = "Social")
	FWUSocialIncludeOfflineChangedSignature OnIncludeOfflineChanged;

	UPROPERTY(BlueprintAssignable, Category = "Social")
	FWUSocialRefreshRequestedSignature OnRefreshRequested;

	UPROPERTY(BlueprintAssignable, Category = "Social")
	FWUSocialInviteRequestedSignature OnInviteRequested;

	UPROPERTY(BlueprintAssignable, Category = "Social")
	FWUSocialKickRequestedSignature OnKickRequested;

	UFUNCTION(BlueprintCallable, Category = "Social")
	void ShowSocial(const FWUClubSummary& InClub, bool bInIncludeOffline);

	UFUNCTION(BlueprintCallable, Category = "Social")
	void HideSocial();

	UFUNCTION(BlueprintPure, Category = "Social")
	bool IsSocialOpen() const;

	UFUNCTION(BlueprintCallable, Category = "Social")
	void SetClubRoster(const TArray<FWUClubMemberSummary>& InMembers, bool bInIncludeOffline);

	UFUNCTION(BlueprintCallable, Category = "Social")
	void SetStatusText(const FText& InStatusText);

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D PanelSize = FVector2D(620.0f, 560.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(0.92f, 0.94f, 0.98f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MutedValueColor = FSlateColor(FLinearColor(0.58f, 0.55f, 0.50f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	EVisibility GetSocialVisibility() const;
	FText GetClubTitleText() const;
	FText GetClubSubtitleText() const;
	FText GetFooterText() const;
	FText GetMotdText() const;
	FText GetStatusText() const;
	EVisibility GetStatusVisibility() const;
	ECheckBoxState GetIncludeOfflineCheckState() const;
	void HandleIncludeOfflineChanged(ECheckBoxState NewState);
	void HandleSearchTextChanged(const FText& NewText);
	FReply HandleRefreshClicked();
	FReply HandleCloseClicked();
	FReply HandleInviteClicked();
	FReply HandleKickSelectedClicked();
	FReply HandleMemberClicked(FString CharacterId, FString DisplayName);
	FReply HandleDisabledActionClicked();
	void RefreshRosterRows();
	TSharedRef<SWidget> CreateTabsRow() const;
	TSharedRef<SWidget> CreateControlsRow();
	TSharedRef<SWidget> CreateRosterHeader() const;
	TSharedRef<SWidget> CreateRosterRow(const FWUClubMemberSummary& Member) const;
	TSharedRef<SWidget> CreateFooterActions();
	bool CanInviteMembers() const;
	bool CanKickMembers() const;
	bool CanManageClub() const;
	bool MemberMatchesSearch(const FWUClubMemberSummary& Member) const;
	FText GetMemberZoneText(const FWUClubMemberSummary& Member) const;
	FText GetMemberPathText(const FWUClubMemberSummary& Member) const;
	FText GetMemberLastOnlineText(const FWUClubMemberSummary& Member) const;
	FSlateColor GetMemberNameColor(const FWUClubMemberSummary& Member) const;
	FSlateColor GetRankColor(EWUClubRank Rank) const;
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));

private:

	bool bSocialOpen = false;
	bool bIncludeOffline = false;
	FWUClubSummary Club;
	TArray<FWUClubMemberSummary> Members;
	FText SearchText;
	FText StatusText;

	TSharedPtr<SVerticalBox> RosterRowsBox;
	TSharedPtr<SScrollBox> RosterScrollBox;
	TSharedPtr<SEditableTextBox> SearchBox;
	TSharedPtr<SEditableTextBox> InviteTextBox;
	FString SelectedMemberCharacterId;
	FString SelectedMemberName;

	FSlateBrush PanelBrush;
};
