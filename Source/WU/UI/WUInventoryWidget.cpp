// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUInventoryWidget.h"
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "WUCharacter.h"

#define LOCTEXT_NAMESPACE "WUInventoryWidget"

namespace
{
	constexpr int32 InventoryColumnsPerBag = 4;
}

UWUInventoryWidget::UWUInventoryWidget(const FObjectInitializer& ObjectInitializer)
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

	static ConstructorHelpers::FObjectFinder<UTexture2D> DisabledSlotAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_ActionSlot_Disabled.T_HUD_ActionSlot_Disabled"));
	if (DisabledSlotAsset.Succeeded())
	{
		DisabledSlotTexture = DisabledSlotAsset.Object;
	}
}

TSharedRef<SWidget> UWUInventoryWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, InventorySize, FMargin(0.24f));
	ConfigureImageBrush(SlotBrush, SlotTexture, InventorySlotSize);
	ConfigureImageBrush(DisabledSlotBrush, DisabledSlotTexture, InventorySlotSize);

	return SNew(SBox)
		.WidthOverride(InventorySize.X)
		.HeightOverride(InventorySize.Y)
		.Visibility_UObject(this, &UWUInventoryWidget::GetInventoryVisibility)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(16.0f, 12.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("InventoryTitle", "Inventory"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
					.ColorAndOpacity(LabelColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 10.0f, 0.0f, 12.0f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						CreateBagSlot(0)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateBagSlot(1)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateBagSlot(2)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateBagSlot(3)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateBagSlot(4)
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.HAlign(HAlign_Right)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::Format(LOCTEXT("InventoryCapacity", "{0} slots"), FText::AsNumber(GetUnlockedInventorySlotCount())))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
						.ColorAndOpacity(LabelColor)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.AutoWidth()
					[
						CreateBagSection(0)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(16.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateBagSection(1)
					]

					+ SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(16.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateBagSection(2)
					]
				]
			]
		];
}

void UWUInventoryWidget::ToggleInventory()
{
	bInventoryOpen ? HideInventory() : ShowInventory();
}

void UWUInventoryWidget::ShowInventory()
{
	bInventoryOpen = true;
	InvalidateLayoutAndVolatility();
}

void UWUInventoryWidget::HideInventory()
{
	bInventoryOpen = false;
	InvalidateLayoutAndVolatility();
}

bool UWUInventoryWidget::IsInventoryOpen() const
{
	return bInventoryOpen;
}

int32 UWUInventoryWidget::GetUnlockedInventorySlotCount() const
{
	return FMath::Max(0, StartingBagCount) * FMath::Max(0, SlotsPerBag);
}

EVisibility UWUInventoryWidget::GetInventoryVisibility() const
{
	return bInventoryOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

bool UWUInventoryWidget::IsBagSlotUnlocked(int32 BagSlotIndex) const
{
	return BagSlotIndex >= 0 && BagSlotIndex < StartingBagCount;
}

TSharedRef<SWidget> UWUInventoryWidget::CreateBagSlot(int32 BagSlotIndex) const
{
	const bool bUnlocked = IsBagSlotUnlocked(BagSlotIndex);

	return SNew(SBox)
		.WidthOverride(BagSlotSize.X)
		.HeightOverride(BagSlotSize.Y)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(bUnlocked ? &SlotBrush : &DisabledSlotBrush)
				.ColorAndOpacity(bUnlocked ? FLinearColor::White : FLinearColor(0.55f, 0.55f, 0.55f, 0.75f))
			]

			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(bUnlocked
					? FText::AsNumber(BagSlotIndex + 1)
					: LOCTEXT("LockedBagMarker", "+"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(bUnlocked ? LabelColor : MutedLabelColor)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
			]
		];
}

TSharedRef<SWidget> UWUInventoryWidget::CreateBagSection(int32 BagIndex)
{
	TSharedRef<SGridPanel> SlotGrid = SNew(SGridPanel);
	const int32 Rows = SlotsPerBag / InventoryColumnsPerBag;

	for (int32 Row = 0; Row < Rows; ++Row)
	{
		for (int32 Column = 0; Column < InventoryColumnsPerBag; ++Column)
		{
			const int32 SlotIndexInBag = (Row * InventoryColumnsPerBag) + Column;
			const int32 AbsoluteSlotIndex = (BagIndex * SlotsPerBag) + SlotIndexInBag;

			SlotGrid->AddSlot(Column, Row)
			.Padding(FMargin(Column > 0 ? 4.0f : 0.0f, Row > 0 ? 4.0f : 0.0f, 0.0f, 0.0f))
			[
				CreateInventorySlot(AbsoluteSlotIndex)
			];
		}
	}

	return SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(1.0f, 0.0f, 0.0f, 5.0f))
		[
			SNew(STextBlock)
			.Text(FText::Format(LOCTEXT("BagTitle", "Bag {0}"), FText::AsNumber(BagIndex + 1)))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SlotGrid
		];
}

TSharedRef<SWidget> UWUInventoryWidget::CreateInventorySlot(int32 AbsoluteSlotIndex)
{
	return SNew(SBox)
		.WidthOverride(InventorySlotSize.X)
		.HeightOverride(InventorySlotSize.Y)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("NoBrush"))
			.Padding(FMargin(0.0f))
			.ToolTipText_Lambda([this, AbsoluteSlotIndex]()
			{
				return GetInventorySlotTooltipText(AbsoluteSlotIndex);
			})
			.OnMouseButtonDown_Lambda([this, AbsoluteSlotIndex](const FGeometry&, const FPointerEvent& PointerEvent)
			{
				if (PointerEvent.GetEffectingButton() == EKeys::LeftMouseButton
					|| PointerEvent.GetEffectingButton() == EKeys::RightMouseButton)
				{
					return HandleInventorySlotClicked(AbsoluteSlotIndex);
				}

				return FReply::Unhandled();
			})
			[
				SNew(SOverlay)

				+ SOverlay::Slot()
				[
					SNew(SImage)
					.Image(&SlotBrush)
					.ColorAndOpacity_Lambda([this, AbsoluteSlotIndex]()
					{
						return GetInventorySlotTint(AbsoluteSlotIndex);
					})
				]

				+ SOverlay::Slot()
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text_Lambda([this, AbsoluteSlotIndex]()
					{
						return GetInventorySlotText(AbsoluteSlotIndex);
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
					.ColorAndOpacity_Lambda([this, AbsoluteSlotIndex]()
					{
						return GetInventorySlotTextColor(AbsoluteSlotIndex);
					})
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]
			]
		];
}

FText UWUInventoryWidget::GetInventorySlotText(int32 AbsoluteSlotIndex) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventorySlot InventorySlot;
	if (!Character || !Character->GetInventorySlot(AbsoluteSlotIndex, InventorySlot) || !InventorySlot.bHasItem)
	{
		return FText::GetEmpty();
	}

	return FText::FromString(WUInventory::GetShortItemLabel(InventorySlot.Item));
}

FText UWUInventoryWidget::GetInventorySlotTooltipText(int32 AbsoluteSlotIndex) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventorySlot InventorySlot;
	if (!Character || !Character->GetInventorySlot(AbsoluteSlotIndex, InventorySlot) || !InventorySlot.bHasItem)
	{
		return LOCTEXT("EmptyInventorySlotTooltip", "Empty");
	}

	return FText::Format(
		LOCTEXT("InventorySlotTooltip", "{0}\n{1}"),
		FText::FromString(InventorySlot.Item.DisplayName),
		WUInventory::EquipmentSlotToText(InventorySlot.Item.EquipmentSlot));
}

FSlateColor UWUInventoryWidget::GetInventorySlotTextColor(int32 AbsoluteSlotIndex) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventorySlot InventorySlot;
	if (!Character || !Character->GetInventorySlot(AbsoluteSlotIndex, InventorySlot) || !InventorySlot.bHasItem)
	{
		return MutedLabelColor;
	}

	return ValueColor;
}

FLinearColor UWUInventoryWidget::GetInventorySlotTint(int32 AbsoluteSlotIndex) const
{
	const AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventorySlot InventorySlot;
	if (!Character || !Character->GetInventorySlot(AbsoluteSlotIndex, InventorySlot) || !InventorySlot.bHasItem)
	{
		return FLinearColor(1.0f, 1.0f, 1.0f, 0.88f);
	}

	FLinearColor Tint = InventorySlot.Item.ItemTint;
	Tint.A = 0.92f;
	return Tint;
}

FReply UWUInventoryWidget::HandleInventorySlotClicked(int32 AbsoluteSlotIndex)
{
	AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn());
	FWUInventorySlot InventorySlot;
	if (Character && Character->GetInventorySlot(AbsoluteSlotIndex, InventorySlot) && InventorySlot.bHasItem)
	{
		Character->EquipInventorySlot(AbsoluteSlotIndex);
		InvalidateLayoutAndVolatility();
		return FReply::Handled();
	}

	return FReply::Unhandled();
}

void UWUInventoryWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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
