// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WULoginScreenWidget.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WULoginScreenWidget"

namespace
{
	const FLinearColor PanelTint(0.015f, 0.012f, 0.01f, 0.78f);
	const FLinearColor GoldText(0.96f, 0.84f, 0.58f, 1.0f);
}

UWULoginScreenWidget::UWULoginScreenWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	StatusText = LOCTEXT("LoginReady", "Backend ready");
}

void UWULoginScreenWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		Session->OnLoginSucceeded.AddDynamic(this, &UWULoginScreenWidget::HandleLoginSucceeded);
		Session->OnCharactersLoaded.AddDynamic(this, &UWULoginScreenWidget::HandleCharactersLoaded);
		Session->OnRequestFailed.AddDynamic(this, &UWULoginScreenWidget::HandleRequestFailed);
	}
}

void UWULoginScreenWidget::NativeDestruct()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->OnLoginSucceeded.RemoveDynamic(this, &UWULoginScreenWidget::HandleLoginSucceeded);
			Session->OnCharactersLoaded.RemoveDynamic(this, &UWULoginScreenWidget::HandleCharactersLoaded);
			Session->OnRequestFailed.RemoveDynamic(this, &UWULoginScreenWidget::HandleRequestFailed);
		}
	}

	Super::NativeDestruct();
}

TSharedRef<SWidget> UWULoginScreenWidget::RebuildWidget()
{
	BackgroundBrush.SetResourceObject(ResolveBackgroundTexture());
	BackgroundBrush.DrawAs = ESlateBrushDrawType::Image;
	BackgroundBrush.ImageSize = FVector2D(1920.0f, 1080.0f);

	PanelBrush.DrawAs = ESlateBrushDrawType::Box;
	PanelBrush.TintColor = FSlateColor(PanelTint);

	return SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&BackgroundBrush)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 110.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(420.0f)
			[
				SNew(SBorder)
				.BorderImage(&PanelBrush)
				.Padding(FMargin(28.0f, 24.0f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("GameTitle", "WU"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 44))
						.ColorAndOpacity(GoldText)
						.ShadowOffset(FVector2D(2.0f, 2.0f))
						.ShadowColorAndOpacity(FLinearColor::Black)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 4.0f, 0.0f, 26.0f))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("LoginSubtitle", "Persistence prototype"))
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
						.ColorAndOpacity(FLinearColor(0.92f, 0.9f, 0.84f, 0.88f))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SButton)
						.ContentPadding(FMargin(18.0f, 11.0f))
						.OnClicked_UObject(this, &UWULoginScreenWidget::HandleDevLoginClicked)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("DevLogin", "Dev Login"))
							.Justification(ETextJustify::Center)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text_UObject(this, &UWULoginScreenWidget::GetStatusText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
						.ColorAndOpacity(FLinearColor(0.9f, 0.86f, 0.76f, 0.88f))
						.AutoWrapText(true)
					]
				]
			]
		];
}

void UWULoginScreenWidget::HandleLoginSucceeded()
{
	StatusText = LOCTEXT("LoadingCharacters", "Loading characters...");
}

void UWULoginScreenWidget::HandleCharactersLoaded(const TArray<FWUBackendCharacterSummary>& Characters)
{
	StatusText = FText::Format(LOCTEXT("CharactersLoaded", "{0} characters loaded"), FText::AsNumber(Characters.Num()));
	OnLoginFlowReady.Broadcast();
}

void UWULoginScreenWidget::HandleRequestFailed(const FString& ErrorMessage)
{
	StatusText = FText::FromString(ErrorMessage);
}

FReply UWULoginScreenWidget::HandleDevLoginClicked()
{
	StatusText = LOCTEXT("SigningIn", "Signing in...");

	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		Session->DevLogin();
	}
	else
	{
		StatusText = LOCTEXT("NoSessionSubsystem", "Client session subsystem is unavailable.");
	}

	return FReply::Handled();
}

FText UWULoginScreenWidget::GetStatusText() const
{
	return StatusText;
}

UTexture2D* UWULoginScreenWidget::ResolveBackgroundTexture()
{
	if (BackgroundTexture)
	{
		return BackgroundTexture;
	}

	if (!LoadedBackgroundTexture)
	{
		const FString FullPath = FPaths::ProjectContentDir() / BackgroundSourcePath;
		LoadedBackgroundTexture = FImageUtils::ImportFileAsTexture2D(FullPath);
	}

	return LoadedBackgroundTexture;
}

#undef LOCTEXT_NAMESPACE
