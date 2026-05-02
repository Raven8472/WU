// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUExperienceBarWidget.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "WUCharacter.h"

#define LOCTEXT_NAMESPACE "WUExperienceBarWidget"

UWUExperienceBarWidget::UWUExperienceBarWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Compact_9Slice.T_HUD_Panel_Compact_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> BarFrameAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_BarFrame_Player.T_HUD_BarFrame_Player"));
	if (BarFrameAsset.Succeeded())
	{
		BarFrameTexture = BarFrameAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UTexture2D> ExperienceFillAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Bar_Fill_Health.T_HUD_Bar_Fill_Health"));
	if (ExperienceFillAsset.Succeeded())
	{
		ExperienceFillTexture = ExperienceFillAsset.Object;
	}
}

TSharedRef<SWidget> UWUExperienceBarWidget::RebuildWidget()
{
	const float BarFrameWidth = WidgetSize.X - 20.0f;
	const float FillWidth = WidgetSize.X - 38.0f;

	ConfigureImageBrush(PanelBrush, PanelTexture, WidgetSize, FMargin(0.24f));
	ConfigureImageBrush(BarFrameBrush, BarFrameTexture, FVector2D(BarFrameWidth, 18.0f));
	ConfigureImageBrush(ExperienceFillBrush, ExperienceFillTexture, FVector2D(FillWidth, 8.0f));
	ConfigureColorBrush(ExperienceBackgroundBrush, FLinearColor(0.055f, 0.035f, 0.015f, 0.96f), FVector2D(FillWidth, 8.0f));

	ExperienceBarStyle = FProgressBarStyle()
		.SetBackgroundImage(ExperienceBackgroundBrush)
		.SetFillImage(ExperienceFillBrush);

	return SNew(SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		.Visibility_Lambda([this]()
		{
			return HasCharacter() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
		})
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(10.0f, 3.0f, 10.0f, 3.0f))
			[
				SNew(SBox)
				.WidthOverride(BarFrameWidth)
				.HeightOverride(18.0f)
				.HAlign(HAlign_Center)
				[
					SNew(SOverlay)

					+ SOverlay::Slot()
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(FillWidth)
						.HeightOverride(8.0f)
						[
							SNew(SProgressBar)
							.Style(&ExperienceBarStyle)
							.Percent_UObject(this, &UWUExperienceBarWidget::GetExperiencePercent)
							.FillColorAndOpacity(ExperienceFillTint)
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
						.Text_UObject(this, &UWUExperienceBarWidget::GetExperienceText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 8))
						.ColorAndOpacity(ValueColor)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
					]
				]
			]
		];
}

AWUCharacter* UWUExperienceBarWidget::GetCharacter() const
{
	return Cast<AWUCharacter>(GetOwningPlayerPawn());
}

bool UWUExperienceBarWidget::HasCharacter() const
{
	return GetCharacter() != nullptr;
}

FText UWUExperienceBarWidget::GetExperienceText() const
{
	if (const AWUCharacter* Character = GetCharacter())
	{
		const int32 ExperienceToNextLevel = Character->GetExperienceToNextLevel();
		if (ExperienceToNextLevel <= 0)
		{
			return LOCTEXT("MaxLevelText", "Max Level");
		}

		return FText::Format(
			LOCTEXT("ExperienceValueText", "{0} / {1}"),
			FText::AsNumber(Character->GetCharacterExperience()),
			FText::AsNumber(ExperienceToNextLevel));
	}

	return FText::GetEmpty();
}

TOptional<float> UWUExperienceBarWidget::GetExperiencePercent() const
{
	if (const AWUCharacter* Character = GetCharacter())
	{
		return Character->GetExperiencePercent();
	}

	return 0.0f;
}

void UWUExperienceBarWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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

void UWUExperienceBarWidget::ConfigureColorBrush(FSlateBrush& Brush, const FLinearColor& Color, const FVector2D& ImageSize)
{
	Brush.TintColor = FSlateColor(Color);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Image;
}

#undef LOCTEXT_NAMESPACE
