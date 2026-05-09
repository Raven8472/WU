// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUVendorWidget.h"
#include "Engine/GameInstance.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "WUCharacter.h"

#define LOCTEXT_NAMESPACE "WUVendorWidget"

UWUVendorWidget::UWUVendorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Large_9Slice.T_HUD_Panel_Large_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

void UWUVendorWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWUClientSessionSubsystem* Session = GetSessionSubsystem())
	{
		Session->OnVendorPurchaseCompleted.AddDynamic(this, &UWUVendorWidget::HandleVendorPurchaseCompleted);
		Session->OnRequestFailed.AddDynamic(this, &UWUVendorWidget::HandleSessionRequestFailed);
	}
}

void UWUVendorWidget::NativeDestruct()
{
	if (UWUClientSessionSubsystem* Session = GetSessionSubsystem())
	{
		Session->OnVendorPurchaseCompleted.RemoveDynamic(this, &UWUVendorWidget::HandleVendorPurchaseCompleted);
		Session->OnRequestFailed.RemoveDynamic(this, &UWUVendorWidget::HandleSessionRequestFailed);
	}

	Super::NativeDestruct();
}

TSharedRef<SWidget> UWUVendorWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, VendorSize, FMargin(0.24f));

	return SNew(SBox)
		.WidthOverride(VendorSize.X)
		.HeightOverride(VendorSize.Y)
		.Visibility_UObject(this, &UWUVendorWidget::GetVendorVisibility)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(16.0f, 14.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUVendorWidget::GetVendorTitleText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
					.ColorAndOpacity(LabelColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 12.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUVendorWidget::GetVendorGreetingText)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(ValueColor)
					.AutoWrapText(true)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					CreateVendorItemsSection()
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUVendorWidget::GetVendorStatusText)
					.Visibility_UObject(this, &UWUVendorWidget::GetVendorStatusVisibility)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					.ColorAndOpacity(ValueColor)
					.AutoWrapText(true)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]
			]
		];
}

void UWUVendorWidget::ShowVendor(FName VendorTableId, FText VendorName, FText GreetingText)
{
	ActiveVendorTableId = VendorTableId;
	ActiveVendorName = VendorName;
	ActiveGreetingText = GreetingText;
	StatusText = FText::GetEmpty();
	bVendorOpen = true;
	InvalidateLayoutAndVolatility();
}

void UWUVendorWidget::HideVendor()
{
	bVendorOpen = false;
	StatusText = FText::GetEmpty();
	InvalidateLayoutAndVolatility();
}

bool UWUVendorWidget::IsVendorOpen() const
{
	return bVendorOpen;
}

EVisibility UWUVendorWidget::GetVendorVisibility() const
{
	return bVendorOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

FText UWUVendorWidget::GetVendorTitleText() const
{
	return ActiveVendorName.IsEmpty() ? LOCTEXT("FallbackVendorTitle", "Vendor") : ActiveVendorName;
}

FText UWUVendorWidget::GetVendorGreetingText() const
{
	return ActiveGreetingText.IsEmpty() ? LOCTEXT("FallbackVendorGreeting", "Have a look.") : ActiveGreetingText;
}

FText UWUVendorWidget::GetVendorStatusText() const
{
	return StatusText;
}

EVisibility UWUVendorWidget::GetVendorStatusVisibility() const
{
	return StatusText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

TSharedRef<SWidget> UWUVendorWidget::CreateVendorItemsSection()
{
	const FWUVendorTable* VendorTable = WUInventory::FindVendorTable(ActiveVendorTableId);
	if (!VendorTable)
	{
		return SNew(STextBlock)
			.Text(LOCTEXT("VendorTableMissing", "This vendor has nothing for sale yet."))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(MutedLabelColor)
			.AutoWrapText(true);
	}

	TSharedRef<SScrollBox> ItemList = SNew(SScrollBox);
	for (const FWUVendorItem& VendorItem : VendorTable->Items)
	{
		ItemList->AddSlot()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
		[
			CreateVendorItemRow(VendorItem)
		];
	}

	return ItemList;
}

TSharedRef<SWidget> UWUVendorWidget::CreateVendorItemRow(const FWUVendorItem& VendorItem)
{
	const FWUInventoryItem* ItemDefinition = WUInventory::FindItemDefinition(VendorItem.ItemId);
	const FText ItemName = ItemDefinition
		? FText::FromString(ItemDefinition->DisplayName)
		: FText::FromName(VendorItem.ItemId);
	const FText PriceText = WUInventory::FormatCurrencyAmountKnuts(VendorItem.PriceKnuts);
	const FName ItemId = VendorItem.ItemId;

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.04f, 0.035f, 0.028f, 0.72f))
		.Padding(FMargin(8.0f, 6.0f))
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
					.Text(ItemName)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
					.ColorAndOpacity(LabelColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 3.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(PriceText)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
					.ColorAndOpacity(ValueColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
				]
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(10.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SButton)
				.Text(LOCTEXT("BuyButton", "Buy"))
				.OnClicked_UObject(this, &UWUVendorWidget::HandleBuyClicked, ItemId)
			]
		];
}

FReply UWUVendorWidget::HandleBuyClicked(FName ItemId)
{
	UWUClientSessionSubsystem* Session = GetSessionSubsystem();
	if (!Session)
	{
		StatusText = LOCTEXT("NoSessionStatus", "No backend session is available.");
		InvalidateLayoutAndVolatility();
		return FReply::Handled();
	}

	StatusText = LOCTEXT("BuyingStatus", "Buying...");
	InvalidateLayoutAndVolatility();
	Session->PurchaseSelectedVendorItem(ActiveVendorTableId.ToString(), ItemId.ToString());
	return FReply::Handled();
}

void UWUVendorWidget::HandleVendorPurchaseCompleted(const FWUBackendVendorPurchase& Purchase)
{
	if (!bVendorOpen || Purchase.VendorTableId != ActiveVendorTableId.ToString())
	{
		return;
	}

	if (AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn()))
	{
		Character->AddInventoryItemById(FName(*Purchase.ItemId));
	}

	StatusText = FText::Format(
		LOCTEXT("PurchasedStatus", "Purchased {0}."),
		FText::FromString(Purchase.DisplayName));
	InvalidateLayoutAndVolatility();
}

void UWUVendorWidget::HandleSessionRequestFailed(const FString& ErrorMessage)
{
	if (!bVendorOpen)
	{
		return;
	}

	StatusText = FText::FromString(ErrorMessage);
	InvalidateLayoutAndVolatility();
}

void UWUVendorWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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

UWUClientSessionSubsystem* UWUVendorWidget::GetSessionSubsystem() const
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		return GameInstance->GetSubsystem<UWUClientSessionSubsystem>();
	}

	return nullptr;
}

#undef LOCTEXT_NAMESPACE
