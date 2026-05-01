// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUTargetFrameWidget.h"
#include "WUCharacter.h"
#include "WUPlayerController.h"
#include "Components/Widget.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUTargetFrameWidget"

UWUTargetFrameWidget::UWUTargetFrameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PortraitFrameAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_PortraitFrame_Target.T_HUD_PortraitFrame_Target"));
	if (PortraitFrameAsset.Succeeded())
	{
		PortraitFrameTexture = PortraitFrameAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> BarFrameAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_BarFrame_Target.T_HUD_BarFrame_Target"));
	if (BarFrameAsset.Succeeded())
	{
		BarFrameTexture = BarFrameAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> HealthFillAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Bar_Fill_Health.T_HUD_Bar_Fill_Health"));
	if (HealthFillAsset.Succeeded())
	{
		HealthFillTexture = HealthFillAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> MagicFillAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Bar_Fill_Mana.T_HUD_Bar_Fill_Mana"));
	if (MagicFillAsset.Succeeded())
	{
		MagicFillTexture = MagicFillAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Compact_9Slice.T_HUD_Panel_Compact_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

TSharedRef<SWidget> UWUTargetFrameWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, FrameSize, FMargin(0.24f));
	ConfigureImageBrush(PortraitFrameBrush, PortraitFrameTexture, FVector2D(58.0f, 58.0f));
	ConfigureImageBrush(BarFrameBrush, BarFrameTexture, FVector2D(180.0f, 22.0f));
	ConfigureImageBrush(HealthFillBrush, HealthFillTexture, FVector2D(165.0f, 9.0f));
	ConfigureImageBrush(MagicFillBrush, MagicFillTexture, FVector2D(165.0f, 9.0f));
	ConfigureColorBrush(PortraitFallbackBrush, PortraitBackgroundTint, FVector2D(44.0f, 44.0f));
	ConfigureColorBrush(HealthBackgroundBrush, FLinearColor(0.045f, 0.018f, 0.015f, 0.96f), FVector2D(165.0f, 9.0f));
	ConfigureColorBrush(MagicBackgroundBrush, FLinearColor(0.012f, 0.022f, 0.055f, 0.96f), FVector2D(165.0f, 9.0f));

	HealthBarStyle = FProgressBarStyle()
		.SetBackgroundImage(HealthBackgroundBrush)
		.SetFillImage(HealthFillBrush);

	MagicBarStyle = FProgressBarStyle()
		.SetBackgroundImage(MagicBackgroundBrush)
		.SetFillImage(MagicFillBrush);

	return SNew(SBox)
		.WidthOverride(FrameSize.X)
		.HeightOverride(FrameSize.Y)
		.Visibility_Lambda([this]()
		{
			return HasTarget() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
		})
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
			[
				SNew(SImage)
				.Image(&PanelBrush)
				.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.9f))
			]

			+ SOverlay::Slot()
			.Padding(FMargin(9.0f, 5.0f, 9.0f, 5.0f))
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(FMargin(0.0f, 0.0f, 6.0f, 0.0f))
				[
					SNew(SBox)
					.WidthOverride(58.0f)
					.HeightOverride(58.0f)
					[
						SNew(SOverlay)

						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(44.0f)
							.HeightOverride(44.0f)
							[
								SNew(SImage)
								.Image(&PortraitFallbackBrush)
							]
						]

						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(41.0f)
							.HeightOverride(41.0f)
							.Visibility_Lambda([this]()
							{
								const AWUCharacter* Target = GetTargetCharacter();
								return Target && Target->GetPortraitTexture() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
							})
							[
								SNew(SImage)
								.Image_Lambda([this]()
								{
									return GetPortraitImageBrush();
								})
							]
						]

						+ SOverlay::Slot()
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						[
							SNew(STextBlock)
							.Text_Lambda([this]()
							{
								return GetFallbackPortraitText();
							})
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
							.ColorAndOpacity(LabelColor)
							.Visibility_Lambda([this]()
							{
								const AWUCharacter* Target = GetTargetCharacter();
								return Target && !Target->GetPortraitTexture() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
							})
						]

						+ SOverlay::Slot()
						[
							SNew(SImage)
							.Image(&PortraitFrameBrush)
						]
					]
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(1.0f, 0.0f, 0.0f, 3.0f))
					[
						SNew(STextBlock)
						.Text_Lambda([this]()
						{
							return GetTargetNameText();
						})
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
						.ColorAndOpacity(LabelColor)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f))
						.OverflowPolicy(ETextOverflowPolicy::Ellipsis)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.WidthOverride(180.0f)
						.HeightOverride(22.0f)
						[
							SNew(SOverlay)

							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(165.0f)
								.HeightOverride(9.0f)
								[
									SNew(SProgressBar)
									.Style(&HealthBarStyle)
									.Percent_Lambda([this]()
									{
										return GetHealthPercent();
									})
									.FillColorAndOpacity(HealthFillTint)
								]
							]

							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image(&BarFrameBrush)
							]

							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
								{
									return GetHealthText();
								})
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
								.ColorAndOpacity(ValueColor)
								.ShadowOffset(FVector2D(1.0f, 1.0f))
								.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
							]
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 2.0f, 0.0f, 0.0f))
					[
						SNew(SBox)
						.WidthOverride(180.0f)
						.HeightOverride(22.0f)
						[
							SNew(SOverlay)

							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(SBox)
								.WidthOverride(165.0f)
								.HeightOverride(9.0f)
								[
									SNew(SProgressBar)
									.Style(&MagicBarStyle)
									.Percent_Lambda([this]()
									{
										return GetMagicPercent();
									})
									.FillColorAndOpacity(MagicFillTint)
								]
							]

							+ SOverlay::Slot()
							[
								SNew(SImage)
								.Image(&BarFrameBrush)
							]

							+ SOverlay::Slot()
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							[
								SNew(STextBlock)
								.Text_Lambda([this]()
								{
									return GetMagicText();
								})
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
								.ColorAndOpacity(ValueColor)
								.ShadowOffset(FVector2D(1.0f, 1.0f))
								.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
							]
						]
					]
				]
			]
		];
}

AWUCharacter* UWUTargetFrameWidget::GetTargetCharacter() const
{
	if (const AWUPlayerController* PC = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		return PC->GetCurrentTarget();
	}

	return nullptr;
}

bool UWUTargetFrameWidget::HasTarget() const
{
	return GetTargetCharacter() != nullptr;
}

FText UWUTargetFrameWidget::GetTargetNameText() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return Target->GetDisplayName();
	}

	return FText::GetEmpty();
}

FText UWUTargetFrameWidget::GetHealthText() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return FText::Format(
			LOCTEXT("TargetHealthFormat", "{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(Target->GetCurrentHealth())),
			FText::AsNumber(FMath::RoundToInt(Target->GetMaxHealth()))
		);
	}

	return FText::GetEmpty();
}

FText UWUTargetFrameWidget::GetMagicText() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return FText::Format(
			LOCTEXT("TargetMagicFormat", "{0} / {1}"),
			FText::AsNumber(FMath::RoundToInt(Target->GetCurrentMagic())),
			FText::AsNumber(FMath::RoundToInt(Target->GetMaxMagic()))
		);
	}

	return FText::GetEmpty();
}

FText UWUTargetFrameWidget::GetFallbackPortraitText() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		const FString Name = Target->GetDisplayName().ToString();
		if (!Name.IsEmpty())
		{
			return FText::FromString(Name.Left(1).ToUpper());
		}
	}

	return FText::GetEmpty();
}

TOptional<float> UWUTargetFrameWidget::GetHealthPercent() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return Target->GetHealthPercent();
	}

	return 0.0f;
}

TOptional<float> UWUTargetFrameWidget::GetMagicPercent() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		return Target->GetMagicPercent();
	}

	return 0.0f;
}

const FSlateBrush* UWUTargetFrameWidget::GetPortraitImageBrush() const
{
	if (const AWUCharacter* Target = GetTargetCharacter())
	{
		if (UTexture2D* PortraitTexture = Target->GetPortraitTexture())
		{
			PortraitImageBrush.SetResourceObject(PortraitTexture);
			PortraitImageBrush.ImageSize = FVector2D(41.0f, 41.0f);
			PortraitImageBrush.DrawAs = ESlateBrushDrawType::Image;
			return &PortraitImageBrush;
		}
	}

	return nullptr;
}

void UWUTargetFrameWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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

void UWUTargetFrameWidget::ConfigureColorBrush(FSlateBrush& Brush, const FLinearColor& Color, const FVector2D& ImageSize)
{
	Brush.TintColor = FSlateColor(Color);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
}

#undef LOCTEXT_NAMESPACE
