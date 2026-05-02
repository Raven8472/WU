// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUCharacterPanelWidget.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "InputCoreTypes.h"
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

	const TArray<EWUEquipmentSlot> LeftSlots =
	{
		EWUEquipmentSlot::Hat,
		EWUEquipmentSlot::Shirt,
		EWUEquipmentSlot::Undershirt,
		EWUEquipmentSlot::Gloves,
		EWUEquipmentSlot::Ring1,
		EWUEquipmentSlot::Ring2,
		EWUEquipmentSlot::Bracelet1,
		EWUEquipmentSlot::Bracelet2
	};

	const TArray<EWUEquipmentSlot> RightSlots =
	{
		EWUEquipmentSlot::ChestRobes,
		EWUEquipmentSlot::Belt,
		EWUEquipmentSlot::PantsSkirt,
		EWUEquipmentSlot::Shoes,
		EWUEquipmentSlot::Earring1,
		EWUEquipmentSlot::Earring2,
		EWUEquipmentSlot::Nicnak1,
		EWUEquipmentSlot::Nicnak2,
		EWUEquipmentSlot::Wand
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
	return bPanelOpen ? EVisibility::Visible : EVisibility::Collapsed;
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
	const int32 ExperienceToNext = Character->GetExperienceToNextLevel();
	const FString ExperienceText = ExperienceToNext > 0
		? FString::Printf(
			TEXT("Experience: %d / %d (%.0f%%)"),
			Character->GetCharacterExperience(),
			ExperienceToNext,
			Character->GetExperiencePercent() * 100.0f)
		: FString(TEXT("Experience: Max Level"));
	return FText::FromString(FString::Printf(
		TEXT("Health: %.0f / %.0f\nMagic: %.0f / %.0f\n%s\nCrit: %.2f%%\nSpell Power: +%.1f%%"),
		Character->GetCurrentHealth(),
		Character->GetMaxHealth(),
		Character->GetCurrentMagic(),
		Character->GetMaxMagic(),
		*ExperienceText,
		Stats.CriticalChancePercent,
		Stats.SpellPowerPercent));
}

TSharedRef<SWidget> UWUCharacterPanelWidget::CreateEquipmentColumn(const TArray<EWUEquipmentSlot>& Slots)
{
	TSharedRef<SVerticalBox> Column = SNew(SVerticalBox);

	for (int32 SlotIndex = 0; SlotIndex < Slots.Num(); ++SlotIndex)
	{
		Column->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, SlotIndex > 0 ? 7.0f : 0.0f, 0.0f, 0.0f))
		[
			CreateEquipmentSlot(Slots[SlotIndex])
		];
	}

	return Column;
}

TSharedRef<SWidget> UWUCharacterPanelWidget::CreateEquipmentSlot(EWUEquipmentSlot EquipmentSlot)
{
	return SNew(SBox)
		.WidthOverride(154.0f)
		.HeightOverride(EquipmentSlotSize.Y)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Padding(FMargin(0.0f))
			.ToolTipText_Lambda([this, EquipmentSlot]()
			{
				return GetEquipmentSlotTooltipText(EquipmentSlot);
			})
			.OnMouseButtonDown_Lambda([this, EquipmentSlot](const FGeometry&, const FPointerEvent& PointerEvent)
			{
				if (PointerEvent.GetEffectingButton() == EKeys::LeftMouseButton
					|| PointerEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					return HandleEquipmentSlotClicked(EquipmentSlot);
				}

				return FReply::Unhandled();
			})
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
						.ColorAndOpacity_Lambda([this, EquipmentSlot]()
						{
							return GetEquipmentSlotTint(EquipmentSlot);
						})
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(FMargin(7.0f, 0.0f, 0.0f, 0.0f))
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(WUInventory::EquipmentSlotToText(EquipmentSlot))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
						.ColorAndOpacity(LabelColor)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text_Lambda([this, EquipmentSlot]()
						{
							return GetEquipmentSlotItemText(EquipmentSlot);
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 9))
						.ColorAndOpacity_Lambda([this, EquipmentSlot]()
						{
							return GetEquipmentSlotItemColor(EquipmentSlot);
						})
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
					]
				]
			]
		];
}

FText UWUCharacterPanelWidget::GetEquipmentSlotItemText(EWUEquipmentSlot EquipmentSlot) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventoryItem EquippedItem;
	if (!Character || !Character->GetEquippedItem(EquipmentSlot, EquippedItem))
	{
		return LOCTEXT("EmptyEquipmentSlot", "Empty");
	}

	return FText::FromString(EquippedItem.DisplayName);
}

FText UWUCharacterPanelWidget::GetEquipmentSlotTooltipText(EWUEquipmentSlot EquipmentSlot) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventoryItem EquippedItem;
	if (!Character || !Character->GetEquippedItem(EquipmentSlot, EquippedItem))
	{
		return WUInventory::EquipmentSlotToText(EquipmentSlot);
	}

	return FText::Format(
		LOCTEXT("EquipmentSlotTooltip", "{0}\n{1}"),
		WUInventory::EquipmentSlotToText(EquipmentSlot),
		FText::FromString(EquippedItem.DisplayName));
}

FSlateColor UWUCharacterPanelWidget::GetEquipmentSlotItemColor(EWUEquipmentSlot EquipmentSlot) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventoryItem EquippedItem;
	if (!Character || !Character->GetEquippedItem(EquipmentSlot, EquippedItem))
	{
		return MutedLabelColor;
	}

	return ValueColor;
}

FLinearColor UWUCharacterPanelWidget::GetEquipmentSlotTint(EWUEquipmentSlot EquipmentSlot) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventoryItem EquippedItem;
	if (!Character || !Character->GetEquippedItem(EquipmentSlot, EquippedItem))
	{
		return FLinearColor(1.0f, 1.0f, 1.0f, 0.88f);
	}

	FLinearColor Tint = EquippedItem.ItemTint;
	Tint.A = 0.92f;
	return Tint;
}

FReply UWUCharacterPanelWidget::HandleEquipmentSlotClicked(EWUEquipmentSlot EquipmentSlot)
{
	AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventoryItem EquippedItem;
	if (Character && Character->GetEquippedItem(EquipmentSlot, EquippedItem))
	{
		Character->UnequipEquipmentSlot(EquipmentSlot);
		InvalidateLayoutAndVolatility();
		return FReply::Handled();
	}

	return FReply::Unhandled();
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
