// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUCharacterPanelWidget.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "WUCharacter.h"

#define LOCTEXT_NAMESPACE "WUCharacterPanelWidget"

namespace
{
	FText GetBloodStatusDisplayText(EWUCharacterRace BloodStatus)
	{
		switch (BloodStatus)
		{
		case EWUCharacterRace::Pureblood:
			return LOCTEXT("PurebloodStatus", "Pureblood");
		case EWUCharacterRace::Halfblood:
			return LOCTEXT("HalfbloodStatus", "Half-blood");
		case EWUCharacterRace::Mudblood:
			return LOCTEXT("MugglebornStatus", "Muggle-born");
		default:
			return LOCTEXT("UnknownStatus", "Unknown");
		}
	}
}

UWUCharacterPanelWidget::UWUCharacterPanelWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Large_9Slice.T_HUD_Panel_Large_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> SlotAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_ActionSlot_Normal.T_HUD_ActionSlot_Normal"));
	if (SlotAsset.Succeeded())
	{
		SlotTexture = SlotAsset.Object;
	}
}

TSharedRef<SWidget> UWUCharacterPanelWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, PanelSize, FMargin(0.24f));
	ConfigureImageBrush(SlotBrush, SlotTexture, EquipmentSlotSize);

	const TArray<FText> LeftSlots =
	{
		LOCTEXT("HatSlot", "Hat"),
		LOCTEXT("ShirtSlot", "Shirt"),
		LOCTEXT("UndershirtSlot", "Undershirt"),
		LOCTEXT("GlovesSlot", "Gloves"),
		LOCTEXT("RingOneSlot", "Ring 1"),
		LOCTEXT("RingTwoSlot", "Ring 2"),
		LOCTEXT("BraceletOneSlot", "Bracelet 1"),
		LOCTEXT("BraceletTwoSlot", "Bracelet 2")
	};

	const TArray<FText> RightSlots =
	{
		LOCTEXT("ChestRobesSlot", "Chest / Robes"),
		LOCTEXT("BeltSlot", "Belt"),
		LOCTEXT("PantsSkirtSlot", "Pants / Skirt"),
		LOCTEXT("ShoesSlot", "Shoes"),
		LOCTEXT("EarringOneSlot", "Earring 1"),
		LOCTEXT("EarringTwoSlot", "Earring 2"),
		LOCTEXT("NicnakOneSlot", "Nicnak 1"),
		LOCTEXT("NicnakTwoSlot", "Nicnak 2"),
		LOCTEXT("WandSlot", "Wand")
	};

	return SNew(SBox)
		.WidthOverride(PanelSize.X)
		.HeightOverride(PanelSize.Y)
		.Visibility_UObject(this, &UWUCharacterPanelWidget::GetPanelVisibility)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(18.0f, 14.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CharacterPanelTitle", "Character"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(LabelColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 3.0f, 0.0f, 12.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUCharacterPanelWidget::GetCharacterSubtitleText)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
					.ColorAndOpacity(MutedLabelColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						CreateEquipmentColumn(LeftSlots)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(16.0f, 0.0f))
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
						[
							SNew(STextBlock)
							.Text_UObject(this, &UWUCharacterPanelWidget::GetCharacterNameText)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
							.ColorAndOpacity(ValueColor)
							.Justification(ETextJustify::Center)
							.ShadowOffset(FVector2D(1.0f, 1.0f))
							.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 0.0f, 0.0f, 14.0f))
						[
							SNew(SBorder)
							.BorderImage(&PanelBrush)
							.Padding(FMargin(12.0f, 10.0f))
							[
								SNew(STextBlock)
								.Text_UObject(this, &UWUCharacterPanelWidget::GetDerivedStatsText)
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
								.ColorAndOpacity(ValueColor)
								.LineHeightPercentage(1.15f)
								.ShadowOffset(FVector2D(1.0f, 1.0f))
								.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
							]
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						[
							SNew(SBorder)
							.BorderImage(&PanelBrush)
							.Padding(FMargin(12.0f, 10.0f))
							[
								SNew(STextBlock)
								.Text_UObject(this, &UWUCharacterPanelWidget::GetPrimaryStatsText)
								.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
								.ColorAndOpacity(ValueColor)
								.LineHeightPercentage(1.15f)
								.ShadowOffset(FVector2D(1.0f, 1.0f))
								.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
							]
						]
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						CreateEquipmentColumn(RightSlots)
					]
				]
			]
		];
}

void UWUCharacterPanelWidget::TogglePanel()
{
	bPanelOpen ? HidePanel() : ShowPanel();
}

void UWUCharacterPanelWidget::ShowPanel()
{
	bPanelOpen = true;
	InvalidateLayoutAndVolatility();
}

void UWUCharacterPanelWidget::HidePanel()
{
	bPanelOpen = false;
	InvalidateLayoutAndVolatility();
}

bool UWUCharacterPanelWidget::IsPanelOpen() const
{
	return bPanelOpen;
}

EVisibility UWUCharacterPanelWidget::GetPanelVisibility() const
{
	return bPanelOpen ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

FText UWUCharacterPanelWidget::GetCharacterNameText() const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return LOCTEXT("FallbackCharacterName", "Character");
	}

	const FText CharacterName = Character->GetDisplayName();
	return CharacterName.IsEmpty() ? LOCTEXT("FallbackCharacterName", "Character") : CharacterName;
}

FText UWUCharacterPanelWidget::GetCharacterSubtitleText() const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return LOCTEXT("FallbackCharacterSubtitle", "Equipment");
	}

	return FText::Format(
		LOCTEXT("CharacterSubtitleFormat", "Level {0} {1}"),
		FText::AsNumber(Character->GetCharacterLevel()),
		GetBloodStatusDisplayText(Character->GetBloodStatus()));
}

FText UWUCharacterPanelWidget::GetPrimaryStatsText() const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return LOCTEXT("FallbackPrimaryStats", "Nerve\nAmbition\nWit\nPatience\nCunning\nKnowledge");
	}

	const FWUPrimaryStats Stats = Character->GetPrimaryStats();
	return FText::FromString(FString::Printf(
		TEXT("Nerve: %d\nAmbition: %d\nWit: %d\nPatience: %d\nCunning: %d\nKnowledge: %d"),
		Stats.Nerve,
		Stats.Ambition,
		Stats.Wit,
		Stats.Patience,
		Stats.Cunning,
		Stats.Knowledge));
}

FText UWUCharacterPanelWidget::GetDerivedStatsText() const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	if (!Character)
	{
		return LOCTEXT("FallbackDerivedStats", "Health\nMagic");
	}

	const FWUDerivedStats Stats = Character->GetDerivedStats();
	return FText::FromString(FString::Printf(
		TEXT("Health: %.0f / %.0f\nMagic: %.0f / %.0f\nCrit: %.2f%%\nSpell Power: +%.1f%%"),
		Character->GetCurrentHealth(),
		Character->GetMaxHealth(),
		Character->GetCurrentMagic(),
		Character->GetMaxMagic(),
		Stats.CriticalChancePercent,
		Stats.SpellPowerPercent));
}

TSharedRef<SWidget> UWUCharacterPanelWidget::CreateEquipmentColumn(const TArray<FText>& SlotNames) const
{
	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	for (int32 SlotIndex = 0; SlotIndex < SlotNames.Num(); ++SlotIndex)
	{
		Column->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, SlotIndex > 0 ? 7.0f : 0.0f, 0.0f, 0.0f))
		[
			CreateEquipmentSlot(SlotNames[SlotIndex])
		];
	}

	return Column;
}

TSharedRef<SWidget> UWUCharacterPanelWidget::CreateEquipmentSlot(const FText& SlotName) const
{
	return SNew(SBox)
		.WidthOverride(154.0f)
		.HeightOverride(EquipmentSlotSize.Y)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SBox)
				.WidthOverride(EquipmentSlotSize.X)
				.HeightOverride(EquipmentSlotSize.Y)
				[
					SNew(SImage)
					.Image(&SlotBrush)
					.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.88f))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(SlotName)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(LabelColor)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
			]
		];
}

void UWUCharacterPanelWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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
