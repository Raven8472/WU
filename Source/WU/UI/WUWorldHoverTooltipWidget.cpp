// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUWorldHoverTooltipWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUWorldHoverTooltipWidget"

UWUWorldHoverTooltipWidget::UWUWorldHoverTooltipWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);
	SetVisibility(ESlateVisibility::Collapsed);
}

void UWUWorldHoverTooltipWidget::ShowTooltip(const FText& NewTitleText, const FText& NewDetailText, FSlateColor NewTitleColor)
{
	TitleText = NewTitleText;
	DetailText = NewDetailText;
	TitleColor = NewTitleColor;
	bTooltipVisible = true;

	SetVisibility(ESlateVisibility::HitTestInvisible);
	InvalidateLayoutAndVolatility();
}

void UWUWorldHoverTooltipWidget::HideTooltip()
{
	if (!bTooltipVisible)
	{
		return;
	}

	bTooltipVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
	InvalidateLayoutAndVolatility();
}

bool UWUWorldHoverTooltipWidget::IsTooltipVisible() const
{
	return bTooltipVisible;
}

TSharedRef<SWidget> UWUWorldHoverTooltipWidget::RebuildWidget()
{
	return SNew(SBorder)
		.Visibility_UObject(this, &UWUWorldHoverTooltipWidget::GetTooltipVisibility)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.025f, 0.92f))
		.Padding(FMargin(10.0f, 8.0f))
		[
			SNew(SBox)
			.MinDesiredWidth(250.0f)
			.MaxDesiredWidth(310.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUWorldHoverTooltipWidget::GetTitleText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity_UObject(this, &UWUWorldHoverTooltipWidget::GetTitleColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 2.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUWorldHoverTooltipWidget::GetDetailText)
					.Visibility_UObject(this, &UWUWorldHoverTooltipWidget::GetDetailVisibility)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(FSlateColor(FLinearColor(0.88f, 0.92f, 0.86f, 1.0f)))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.95f))
				]
			]
		];
}

FText UWUWorldHoverTooltipWidget::GetTitleText() const
{
	return TitleText.IsEmpty() ? LOCTEXT("FallbackTitle", "Unknown") : TitleText;
}

FText UWUWorldHoverTooltipWidget::GetDetailText() const
{
	return DetailText;
}

FSlateColor UWUWorldHoverTooltipWidget::GetTitleColor() const
{
	return TitleColor;
}

EVisibility UWUWorldHoverTooltipWidget::GetTooltipVisibility() const
{
	return bTooltipVisible ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
}

EVisibility UWUWorldHoverTooltipWidget::GetDetailVisibility() const
{
	return DetailText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

#undef LOCTEXT_NAMESPACE
