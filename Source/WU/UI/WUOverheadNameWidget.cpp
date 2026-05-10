// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUOverheadNameWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUOverheadNameWidget"

UWUOverheadNameWidget::UWUOverheadNameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);
}

void UWUOverheadNameWidget::SetNameText(const FText& NewNameText)
{
	if (NameText.EqualTo(NewNameText))
	{
		return;
	}

	NameText = NewNameText;
	InvalidateLayoutAndVolatility();
}

void UWUOverheadNameWidget::SetSubtitleText(const FText& NewSubtitleText)
{
	if (SubtitleText.EqualTo(NewSubtitleText))
	{
		return;
	}

	SubtitleText = NewSubtitleText;
	InvalidateLayoutAndVolatility();
}

void UWUOverheadNameWidget::SetNameColor(FSlateColor NewNameColor)
{
	NameColor = NewNameColor;
	InvalidateLayoutAndVolatility();
}

TSharedRef<SWidget> UWUOverheadNameWidget::RebuildWidget()
{
	return SNew(SBox)
		.MinDesiredWidth(120.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			[
				SNew(STextBlock)
				.Text_UObject(this, &UWUOverheadNameWidget::GetNameText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity_UObject(this, &UWUOverheadNameWidget::GetNameColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.0f, 1.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text_UObject(this, &UWUOverheadNameWidget::GetSubtitleText)
				.Visibility_UObject(this, &UWUOverheadNameWidget::GetSubtitleVisibility)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
				.ColorAndOpacity(SubtitleColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.90f))
			]
		];
}

FText UWUOverheadNameWidget::GetNameText() const
{
	return NameText.IsEmpty() ? LOCTEXT("FallbackName", "Unknown") : NameText;
}

FText UWUOverheadNameWidget::GetSubtitleText() const
{
	return SubtitleText;
}

FSlateColor UWUOverheadNameWidget::GetNameColor() const
{
	return NameColor;
}

EVisibility UWUOverheadNameWidget::GetSubtitleVisibility() const
{
	return SubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible;
}

#undef LOCTEXT_NAMESPACE
