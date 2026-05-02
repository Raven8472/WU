// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUZoneNameWidget.h"
#include "Styling/CoreStyle.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "WUCharacter.h"

UWUZoneNameWidget::UWUZoneNameWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Compact_9Slice.T_HUD_Panel_Compact_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

TSharedRef<SWidget> UWUZoneNameWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, WidgetSize, FMargin(0.24f));

	return SNew(SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		.Visibility_Lambda([this]()
		{
			return HasZoneName() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
		})
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(12.0f, 4.0f, 12.0f, 4.0f))
			[
				SNew(STextBlock)
				.Text_UObject(this, &UWUZoneNameWidget::GetZoneNameText)
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				.ColorAndOpacity(ZoneTextColor)
				.Justification(ETextJustify::Center)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
			]
		];
}

AWUCharacter* UWUZoneNameWidget::GetCharacter() const
{
	return Cast<AWUCharacter>(GetOwningPlayerPawn());
}

bool UWUZoneNameWidget::HasZoneName() const
{
	return !GetZoneNameText().IsEmpty();
}

FText UWUZoneNameWidget::GetZoneNameText() const
{
	if (const AWUCharacter* Character = GetCharacter())
	{
		const FText DisplayName = Character->GetCurrentZoneDisplayName();
		if (!DisplayName.IsEmpty())
		{
			return DisplayName;
		}

		const FName ZoneId = Character->GetCurrentZoneId();
		if (!ZoneId.IsNone())
		{
			return FText::FromName(ZoneId);
		}
	}

	return FText::GetEmpty();
}

void UWUZoneNameWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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
