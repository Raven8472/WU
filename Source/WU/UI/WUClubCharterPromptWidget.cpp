// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUClubCharterPromptWidget.h"

#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUClubCharterPromptWidget"

UWUClubCharterPromptWidget::UWUClubCharterPromptWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UWUClubCharterPromptWidget::ShowPrompt(int32 SlotIndex, const FWUInventoryItem& Item)
{
	ActiveSlotIndex = SlotIndex;
	ActiveItem = Item;
	StatusText = FText::GetEmpty();
	bPromptOpen = true;
	SetVisibility(ESlateVisibility::Visible);

	if (ClubNameTextBox.IsValid())
	{
		ClubNameTextBox->SetText(FText::GetEmpty());
		FSlateApplication::Get().SetKeyboardFocus(ClubNameTextBox);
	}

	InvalidateLayoutAndVolatility();
}

void UWUClubCharterPromptWidget::HidePrompt()
{
	if (!bPromptOpen)
	{
		return;
	}

	bPromptOpen = false;
	ActiveSlotIndex = INDEX_NONE;
	ActiveItem = FWUInventoryItem();
	StatusText = FText::GetEmpty();
	SetVisibility(ESlateVisibility::Collapsed);
	InvalidateLayoutAndVolatility();
}

void UWUClubCharterPromptWidget::SetStatusText(const FText& NewStatusText)
{
	StatusText = NewStatusText;
	InvalidateLayoutAndVolatility();
}

bool UWUClubCharterPromptWidget::IsPromptOpen() const
{
	return bPromptOpen;
}

TSharedRef<SWidget> UWUClubCharterPromptWidget::RebuildWidget()
{
	return SNew(SBox)
		.WidthOverride(360.0f)
		.HeightOverride(210.0f)
		.Visibility_UObject(this, &UWUClubCharterPromptWidget::GetPromptVisibility)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.018f, 0.016f, 0.012f, 0.94f))
			.Padding(FMargin(16.0f, 14.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Title", "Club Charter"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f)))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("Prompt", "Enter a club name."))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.90f, 0.92f, 0.96f, 1.0f)))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
				[
					SAssignNew(ClubNameTextBox, SEditableTextBox)
					.HintText(LOCTEXT("NameHint", "Club name"))
					.SelectAllTextWhenFocused(true)
					.OnTextCommitted_UObject(this, &UWUClubCharterPromptWidget::HandleNameCommitted)
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.VAlign(VAlign_Top)
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUClubCharterPromptWidget::GetStatusText)
					.Visibility_UObject(this, &UWUClubCharterPromptWidget::GetStatusVisibility)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.78f, 0.34f, 1.0f)))
					.AutoWrapText(true)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("Cancel", "Cancel"))
						.OnClicked_UObject(this, &UWUClubCharterPromptWidget::HandleCancelClicked)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
					[
						SNew(SButton)
						.Text(LOCTEXT("Create", "Create"))
						.OnClicked_UObject(this, &UWUClubCharterPromptWidget::HandleCreateClicked)
					]
				]
			]
		];
}

FText UWUClubCharterPromptWidget::GetStatusText() const
{
	return StatusText;
}

EVisibility UWUClubCharterPromptWidget::GetPromptVisibility() const
{
	return bPromptOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

EVisibility UWUClubCharterPromptWidget::GetStatusVisibility() const
{
	return StatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

FReply UWUClubCharterPromptWidget::HandleCreateClicked()
{
	const FString ClubName = GetTrimmedClubName();
	if (ClubName.Len() < 3 || ClubName.Len() > 32)
	{
		SetStatusText(LOCTEXT("InvalidName", "Club name must be 3-32 characters."));
		return FReply::Handled();
	}

	SetStatusText(LOCTEXT("CreatingClub", "Creating club..."));
	OnClubNameSubmitted.Broadcast(ClubName, ActiveSlotIndex, ActiveItem);
	return FReply::Handled();
}

FReply UWUClubCharterPromptWidget::HandleCancelClicked()
{
	HidePrompt();
	OnPromptCancelled.Broadcast();
	return FReply::Handled();
}

void UWUClubCharterPromptWidget::HandleNameCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		HandleCreateClicked();
	}
}

FString UWUClubCharterPromptWidget::GetTrimmedClubName() const
{
	FString ClubName = ClubNameTextBox.IsValid() ? ClubNameTextBox->GetText().ToString() : FString();
	ClubName.TrimStartAndEndInline();
	return ClubName;
}

#undef LOCTEXT_NAMESPACE
