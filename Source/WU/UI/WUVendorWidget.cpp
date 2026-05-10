// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUVendorWidget.h"
#include "Engine/GameInstance.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "WUCharacter.h"
#include "WUPlayerController.h"

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
					CreateHeaderSection()
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
				[
					SAssignNew(ActiveContentBox, SBox)
					[
						CreateActiveContent()
					]
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

void UWUVendorWidget::ShowVendor(const FWUNpcProfile& NpcProfile, FText VendorName, FText GreetingText)
{
	ActiveNpcProfile = NpcProfile;
	ActiveVendorTableId = NpcProfile.VendorTableId;
	ActiveVendorName = VendorName;
	ActiveGreetingText = GreetingText;
	ActiveMode = EVendorPanelMode::Dialogue;
	MerchantPageIndex = 0;
	StatusText = FText::GetEmpty();
	bVendorOpen = true;
	RefreshActiveContent();
	InvalidateLayoutAndVolatility();
}

void UWUVendorWidget::HideVendor()
{
	bVendorOpen = false;
	ActiveMode = EVendorPanelMode::Dialogue;
	StatusText = FText::GetEmpty();
	RefreshActiveContent();
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

const FWUVendorTable* UWUVendorWidget::ResolveActiveVendorTable() const
{
	if (const FWUVendorTable* VendorTable = WUInventory::FindVendorTable(ActiveVendorTableId))
	{
		return VendorTable;
	}

	return nullptr;
}

FName UWUVendorWidget::GetResolvedVendorTableId() const
{
	if (const FWUVendorTable* VendorTable = ResolveActiveVendorTable())
	{
		return VendorTable->VendorTableId;
	}

	return ActiveVendorTableId;
}

void UWUVendorWidget::RefreshActiveContent()
{
	if (ActiveContentBox.IsValid())
	{
		ActiveContentBox->SetContent(CreateActiveContent());
	}
}

TSharedRef<SWidget> UWUVendorWidget::CreateActiveContent()
{
	switch (ActiveMode)
	{
	case EVendorPanelMode::Merchant:
		return CreateMerchantSection();
	case EVendorPanelMode::Buyback:
		return CreateBuybackSection();
	default:
		return CreateDialogueSection();
	}
}

TSharedRef<SWidget> UWUVendorWidget::CreateHeaderSection()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_UObject(this, &UWUVendorWidget::GetVendorTitleText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
			.ColorAndOpacity(LabelColor)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Text(LOCTEXT("CloseVendorButton", "X"))
			.OnClicked_UObject(this, &UWUVendorWidget::HandleGoodbyeClicked)
		];
}

TSharedRef<SWidget> UWUVendorWidget::CreateDialogueSection()
{
	TSharedRef<SVerticalBox> DialogueBox = SNew(SVerticalBox);

	DialogueBox->AddSlot()
	.AutoHeight()
	.Padding(FMargin(0.0f, 0.0f, 0.0f, 14.0f))
	[
		SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.20f, 0.14f, 0.08f, 0.86f))
		.Padding(FMargin(14.0f, 12.0f))
		[
			SNew(STextBlock)
			.Text_UObject(this, &UWUVendorWidget::GetVendorGreetingText)
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 12))
			.ColorAndOpacity(ValueColor)
			.AutoWrapText(true)
			.ShadowOffset(FVector2D(1.0f, 1.0f))
			.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
		]
	];

	bool bAddedOption = false;
	if (HasMerchantItems())
	{
		bAddedOption = true;
		DialogueBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
		[
			SNew(SButton)
			.HAlign(HAlign_Left)
			.Text(LOCTEXT("BrowseGoodsOption", "I want to browse your goods."))
			.OnClicked_UObject(this, &UWUVendorWidget::HandleBrowseClicked)
		];
	}

	if (!ActiveNpcProfile.QuestId.IsNone())
	{
		bAddedOption = true;
		DialogueBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
		[
			SNew(SButton)
			.HAlign(HAlign_Left)
			.Text(LOCTEXT("QuestDialogueOption", "Do you have work for me?"))
			.OnClicked_UObject(this, &UWUVendorWidget::HandleQuestClicked)
		];
	}

	if (!bAddedOption)
	{
		DialogueBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoDialogueOptions", "They have nothing else to offer right now."))
			.Font(FCoreStyle::GetDefaultFontStyle("Italic", 11))
			.ColorAndOpacity(MutedLabelColor)
			.AutoWrapText(true)
		];
	}

	DialogueBox->AddSlot()
	.FillHeight(1.0f)
	[
		SNew(SBox)
	];

	DialogueBox->AddSlot()
	.AutoHeight()
	.HAlign(HAlign_Right)
	[
		SNew(SButton)
		.Text(LOCTEXT("GoodbyeButton", "Goodbye"))
		.OnClicked_UObject(this, &UWUVendorWidget::HandleGoodbyeClicked)
	];

	return DialogueBox;
}

TSharedRef<SWidget> UWUVendorWidget::CreateMerchantSection()
{
	const FWUVendorTable* VendorTable = ResolveActiveVendorTable();
	if (!VendorTable)
	{
		return SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(FText::Format(
					LOCTEXT("VendorTableMissing", "No shop table found for {0}."),
					FText::FromName(ActiveVendorTableId)))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
				.ColorAndOpacity(MutedLabelColor)
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SBox)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Right)
			[
				SNew(SButton)
				.Text(LOCTEXT("BackToDialogueFromMissingShop", "Back"))
				.OnClicked_UObject(this, &UWUVendorWidget::HandleGoodbyeClicked)
			];
	}

	const int32 PageIndex = GetClampedMerchantPageIndex();
	const int32 StartIndex = PageIndex * ItemsPerMerchantPage;
	const int32 EndIndex = FMath::Min(StartIndex + ItemsPerMerchantPage, VendorTable->Items.Num());
	TSharedRef<SUniformGridPanel> ItemGrid = SNew(SUniformGridPanel)
		.SlotPadding(FMargin(0.0f, 0.0f, 8.0f, 8.0f));

	for (int32 ItemIndex = StartIndex; ItemIndex < EndIndex; ++ItemIndex)
	{
		const int32 LocalIndex = ItemIndex - StartIndex;
		ItemGrid->AddSlot(LocalIndex % 2, LocalIndex / 2)
		[
			CreateVendorItemCard(VendorTable->Items[ItemIndex])
		];
	}

	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				CreateTabButton(LOCTEXT("MerchantTab", "Merchant"), EVendorPanelMode::Merchant)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
			[
				CreateTabButton(LOCTEXT("BuybackTab", "Buyback"), EVendorPanelMode::Buyback)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(FMargin(0.0f, 12.0f, 0.0f, 8.0f))
		[
			SNew(SScrollBox)
			+ SScrollBox::Slot()
			[
				ItemGrid
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			CreatePageControls()
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
		[
			CreateRepairSection()
		];
}

TSharedRef<SWidget> UWUVendorWidget::CreateBuybackSection()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			[
				CreateTabButton(LOCTEXT("MerchantTabBuybackView", "Merchant"), EVendorPanelMode::Merchant)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
			[
				CreateTabButton(LOCTEXT("BuybackTabBuybackView", "Buyback"), EVendorPanelMode::Buyback)
			]
		]

		+ SVerticalBox::Slot()
		.FillHeight(1.0f)
		.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.04f, 0.035f, 0.028f, 0.72f))
			.Padding(FMargin(12.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("EmptyBuybackMessage", "Buyback history will appear here after item selling is wired."))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(MutedLabelColor)
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> UWUVendorWidget::CreateTabButton(FText Label, EVendorPanelMode TargetMode) const
{
	return SNew(SButton)
		.HAlign(HAlign_Center)
		.IsEnabled(ActiveMode != TargetMode)
		.Text(Label)
		.OnClicked(TargetMode == EVendorPanelMode::Merchant
			? FOnClicked::CreateUObject(const_cast<UWUVendorWidget*>(this), &UWUVendorWidget::HandleMerchantTabClicked)
			: FOnClicked::CreateUObject(const_cast<UWUVendorWidget*>(this), &UWUVendorWidget::HandleBuybackTabClicked));
}

TSharedRef<SWidget> UWUVendorWidget::CreateVendorItemCard(const FWUVendorItem& VendorItem)
{
	const FWUInventoryItem* ItemDefinition = WUInventory::FindItemDefinition(VendorItem.ItemId);
	const FText ItemName = ItemDefinition
		? FText::FromString(ItemDefinition->DisplayName)
		: FText::FromName(VendorItem.ItemId);
	const FText PriceText = WUInventory::FormatCurrencyAmountKnuts(VendorItem.PriceKnuts);
	const FName ItemId = VendorItem.ItemId;
	const FSlateBrush* IconBrush = ItemDefinition ? GetItemIconBrush(*ItemDefinition, FVector2D(42.0f, 42.0f)) : nullptr;

	return SNew(SBox)
		.WidthOverride(204.0f)
		.HeightOverride(78.0f)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.04f, 0.035f, 0.028f, 0.72f))
			.Padding(FMargin(6.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SBorder)
					.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
					.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
					.Padding(FMargin(3.0f))
					[
						SNew(SBox)
						.WidthOverride(42.0f)
						.HeightOverride(42.0f)
						[
							SNew(SImage)
							.Image(IconBrush)
							.Visibility(IconBrush ? EVisibility::HitTestInvisible : EVisibility::Collapsed)
						]
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(ItemName)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity(LabelColor)
						.AutoWrapText(true)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.8f))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 4.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text(PriceText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
						.ColorAndOpacity(ValueColor)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
					]
				]

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
				[
					SNew(SButton)
					.Text(LOCTEXT("BuyButton", "Buy"))
					.OnClicked_UObject(this, &UWUVendorWidget::HandleBuyClicked, ItemId)
				]
			]
		];
}

TSharedRef<SWidget> UWUVendorWidget::CreateRepairSection()
{
	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.04f, 0.035f, 0.028f, 0.72f))
		.Padding(FMargin(8.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("RepairItemsLabel", "Repair Items"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 11))
				.ColorAndOpacity(ValueColor)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.Text(LOCTEXT("RepairOneButton", "Repair"))
				.OnClicked_UObject(this, &UWUVendorWidget::HandleRepairClicked, false)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
			[
				SNew(SButton)
				.Text(LOCTEXT("RepairAllButton", "All"))
				.OnClicked_UObject(this, &UWUVendorWidget::HandleRepairClicked, true)
			]
		];
}

TSharedRef<SWidget> UWUVendorWidget::CreatePageControls() const
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("PreviousMerchantPage", "Prev"))
			.IsEnabled_Lambda([this]()
			{
				return GetClampedMerchantPageIndex() > 0;
			})
			.OnClicked_UObject(const_cast<UWUVendorWidget*>(this), &UWUVendorWidget::HandlePreviousPageClicked)
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.Text_UObject(this, &UWUVendorWidget::GetMerchantPageText)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
			.ColorAndOpacity(LabelColor)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			SNew(SButton)
			.Text(LOCTEXT("NextMerchantPage", "Next"))
			.IsEnabled_Lambda([this]()
			{
				return GetClampedMerchantPageIndex() + 1 < GetMerchantPageCount();
			})
			.OnClicked_UObject(const_cast<UWUVendorWidget*>(this), &UWUVendorWidget::HandleNextPageClicked)
		];
}

const FSlateBrush* UWUVendorWidget::GetItemIconBrush(const FWUInventoryItem& Item, const FVector2D& ImageSize)
{
	const FString& IconTexturePath = Item.IconTexturePath;
	if (IconTexturePath.IsEmpty())
	{
		return nullptr;
	}

	if (const FSlateBrush* CachedBrush = IconBrushCache.Find(IconTexturePath))
	{
		return CachedBrush;
	}

	UTexture2D* IconTexture = nullptr;
	if (TObjectPtr<UTexture2D>* CachedTexture = IconTextureCache.Find(IconTexturePath))
	{
		IconTexture = CachedTexture->Get();
	}

	if (!IconTexture)
	{
		IconTexture = LoadObject<UTexture2D>(nullptr, *IconTexturePath);
		if (!IconTexture)
		{
			return nullptr;
		}

		IconTextureCache.Add(IconTexturePath, IconTexture);
	}

	FSlateBrush Brush;
	Brush.SetResourceObject(IconTexture);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
	return &IconBrushCache.Add(IconTexturePath, Brush);
}

bool UWUVendorWidget::HasMerchantItems() const
{
	const FWUVendorTable* VendorTable = ResolveActiveVendorTable();
	return VendorTable && VendorTable->Items.Num() > 0;
}

int32 UWUVendorWidget::GetMerchantPageCount() const
{
	const FWUVendorTable* VendorTable = ResolveActiveVendorTable();
	return VendorTable ? FMath::Max(1, FMath::DivideAndRoundUp(VendorTable->Items.Num(), ItemsPerMerchantPage)) : 1;
}

int32 UWUVendorWidget::GetClampedMerchantPageIndex() const
{
	return FMath::Clamp(MerchantPageIndex, 0, GetMerchantPageCount() - 1);
}

FText UWUVendorWidget::GetMerchantPageText() const
{
	return FText::Format(
		LOCTEXT("MerchantPageText", "Page {0} of {1}"),
		FText::AsNumber(GetClampedMerchantPageIndex() + 1),
		FText::AsNumber(GetMerchantPageCount()));
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
	Session->PurchaseSelectedVendorItem(GetResolvedVendorTableId().ToString(), ItemId.ToString());
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleBrowseClicked()
{
	ActiveMode = EVendorPanelMode::Merchant;
	StatusText = FText::GetEmpty();
	RefreshActiveContent();
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleMerchantTabClicked()
{
	ActiveMode = EVendorPanelMode::Merchant;
	RefreshActiveContent();
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleBuybackTabClicked()
{
	ActiveMode = EVendorPanelMode::Buyback;
	RefreshActiveContent();
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandlePreviousPageClicked()
{
	MerchantPageIndex = FMath::Max(0, GetClampedMerchantPageIndex() - 1);
	RefreshActiveContent();
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleNextPageClicked()
{
	MerchantPageIndex = FMath::Min(GetMerchantPageCount() - 1, GetClampedMerchantPageIndex() + 1);
	RefreshActiveContent();
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleRepairClicked(bool bRepairAll)
{
	StatusText = bRepairAll
		? LOCTEXT("RepairAllUnavailable", "Repair all is not wired to item durability yet.")
		: LOCTEXT("RepairUnavailable", "Item repair is not wired to durability yet.");
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleQuestClicked()
{
	StatusText = LOCTEXT("QuestUnavailable", "Quest dialogue is not wired yet.");
	InvalidateLayoutAndVolatility();
	return FReply::Handled();
}

FReply UWUVendorWidget::HandleGoodbyeClicked()
{
	if (AWUPlayerController* PlayerController = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		PlayerController->HideVendor();
	}
	else
	{
		HideVendor();
	}

	return FReply::Handled();
}

void UWUVendorWidget::HandleVendorPurchaseCompleted(const FWUBackendVendorPurchase& Purchase)
{
	if (!bVendorOpen || Purchase.VendorTableId != GetResolvedVendorTableId().ToString())
	{
		return;
	}

	if (Purchase.Inventory.CharacterId.IsEmpty())
	{
		if (AWUCharacter* Character = Cast<AWUCharacter>(GetOwningPlayerPawn()))
		{
			Character->AddInventoryItemById(FName(*Purchase.ItemId));
		}
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
