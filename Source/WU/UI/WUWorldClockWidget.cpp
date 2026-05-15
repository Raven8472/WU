// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUWorldClockWidget.h"

#include "Backend/WUClientSessionSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "Styling/CoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"

UWUWorldClockWidget::UWUWorldClockWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Compact_9Slice.T_HUD_Panel_Compact_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

TSharedRef<SWidget> UWUWorldClockWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, WidgetSize, FMargin(0.24f));

	return SNew(SBox)
		.WidthOverride(WidgetSize.X)
		.HeightOverride(WidgetSize.Y)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(12.0f, 6.0f, 12.0f, 6.0f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(NSLOCTEXT("WUWorldClockWidget", "WorldClockLabel", "World Time"))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))
					.ColorAndOpacity(LabelTextColor)
					.Justification(ETextJustify::Center)
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUWorldClockWidget::GetClockText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(ClockTextColor)
					.Justification(ETextJustify::Center)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]
			]
		];
}

FText UWUWorldClockWidget::GetClockText() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (const UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
			{
				return Session->GetEstimatedWorldClockText();
			}
		}
	}

	return FText::FromString(TEXT("--:--"));
}

void UWUWorldClockWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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
