// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUCharacterSelectWidget.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "WULoginPlayerController.h"
#include "UI/WUCharacterCreatorWidget.h"
#include "Engine/Texture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "ImageUtils.h"
#include "Misc/Paths.h"
#include "Styling/CoreStyle.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUCharacterSelectWidget"

namespace
{
	const FLinearColor CharacterSelectPanelTint(0.015f, 0.012f, 0.01f, 0.78f);
	const FLinearColor CharacterSelectGoldText(0.96f, 0.84f, 0.58f, 1.0f);
}

UWUCharacterSelectWidget::UWUCharacterSelectWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	StatusText = LOCTEXT("SelectReady", "Select a character");
	CharacterCreatorWidgetClass = UWUCharacterCreatorWidget::StaticClass();

	static ConstructorHelpers::FObjectFinder<UTexture2D> LoginBackgroundAsset(TEXT("/Game/UI/Login/Login_Background.Login_Background"));
	if (LoginBackgroundAsset.Succeeded())
	{
		BackgroundTexture = LoginBackgroundAsset.Object;
	}
}

void UWUCharacterSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		Session->OnCharactersLoaded.AddDynamic(this, &UWUCharacterSelectWidget::HandleCharactersLoaded);
		Session->OnCharacterCreated.AddDynamic(this, &UWUCharacterSelectWidget::HandleCharacterCreated);
		Session->OnRequestFailed.AddDynamic(this, &UWUCharacterSelectWidget::HandleRequestFailed);
		RefreshCharacterRows();
	}
}

void UWUCharacterSelectWidget::NativeDestruct()
{
	if (CharacterCreatorWidget)
	{
		CharacterCreatorWidget->OnCreateRequested.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCreatorCreateRequested);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->OnCharactersLoaded.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCharactersLoaded);
			Session->OnCharacterCreated.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCharacterCreated);
			Session->OnRequestFailed.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleRequestFailed);
		}
	}

	Super::NativeDestruct();
}

TSharedRef<SWidget> UWUCharacterSelectWidget::RebuildWidget()
{
	BackgroundBrush.SetResourceObject(ResolveBackgroundTexture());
	BackgroundBrush.DrawAs = ESlateBrushDrawType::Image;
	BackgroundBrush.ImageSize = FVector2D(1920.0f, 1080.0f);

	PanelBrush.DrawAs = ESlateBrushDrawType::Box;
	PanelBrush.TintColor = FSlateColor(CharacterSelectPanelTint);

	PreviewBrush.SetResourceObject(ResolvePreviewTexture());
	PreviewBrush.DrawAs = ESlateBrushDrawType::Image;
	PreviewBrush.ImageSize = FVector2D(360.0f, 540.0f);

	TSharedRef<SVerticalBox> CharacterList = SNew(SVerticalBox);
	CharacterListBox = CharacterList;

	TSharedRef<SWidget> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&BackgroundBrush)
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(90.0f, 0.0f, 0.0f, 0.0f))
		[
			CreateCharacterCreatorPanel()
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(550.0f, 0.0f, 0.0f, 0.0f))
		[
			CreateCharacterPreviewPanel()
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 0.0f, 90.0f, 0.0f))
		[
			SNew(SBox)
			.WidthOverride(500.0f)
			.HeightOverride(620.0f)
			[
				SNew(SBorder)
				.BorderImage(&PanelBrush)
				.Padding(FMargin(24.0f))
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("CharacterSelectTitle", "Character Select"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 30))
						.ColorAndOpacity(CharacterSelectGoldText)
						.ShadowOffset(FVector2D(2.0f, 2.0f))
						.ShadowColorAndOpacity(FLinearColor::Black)
					]

					+ SVerticalBox::Slot()
					.FillHeight(1.0f)
					.Padding(FMargin(0.0f, 22.0f, 0.0f, 16.0f))
					[
						SNew(SScrollBox)
						+ SScrollBox::Slot()
						[
							CharacterList
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
					[
						SNew(SButton)
						.ContentPadding(FMargin(12.0f, 9.0f))
						.OnClicked_UObject(this, &UWUCharacterSelectWidget::HandleEnterGameClicked)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("EnterGame", "Enter Game"))
							.Justification(ETextJustify::Center)
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							SNew(SButton)
							.ContentPadding(FMargin(12.0f, 8.0f))
							.OnClicked_UObject(this, &UWUCharacterSelectWidget::HandleCreateClicked)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("CreateCharacter", "Create New"))
								.Justification(ETextJustify::Center)
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
							]
						]

						+ SHorizontalBox::Slot()
						.AutoWidth()
						.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
						[
							SNew(SButton)
							.ContentPadding(FMargin(12.0f, 8.0f))
							.OnClicked_UObject(this, &UWUCharacterSelectWidget::HandleRefreshClicked)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("Refresh", "Refresh"))
								.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
							]
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 14.0f, 0.0f, 0.0f))
					[
						SNew(STextBlock)
						.Text_UObject(this, &UWUCharacterSelectWidget::GetStatusText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
						.ColorAndOpacity(FLinearColor(0.9f, 0.86f, 0.76f, 0.88f))
						.AutoWrapText(true)
					]
				]
			]
		];

	RefreshCharacterRows();
	return Root;
}

void UWUCharacterSelectWidget::HandleCharactersLoaded(const TArray<FWUBackendCharacterSummary>& LoadedCharacters)
{
	StatusText = FText::Format(LOCTEXT("CharactersLoaded", "{0} characters loaded"), FText::AsNumber(LoadedCharacters.Num()));
	RefreshCharacterRows();
}

void UWUCharacterSelectWidget::HandleCharacterCreated(const FWUBackendCharacterSummary& Character)
{
	StatusText = FText::Format(LOCTEXT("CharacterCreated", "{0} created"), FText::FromString(Character.Name));
	if (CharacterCreatorWidget)
	{
		CharacterCreatorWidget->HideCreator();
	}
}

void UWUCharacterSelectWidget::HandleRequestFailed(const FString& ErrorMessage)
{
	StatusText = FText::FromString(ErrorMessage);
}

void UWUCharacterSelectWidget::SetStatusText(const FText& NewStatusText)
{
	StatusText = NewStatusText;
}

void UWUCharacterSelectWidget::HandleCreatorCreateRequested(const FWUCharacterCreateRequest& Request)
{
	FWUCharacterCreateRequest SanitizedRequest = Request;
	SanitizedRequest.CharacterName = Request.CharacterName.TrimStartAndEnd();

	if (SanitizedRequest.CharacterName.Len() < 3)
	{
		StatusText = LOCTEXT("NameTooShort", "Character name must be at least 3 characters.");
		return;
	}

	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		StatusText = LOCTEXT("CreatingCharacter", "Creating character...");
		Session->CreateCharacter(SanitizedRequest);
	}
}

FReply UWUCharacterSelectWidget::HandleCreateClicked()
{
	if (!CharacterCreatorWidget)
	{
		return FReply::Handled();
	}

	StatusText = LOCTEXT("CreatorOpen", "Choose a name and appearance.");
	CharacterCreatorWidget->ShowCreator();

	return FReply::Handled();
}

FReply UWUCharacterSelectWidget::HandleRefreshClicked()
{
	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		StatusText = LOCTEXT("RefreshingCharacters", "Refreshing characters...");
		Session->ListCharacters();
	}

	return FReply::Handled();
}

FReply UWUCharacterSelectWidget::HandleSelectClicked(FString CharacterId)
{
	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		Session->SelectCharacter(CharacterId);
		StatusText = LOCTEXT("CharacterSelected", "Character selected");
		RefreshCharacterRows();
	}

	return FReply::Handled();
}

FReply UWUCharacterSelectWidget::HandleEnterGameClicked()
{
	const UWUClientSessionSubsystem* Session = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Session->GetSelectedCharacterId().IsEmpty())
	{
		StatusText = LOCTEXT("SelectCharacterFirst", "Select a character before entering the game.");
		return FReply::Handled();
	}

	StatusText = LOCTEXT("EnteringGame", "Entering game...");
	OnEnterGameRequested.Broadcast();
	return FReply::Handled();
}

FText UWUCharacterSelectWidget::GetStatusText() const
{
	return StatusText;
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateCharacterCreatorPanel()
{
	if (!CharacterCreatorWidget && CharacterCreatorWidgetClass && GetOwningPlayer())
	{
		CharacterCreatorWidget = CreateWidget<UWUCharacterCreatorWidget>(GetOwningPlayer(), CharacterCreatorWidgetClass);
		if (CharacterCreatorWidget)
		{
			CharacterCreatorWidget->OnCreateRequested.AddUniqueDynamic(this, &UWUCharacterSelectWidget::HandleCreatorCreateRequested);
		}
	}

	return CharacterCreatorWidget ? CharacterCreatorWidget->TakeWidget() : SNullWidget::NullWidget;
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateCharacterPreviewPanel()
{
	return SNew(SBox)
		.WidthOverride(360.0f)
		.HeightOverride(540.0f)
		.Visibility_Lambda([this]()
		{
			return CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen() && ResolvePreviewTexture()
				? EVisibility::HitTestInvisible
				: EVisibility::Collapsed;
		})
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(8.0f))
			[
				SNew(SImage)
				.Image(&PreviewBrush)
			]
		];
}

void UWUCharacterSelectWidget::RefreshCharacterRows()
{
	if (!CharacterListBox)
	{
		return;
	}

	CharacterListBox->ClearChildren();

	UWUClientSessionSubsystem* Session = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session)
	{
		return;
	}

	const TArray<FWUBackendCharacterSummary>& SessionCharacters = Session->GetCharacters();
	if (SessionCharacters.IsEmpty())
	{
		CharacterListBox->AddSlot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(LOCTEXT("NoCharacters", "No characters yet"))
			.Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
			.ColorAndOpacity(FLinearColor(0.9f, 0.86f, 0.76f, 0.9f))
		];
		return;
	}

	for (const FWUBackendCharacterSummary& Character : SessionCharacters)
	{
		CharacterListBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
		[
			CreateCharacterRow(Character)
		];
	}
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateCharacterRow(const FWUBackendCharacterSummary& Character)
{
	const UWUClientSessionSubsystem* Session = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	const bool bSelected = Session && Session->GetSelectedCharacterId() == Character.CharacterId;
	const FLinearColor RowTint = bSelected
		? FLinearColor(0.42f, 0.31f, 0.12f, 0.84f)
		: FLinearColor(0.05f, 0.04f, 0.035f, 0.72f);

	return SNew(SBorder)
		.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
		.BorderBackgroundColor(RowTint)
		.Padding(FMargin(12.0f, 10.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::Format(
					LOCTEXT("CharacterRow", "{0}  Level {1}  {2}"),
					FText::FromString(Character.Name),
					FText::AsNumber(Character.Level),
					FText::FromString(WUCharacterCreation::RaceToString(Character.Race))))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
				.ColorAndOpacity(CharacterSelectGoldText)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
				.ContentPadding(FMargin(10.0f, 6.0f))
				.OnClicked_UObject(this, &UWUCharacterSelectWidget::HandleSelectClicked, Character.CharacterId)
				[
					SNew(STextBlock)
					.Text(bSelected ? LOCTEXT("Selected", "Selected") : LOCTEXT("Select", "Select"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
				]
			]
		];
}

UTexture* UWUCharacterSelectWidget::ResolvePreviewTexture() const
{
	const AWULoginPlayerController* LoginPC = Cast<AWULoginPlayerController>(GetOwningPlayer());
	return LoginPC ? LoginPC->GetCharacterCreatorPreviewRenderTarget() : nullptr;
}

UTexture2D* UWUCharacterSelectWidget::ResolveBackgroundTexture()
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
