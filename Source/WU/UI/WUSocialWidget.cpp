// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUSocialWidget.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "WUPlayerController.h"

#define LOCTEXT_NAMESPACE "WUSocialWidget"

namespace
{
	const FLinearColor OnlineNameColor(0.0f, 0.84f, 0.38f, 1.0f);
	const FLinearColor OfflineNameColor(0.48f, 0.46f, 0.42f, 1.0f);
	const FLinearColor PresidentRankColor(1.0f, 0.82f, 0.32f, 1.0f);
	const FLinearColor OfficerRankColor(0.72f, 0.86f, 1.0f, 1.0f);
	const FLinearColor RowTintA(0.035f, 0.032f, 0.028f, 0.72f);
	const FLinearColor RowTintB(0.060f, 0.055f, 0.046f, 0.72f);

	FString LastOnlineDeltaText(const FDateTime& LastOnlineUtc)
	{
		if (LastOnlineUtc.GetTicks() <= 0)
		{
			return TEXT("-");
		}

		const FTimespan Delta = FDateTime::UtcNow() - LastOnlineUtc;
		if (Delta.GetDays() > 0)
		{
			return FString::Printf(TEXT("%dd"), Delta.GetDays());
		}

		if (Delta.GetHours() > 0)
		{
			return FString::Printf(TEXT("%dh"), Delta.GetHours());
		}

		if (Delta.GetMinutes() > 0)
		{
			return FString::Printf(TEXT("%dm"), Delta.GetMinutes());
		}

		return TEXT("<1m");
	}
}

UWUSocialWidget::UWUSocialWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Large_9Slice.T_HUD_Panel_Large_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

TSharedRef<SWidget> UWUSocialWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, PanelSize, FMargin(0.24f));

	return SNew(SBox)
		.WidthOverride(PanelSize.X)
		.HeightOverride(PanelSize.Y)
		.Visibility_UObject(this, &UWUSocialWidget::GetSocialVisibility)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(16.0f, 14.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.VAlign(VAlign_Center)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.Text_UObject(this, &UWUSocialWidget::GetClubTitleText)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
							.ColorAndOpacity(LabelColor)
							.ShadowOffset(FVector2D(1.0f, 1.0f))
							.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 2.0f, 0.0f, 0.0f))
						[
							SNew(STextBlock)
							.Text_UObject(this, &UWUSocialWidget::GetClubSubtitleText)
							.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
							.ColorAndOpacity(MutedValueColor)
							.ShadowOffset(FVector2D(1.0f, 1.0f))
							.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Text(LOCTEXT("CloseButton", "X"))
						.OnClicked_UObject(this, &UWUSocialWidget::HandleCloseClicked)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 12.0f, 0.0f, 8.0f))
				[
					CreateTabsRow()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
				[
					CreateControlsRow()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateRosterHeader()
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SAssignNew(RosterScrollBox, SScrollBox)
					.Orientation(Orient_Vertical)

					+ SScrollBox::Slot()
					[
						SAssignNew(RosterRowsBox, SVerticalBox)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUSocialWidget::GetFooterText)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(ValueColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(SBorder)
					.BorderBackgroundColor(FLinearColor(0.035f, 0.032f, 0.028f, 0.80f))
					.Padding(FMargin(8.0f, 6.0f))
					[
						SNew(STextBlock)
						.Text_UObject(this, &UWUSocialWidget::GetMotdText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
						.ColorAndOpacity(ValueColor)
						.AutoWrapText(true)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					CreateFooterActions()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUSocialWidget::GetStatusText)
					.Visibility_UObject(this, &UWUSocialWidget::GetStatusVisibility)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					.ColorAndOpacity(LabelColor)
					.AutoWrapText(true)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]
			]
		];
}

void UWUSocialWidget::ShowSocial(const FWUClubSummary& InClub, bool bInIncludeOffline)
{
	Club = InClub;
	bIncludeOffline = bInIncludeOffline;
	bSocialOpen = true;
	StatusText = Club.HasClub() ? LOCTEXT("RosterLoadingStatus", "Loading roster...") : LOCTEXT("NoClubStatus", "This character is not in a club.");
	RefreshRosterRows();
	InvalidateLayoutAndVolatility();
}

void UWUSocialWidget::HideSocial()
{
	bSocialOpen = false;
	StatusText = FText::GetEmpty();
	RefreshRosterRows();
	InvalidateLayoutAndVolatility();
}

bool UWUSocialWidget::IsSocialOpen() const
{
	return bSocialOpen;
}

void UWUSocialWidget::SetClubRoster(const TArray<FWUClubMemberSummary>& InMembers, bool bInIncludeOffline)
{
	Members = InMembers;
	bIncludeOffline = bInIncludeOffline;

	if (!SelectedMemberCharacterId.IsEmpty())
	{
		bool bSelectionStillExists = false;
		for (const FWUClubMemberSummary& Member : Members)
		{
			if (Member.CharacterId == SelectedMemberCharacterId)
			{
				SelectedMemberName = Member.DisplayName;
				bSelectionStillExists = true;
				break;
			}
		}

		if (!bSelectionStillExists)
		{
			SelectedMemberCharacterId.Empty();
			SelectedMemberName.Empty();
		}
	}

	StatusText = FText::GetEmpty();
	RefreshRosterRows();
	InvalidateLayoutAndVolatility();
}

void UWUSocialWidget::SetStatusText(const FText& InStatusText)
{
	StatusText = InStatusText;
	InvalidateLayoutAndVolatility();
}

EVisibility UWUSocialWidget::GetSocialVisibility() const
{
	return bSocialOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

FText UWUSocialWidget::GetClubTitleText() const
{
	return Club.HasClub() && !Club.Name.IsEmpty()
		? FText::FromString(Club.Name)
		: LOCTEXT("SocialTitleFallback", "Club");
}

FText UWUSocialWidget::GetClubSubtitleText() const
{
	if (!Club.HasClub())
	{
		return LOCTEXT("SocialSubtitleNoClub", "No club membership");
	}

	return FText::Format(
		LOCTEXT("SocialSubtitleFormat", "{0} permissions"),
		FText::FromString(WUIdentity::ClubRankToString(Club.Rank)));
}

FText UWUSocialWidget::GetFooterText() const
{
	int32 OnlineCount = 0;
	for (const FWUClubMemberSummary& Member : Members)
	{
		if (Member.bIsOnline)
		{
			++OnlineCount;
		}
	}

	return FText::Format(
		LOCTEXT("SocialFooterFormat", "{0} Club Members ({1} Online)"),
		FText::AsNumber(Members.Num()),
		FText::AsNumber(OnlineCount));
}

FText UWUSocialWidget::GetMotdText() const
{
	if (!Club.PublicNote.IsEmpty())
	{
		return FText::Format(LOCTEXT("ClubMotdFormat", "Club Message Of The Day: {0}"), FText::FromString(Club.PublicNote));
	}

	return LOCTEXT("ClubMotdFallback", "Club Message Of The Day:");
}

FText UWUSocialWidget::GetStatusText() const
{
	return StatusText;
}

EVisibility UWUSocialWidget::GetStatusVisibility() const
{
	return StatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

ECheckBoxState UWUSocialWidget::GetIncludeOfflineCheckState() const
{
	return bIncludeOffline ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

void UWUSocialWidget::HandleIncludeOfflineChanged(ECheckBoxState NewState)
{
	const bool bNewIncludeOffline = NewState == ECheckBoxState::Checked;
	if (bIncludeOffline == bNewIncludeOffline)
	{
		return;
	}

	bIncludeOffline = bNewIncludeOffline;
	OnIncludeOfflineChanged.Broadcast(bIncludeOffline);
	RefreshRosterRows();
}

void UWUSocialWidget::HandleSearchTextChanged(const FText& NewText)
{
	SearchText = NewText;
	RefreshRosterRows();
}

FReply UWUSocialWidget::HandleRefreshClicked()
{
	if (Club.HasClub())
	{
		StatusText = LOCTEXT("RosterRefreshingStatus", "Refreshing roster...");
		OnRefreshRequested.Broadcast();
	}

	return FReply::Handled();
}

FReply UWUSocialWidget::HandleCloseClicked()
{
	if (AWUPlayerController* WUPC = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		WUPC->HideSocialPanel();
	}
	else
	{
		HideSocial();
	}

	return FReply::Handled();
}

FReply UWUSocialWidget::HandleInviteClicked()
{
	if (!InviteTextBox)
	{
		return FReply::Handled();
	}

	const FString InviteTarget = InviteTextBox->GetText().ToString().TrimStartAndEnd();
	if (InviteTarget.IsEmpty())
	{
		StatusText = LOCTEXT("InviteTargetRequired", "Enter a character name or id to invite.");
		InvalidateLayoutAndVolatility();
		return FReply::Handled();
	}

	StatusText = FText::Format(LOCTEXT("InviteSending", "Sending invite to {0}..."), FText::FromString(InviteTarget));
	OnInviteRequested.Broadcast(InviteTarget);
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUSocialWidget::HandleKickSelectedClicked()
{
	if (SelectedMemberCharacterId.IsEmpty())
	{
		StatusText = LOCTEXT("KickSelectionRequired", "Select a club member to kick.");
		InvalidateLayoutAndVolatility();
		return FReply::Handled();
	}

	StatusText = FText::Format(
		LOCTEXT("KickSending", "Removing {0} from the club..."),
		FText::FromString(SelectedMemberName.IsEmpty() ? SelectedMemberCharacterId : SelectedMemberName));
	OnKickRequested.Broadcast(SelectedMemberCharacterId);
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUSocialWidget::HandleMemberClicked(FString CharacterId, FString DisplayName)
{
	SelectedMemberCharacterId = CharacterId;
	SelectedMemberName = DisplayName;
	StatusText = FText::Format(LOCTEXT("MemberSelected", "Selected {0}."), FText::FromString(DisplayName));
	RefreshRosterRows();
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUSocialWidget::HandleDisabledActionClicked()
{
	StatusText = LOCTEXT("SocialActionPending", "Rank controls need the next backend route.");
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

void UWUSocialWidget::RefreshRosterRows()
{
	if (!RosterRowsBox)
	{
		return;
	}

	RosterRowsBox->ClearChildren();

	int32 VisibleRowIndex = 0;
	for (const FWUClubMemberSummary& Member : Members)
	{
		if (!bIncludeOffline && !Member.bIsOnline)
		{
			continue;
		}

		if (!MemberMatchesSearch(Member))
		{
			continue;
		}

		RosterRowsBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 2.0f))
		[
			SNew(SButton)
			.ContentPadding(FMargin(0.0f))
			.OnClicked_UObject(this, &UWUSocialWidget::HandleMemberClicked, Member.CharacterId, Member.DisplayName)
			[
				SNew(SBorder)
				.BorderBackgroundColor(Member.CharacterId == SelectedMemberCharacterId ? FLinearColor(0.18f, 0.13f, 0.04f, 0.92f) : ((VisibleRowIndex % 2) == 0 ? RowTintA : RowTintB))
				.Padding(FMargin(6.0f, 5.0f))
				[
					CreateRosterRow(Member)
				]
			]
		];

		++VisibleRowIndex;
	}

	if (VisibleRowIndex == 0)
	{
		RosterRowsBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(6.0f, 8.0f))
		[
			SNew(STextBlock)
			.Text(Club.HasClub() ? LOCTEXT("RosterEmpty", "No matching members.") : LOCTEXT("RosterNoClub", "No club roster."))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(MutedValueColor)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
		];
	}
}

TSharedRef<SWidget> UWUSocialWidget::CreateTabsRow() const
{
	const auto CreateTab = [this](const FText& Label, bool bActive)
	{
		return SNew(SBorder)
			.BorderBackgroundColor(bActive ? FLinearColor(0.16f, 0.12f, 0.05f, 0.92f) : FLinearColor(0.035f, 0.032f, 0.028f, 0.72f))
			.Padding(FMargin(13.0f, 5.0f))
			[
				SNew(STextBlock)
				.Text(Label)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(bActive ? LabelColor : MutedValueColor)
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0.0f, 0.0f, 4.0f, 0.0f))
		[
			CreateTab(LOCTEXT("FriendsTab", "Friends"), false)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0.0f, 0.0f, 4.0f, 0.0f))
		[
			CreateTab(LOCTEXT("WhoTab", "Who"), false)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(0.0f, 0.0f, 4.0f, 0.0f))
		[
			CreateTab(LOCTEXT("ClubTab", "Club"), true)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			CreateTab(LOCTEXT("PartyTab", "Party"), false)
		];
}

TSharedRef<SWidget> UWUSocialWidget::CreateControlsRow()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SAssignNew(SearchBox, SEditableTextBox)
			.HintText(LOCTEXT("SearchHint", "Search"))
			.OnTextChanged_UObject(this, &UWUSocialWidget::HandleSearchTextChanged)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
		.VAlign(VAlign_Center)
		[
			SNew(SCheckBox)
			.IsChecked_UObject(this, &UWUSocialWidget::GetIncludeOfflineCheckState)
			.OnCheckStateChanged_UObject(this, &UWUSocialWidget::HandleIncludeOfflineChanged)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ShowOfflineMembers", "Show Offline Members"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(ValueColor)
			]
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("RefreshRosterButton", "Refresh"))
			.OnClicked_UObject(this, &UWUSocialWidget::HandleRefreshClicked)
		];
}

TSharedRef<SWidget> UWUSocialWidget::CreateRosterHeader() const
{
	const auto HeaderText = [this](const FText& Text)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.24f).Padding(FMargin(6.0f, 0.0f))[HeaderText(LOCTEXT("NameHeader", "Name"))]
		+ SHorizontalBox::Slot().FillWidth(0.22f).Padding(FMargin(4.0f, 0.0f))[HeaderText(LOCTEXT("ZoneHeader", "Zone"))]
		+ SHorizontalBox::Slot().FillWidth(0.07f).Padding(FMargin(4.0f, 0.0f))[HeaderText(LOCTEXT("LevelHeader", "Lvl"))]
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(FMargin(4.0f, 0.0f))[HeaderText(LOCTEXT("PathHeader", "Path"))]
		+ SHorizontalBox::Slot().FillWidth(0.16f).Padding(FMargin(4.0f, 0.0f))[HeaderText(LOCTEXT("RankHeader", "Rank"))]
		+ SHorizontalBox::Slot().FillWidth(0.16f).Padding(FMargin(4.0f, 0.0f))[HeaderText(LOCTEXT("LastOnlineHeader", "Last"))];
}

TSharedRef<SWidget> UWUSocialWidget::CreateRosterRow(const FWUClubMemberSummary& Member) const
{
	const auto CellText = [](const FText& Text, const FSlateColor& Color)
	{
		return SNew(STextBlock)
			.Text(Text)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
			.ColorAndOpacity(Color)
			.Clipping(EWidgetClipping::ClipToBounds)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(0.24f).Padding(FMargin(0.0f))[CellText(FText::FromString(Member.DisplayName), GetMemberNameColor(Member))]
		+ SHorizontalBox::Slot().FillWidth(0.22f).Padding(FMargin(4.0f, 0.0f))[CellText(GetMemberZoneText(Member), Member.bIsOnline ? ValueColor : MutedValueColor)]
		+ SHorizontalBox::Slot().FillWidth(0.07f).Padding(FMargin(4.0f, 0.0f))[CellText(FText::AsNumber(Member.Level), Member.bIsOnline ? ValueColor : MutedValueColor)]
		+ SHorizontalBox::Slot().FillWidth(0.15f).Padding(FMargin(4.0f, 0.0f))[CellText(GetMemberPathText(Member), Member.bIsOnline ? ValueColor : MutedValueColor)]
		+ SHorizontalBox::Slot().FillWidth(0.16f).Padding(FMargin(4.0f, 0.0f))[CellText(FText::FromString(WUIdentity::ClubRankToString(Member.Rank)), GetRankColor(Member.Rank))]
		+ SHorizontalBox::Slot().FillWidth(0.16f).Padding(FMargin(4.0f, 0.0f))[CellText(GetMemberLastOnlineText(Member), Member.bIsOnline ? ValueColor : MutedValueColor)];
}

TSharedRef<SWidget> UWUSocialWidget::CreateFooterActions()
{
	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SAssignNew(InviteTextBox, SEditableTextBox)
				.HintText(LOCTEXT("InviteHint", "Character name or id"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
			[
			SNew(SButton)
			.Text(LOCTEXT("InviteButton", "Invite"))
			.IsEnabled_Lambda([this]() { return CanInviteMembers(); })
			.OnClicked_UObject(this, &UWUSocialWidget::HandleInviteClicked)
		]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
			[
			SNew(SButton)
			.Text(LOCTEXT("KickSelectedButton", "Kick Selected"))
			.IsEnabled_Lambda([this]() { return CanKickMembers(); })
			.OnClicked_UObject(this, &UWUSocialWidget::HandleKickSelectedClicked)
		]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(0.0f, 0.0f, 8.0f, 0.0f))
			[
				SNew(SButton)
				.Text(LOCTEXT("ClubInformationButton", "Club Information"))
				.OnClicked_UObject(this, &UWUSocialWidget::HandleDisabledActionClicked)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.Text(LOCTEXT("ClubControlButton", "Club Control"))
				.IsEnabled_Lambda([this]() { return CanManageClub(); })
				.OnClicked_UObject(this, &UWUSocialWidget::HandleDisabledActionClicked)
			]
		];
}

bool UWUSocialWidget::CanInviteMembers() const
{
	return Club.HasClub()
		&& (Club.Rank == EWUClubRank::President || Club.HasPermission(EWUClubPermission::Invite));
}

bool UWUSocialWidget::CanKickMembers() const
{
	return Club.HasClub()
		&& (Club.Rank == EWUClubRank::President || Club.HasPermission(EWUClubPermission::Kick));
}

bool UWUSocialWidget::CanManageClub() const
{
	return Club.HasClub()
		&& (Club.Rank == EWUClubRank::President || Club.HasPermission(EWUClubPermission::ManagePreferences));
}

bool UWUSocialWidget::MemberMatchesSearch(const FWUClubMemberSummary& Member) const
{
	const FString Needle = SearchText.ToString().TrimStartAndEnd();
	if (Needle.IsEmpty())
	{
		return true;
	}

	return Member.DisplayName.Contains(Needle, ESearchCase::IgnoreCase)
		|| Member.LocationDisplayName.Contains(Needle, ESearchCase::IgnoreCase)
		|| Member.Path.Contains(Needle, ESearchCase::IgnoreCase)
		|| WUIdentity::ClubRankToString(Member.Rank).Contains(Needle, ESearchCase::IgnoreCase);
}

FText UWUSocialWidget::GetMemberZoneText(const FWUClubMemberSummary& Member) const
{
	if (!Member.LocationDisplayName.IsEmpty())
	{
		return FText::FromString(Member.LocationDisplayName);
	}

	return Member.bIsOnline ? LOCTEXT("UnknownZone", "Unknown") : LOCTEXT("OfflineZone", "-");
}

FText UWUSocialWidget::GetMemberPathText(const FWUClubMemberSummary& Member) const
{
	if (!Member.Path.IsEmpty())
	{
		return FText::FromString(Member.Path);
	}

	return LOCTEXT("FallbackPath", "Student");
}

FText UWUSocialWidget::GetMemberLastOnlineText(const FWUClubMemberSummary& Member) const
{
	return Member.bIsOnline ? LOCTEXT("OnlineNow", "Now") : FText::FromString(LastOnlineDeltaText(Member.LastOnlineUtc));
}

FSlateColor UWUSocialWidget::GetMemberNameColor(const FWUClubMemberSummary& Member) const
{
	return FSlateColor(Member.bIsOnline ? OnlineNameColor : OfflineNameColor);
}

FSlateColor UWUSocialWidget::GetRankColor(EWUClubRank Rank) const
{
	switch (Rank)
	{
	case EWUClubRank::President:
		return FSlateColor(PresidentRankColor);
	case EWUClubRank::Officer:
		return FSlateColor(OfficerRankColor);
	case EWUClubRank::Recruit:
		return MutedValueColor;
	case EWUClubRank::Member:
	default:
		return ValueColor;
	}
}

void UWUSocialWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
{
	Brush.SetResourceObject(Texture);
	Brush.ImageSize = ImageSize;
	Brush.Margin = Margin;
	const bool bHasMargin = !FMath::IsNearlyZero(Margin.Left)
		|| !FMath::IsNearlyZero(Margin.Top)
		|| !FMath::IsNearlyZero(Margin.Right)
		|| !FMath::IsNearlyZero(Margin.Bottom);
	Brush.DrawAs = bHasMargin ? ESlateBrushDrawType::Box : ESlateBrushDrawType::Image;
}

#undef LOCTEXT_NAMESPACE
