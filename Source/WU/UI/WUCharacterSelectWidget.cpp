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
	const FLinearColor CharacterSelectPanelTint(0.015f, 0.012f, 0.01f, 0.62f);
	const FLinearColor CharacterSelectPanelStrongTint(0.015f, 0.012f, 0.01f, 0.82f);
	const FLinearColor CharacterSelectGoldText(0.96f, 0.84f, 0.58f, 1.0f);
	const FLinearColor CharacterSelectMutedText(0.78f, 0.74f, 0.66f, 0.92f);
	const FLinearColor CharacterSelectSelectedTint(0.58f, 0.43f, 0.12f, 0.72f);
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
		Session->OnCharacterDeleted.AddDynamic(this, &UWUCharacterSelectWidget::HandleCharacterDeleted);
		Session->OnRequestFailed.AddDynamic(this, &UWUCharacterSelectWidget::HandleRequestFailed);
		RefreshCharacterRows();
		PreviewSelectedCharacter();
	}
}

void UWUCharacterSelectWidget::NativeDestruct()
{
	if (CharacterCreatorWidget)
	{
		CharacterCreatorWidget->OnCreateRequested.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCreatorCreateRequested);
		CharacterCreatorWidget->OnCreatorClosed.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCreatorClosed);
	}

	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->OnCharactersLoaded.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCharactersLoaded);
			Session->OnCharacterCreated.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCharacterCreated);
			Session->OnCharacterDeleted.RemoveDynamic(this, &UWUCharacterSelectWidget::HandleCharacterDeleted);
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

	UTexture* PreviewTexture = ResolvePreviewTexture();
	PreviewBrush.SetResourceObject(PreviewTexture);
	PreviewBrush.DrawAs = ESlateBrushDrawType::Image;
	PreviewBrush.ImageSize = PreviewTexture
		? FVector2D(PreviewTexture->GetSurfaceWidth(), PreviewTexture->GetSurfaceHeight())
		: FVector2D(1920.0f, 1080.0f);

	TSharedRef<SWidget> Root = SNew(SOverlay)
		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&BackgroundBrush)
		]

		+ SOverlay::Slot()
		[
			SNew(SImage)
			.Image(&PreviewBrush)
			.Visibility_Lambda([this]()
			{
				return ResolvePreviewTexture() ? EVisibility::HitTestInvisible : EVisibility::Collapsed;
			})
		]

		+ SOverlay::Slot()
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.22f))
			.Padding(FMargin(0.0f))
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Top)
		.Padding(FMargin(54.0f, 36.0f, 0.0f, 0.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.Text(LOCTEXT("GameTitle", "WU"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 54))
				.ColorAndOpacity(CharacterSelectGoldText)
				.ShadowOffset(FVector2D(2.0f, 2.0f))
				.ShadowColorAndOpacity(FLinearColor::Black)
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(2.0f, -4.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CharacterSelectSubtitle", "Character Selection"))
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
				.ColorAndOpacity(CharacterSelectMutedText)
				.ShadowOffset(FVector2D(1.0f, 1.0f))
				.ShadowColorAndOpacity(FLinearColor::Black)
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.Padding(FMargin(42.0f, 78.0f, 0.0f, 0.0f))
		[
			SNew(SBox)
			.Visibility_UObject(this, &UWUCharacterSelectWidget::GetCreatorModeVisibility)
			[
				CreateCharacterCreatorPanel()
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 78.0f, 42.0f, 0.0f))
		[
			SNew(SBox)
			.Visibility_UObject(this, &UWUCharacterSelectWidget::GetCreatorModeVisibility)
			[
				CreateCreationContextPanel()
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Center)
		.Padding(FMargin(0.0f, 70.0f, 42.0f, 0.0f))
		[
			SNew(SBox)
			.Visibility_UObject(this, &UWUCharacterSelectWidget::GetSelectPanelVisibility)
			[
				CreateCharacterListPanel()
			]
		]

		+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Bottom)
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 34.0f))
		[
			CreateBottomCommandBar()
		];

	RefreshCharacterRows();
	PreviewSelectedCharacter();
	return Root;
}

void UWUCharacterSelectWidget::HandleCharactersLoaded(const TArray<FWUBackendCharacterSummary>& LoadedCharacters)
{
	StatusText = FText::Format(LOCTEXT("CharactersLoaded", "{0} characters loaded"), FText::AsNumber(LoadedCharacters.Num()));
	RefreshCharacterRows();
	PreviewSelectedCharacter();
}

void UWUCharacterSelectWidget::HandleCharacterCreated(const FWUBackendCharacterSummary& Character)
{
	StatusText = FText::Format(LOCTEXT("CharacterCreated", "{0} created"), FText::FromString(Character.Name));
	if (CharacterCreatorWidget)
	{
		CharacterCreatorWidget->HideCreator();
	}
}

void UWUCharacterSelectWidget::HandleCharacterDeleted(const FString& CharacterId)
{
	StatusText = LOCTEXT("CharacterDeleted", "Character deleted");
	RefreshCharacterRows();
	PreviewSelectedCharacter();
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
	SanitizedRequest.PathId = SanitizedRequest.PathId.TrimStartAndEnd();
	SanitizedRequest.SkinPresetIndex = FMath::Clamp(SanitizedRequest.SkinPresetIndex, 0, 4);
	SanitizedRequest.HeadPresetIndex = SanitizedRequest.SkinPresetIndex;

	if (SanitizedRequest.PathId.IsEmpty())
	{
		SanitizedRequest.PathId = TEXT("Auror");
	}

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

void UWUCharacterSelectWidget::HandleCreatorClosed()
{
	PreviewSelectedCharacter();
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
		PreviewSelectedCharacter();
	}

	return FReply::Handled();
}

FReply UWUCharacterSelectWidget::HandleDeleteClicked(FString CharacterId, FString CharacterName)
{
	if (UWUClientSessionSubsystem* Session = GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>())
	{
		StatusText = FText::Format(LOCTEXT("DeletingCharacter", "Deleting {0}..."), FText::FromString(CharacterName));
		Session->DeleteCharacter(CharacterId);
	}

	return FReply::Handled();
}

FReply UWUCharacterSelectWidget::HandleDeleteSelectedClicked()
{
	const FWUBackendCharacterSummary* SelectedCharacter = GetSelectedCharacter();
	if (!SelectedCharacter)
	{
		StatusText = LOCTEXT("NoSelectedCharacterToDelete", "Select a character before deleting.");
		return FReply::Handled();
	}

	return HandleDeleteClicked(SelectedCharacter->CharacterId, SelectedCharacter->Name);
}

FReply UWUCharacterSelectWidget::HandleChangeRealmClicked()
{
	StatusText = LOCTEXT("RealmSelectUnavailable", "Only the local development realm is available.");
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

FText UWUCharacterSelectWidget::GetSelectedRealmText() const
{
	const UWUClientSessionSubsystem* Session = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session)
	{
		return LOCTEXT("UnknownRealm", "Unknown Realm");
	}

	const FString& SelectedRealmId = Session->GetSelectedRealmId();
	for (const FWUBackendRealmSummary& Realm : Session->GetRealms())
	{
		if (Realm.RealmId == SelectedRealmId)
		{
			return FText::FromString(Realm.DisplayName);
		}
	}

	return SelectedRealmId.IsEmpty() ? LOCTEXT("UnknownRealmFallback", "Unknown Realm") : FText::FromString(SelectedRealmId);
}

FText UWUCharacterSelectWidget::GetSelectedCharacterNameText() const
{
	const FWUBackendCharacterSummary* SelectedCharacter = GetSelectedCharacter();
	return SelectedCharacter ? FText::FromString(SelectedCharacter->Name) : LOCTEXT("NoSelectedCharacterName", "No Character Selected");
}

FText UWUCharacterSelectWidget::GetSelectedCharacterDetailText() const
{
	const FWUBackendCharacterSummary* SelectedCharacter = GetSelectedCharacter();
	if (!SelectedCharacter)
	{
		return LOCTEXT("NoSelectedCharacterDetail", "Create or select a character.");
	}

	return FText::Format(
		LOCTEXT("SelectedCharacterDetail", "Level {0} {1}"),
		FText::AsNumber(SelectedCharacter->Level),
		GetRaceDisplayText(SelectedCharacter->Race));
}

FText UWUCharacterSelectWidget::GetSelectedCharacterLocationText() const
{
	const FWUBackendCharacterSummary* SelectedCharacter = GetSelectedCharacter();
	if (!SelectedCharacter)
	{
		return FText::GetEmpty();
	}

	return FText::Format(
		LOCTEXT("SelectedCharacterLocation", "Last position {0}, {1}, {2}"),
		FText::AsNumber(FMath::RoundToInt(SelectedCharacter->Location.X)),
		FText::AsNumber(FMath::RoundToInt(SelectedCharacter->Location.Y)),
		FText::AsNumber(FMath::RoundToInt(SelectedCharacter->Location.Z)));
}

EVisibility UWUCharacterSelectWidget::GetSelectPanelVisibility() const
{
	return CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen()
		? EVisibility::Collapsed
		: EVisibility::SelfHitTestInvisible;
}

EVisibility UWUCharacterSelectWidget::GetCreatorModeVisibility() const
{
	return CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen()
		? EVisibility::SelfHitTestInvisible
		: EVisibility::Collapsed;
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateCharacterCreatorPanel()
{
	if (!CharacterCreatorWidget && CharacterCreatorWidgetClass && GetOwningPlayer())
	{
		CharacterCreatorWidget = CreateWidget<UWUCharacterCreatorWidget>(GetOwningPlayer(), CharacterCreatorWidgetClass);
		if (CharacterCreatorWidget)
		{
			CharacterCreatorWidget->OnCreateRequested.AddUniqueDynamic(this, &UWUCharacterSelectWidget::HandleCreatorCreateRequested);
			CharacterCreatorWidget->OnCreatorClosed.AddUniqueDynamic(this, &UWUCharacterSelectWidget::HandleCreatorClosed);
		}
	}

	return CharacterCreatorWidget ? CharacterCreatorWidget->TakeWidget() : SNullWidget::NullWidget;
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateCharacterListPanel()
{
	return SNew(SBox)
		.WidthOverride(430.0f)
		.HeightOverride(720.0f)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.BorderBackgroundColor(CharacterSelectPanelTint)
			.Padding(FMargin(18.0f, 16.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_UObject(this, &UWUCharacterSelectWidget::GetSelectedRealmText)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.Justification(ETextJustify::Center)
					.ColorAndOpacity(CharacterSelectGoldText)
					.ShadowOffset(FVector2D(2.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 7.0f, 0.0f, 14.0f))
				[
					CreateActionButton(LOCTEXT("ChangeServer", "Change Server"), &UWUCharacterSelectWidget::HandleChangeRealmClicked)
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNew(SScrollBox)
					+ SScrollBox::Slot()
					[
						SAssignNew(CharacterListBox, SVerticalBox)
					]
				]
			]
		];
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateCreationContextPanel()
{
	return SNew(SBox)
		.WidthOverride(390.0f)
		.HeightOverride(520.0f)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.BorderBackgroundColor(CharacterSelectPanelTint)
			.Padding(FMargin(18.0f, 16.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CreationIdentityTitle", "Blood Status"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
					.ColorAndOpacity(CharacterSelectGoldText)
					.ShadowOffset(FVector2D(2.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 12.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return CharacterCreatorWidget
							? GetRaceDisplayText(CharacterCreatorWidget->GetCurrentRequest().Race)
							: LOCTEXT("FallbackBloodStatus", "Half-blood");
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
					.ColorAndOpacity(FLinearColor(1.0f, 0.96f, 0.9f, 1.0f))
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 18.0f))
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return CharacterCreatorWidget
							? GetRaceDescriptionText(CharacterCreatorWidget->GetCurrentRequest().Race)
							: FText::GetEmpty();
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(CharacterSelectMutedText)
					.AutoWrapText(true)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return CharacterCreatorWidget
							? GetPathDisplayText(CharacterCreatorWidget->GetSelectedPathId())
							: LOCTEXT("CreationPathTitle", "Path");
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 22))
					.ColorAndOpacity(CharacterSelectGoldText)
					.ShadowOffset(FVector2D(2.0f, 2.0f))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text_Lambda([this]()
					{
						return CharacterCreatorWidget
							? GetPathDescriptionText(CharacterCreatorWidget->GetSelectedPathId())
							: LOCTEXT("CreationPathDescription", "Your first path frames early training, social identity, and the sort of trouble your character is likely to find first.");
					})
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
					.ColorAndOpacity(CharacterSelectMutedText)
					.AutoWrapText(true)
				]
			]
		];
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateBottomCommandBar()
{
	return SNew(SBox)
		.WidthOverride(560.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility_UObject(this, &UWUCharacterSelectWidget::GetSelectPanelVisibility)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					[
						SNew(STextBlock)
						.Text_UObject(this, &UWUCharacterSelectWidget::GetSelectedCharacterNameText)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
						.ColorAndOpacity(CharacterSelectGoldText)
						.ShadowOffset(FVector2D(2.0f, 2.0f))
						.ShadowColorAndOpacity(FLinearColor::Black)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.HAlign(HAlign_Center)
					.Padding(FMargin(0.0f, 0.0f, 0.0f, 8.0f))
					[
						SNew(STextBlock)
						.Text_UObject(this, &UWUCharacterSelectWidget::GetSelectedCharacterDetailText)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
						.ColorAndOpacity(CharacterSelectMutedText)
						.ShadowOffset(FVector2D(1.0f, 1.0f))
						.ShadowColorAndOpacity(FLinearColor::Black)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							CreateActionButton(LOCTEXT("EnterWorld", "Enter World"), &UWUCharacterSelectWidget::HandleEnterGameClicked)
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
						[
							CreateActionButton(LOCTEXT("CreateNew", "Create New"), &UWUCharacterSelectWidget::HandleCreateClicked)
						]
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
					[
						SNew(SHorizontalBox)

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						[
							CreateActionButton(LOCTEXT("DeleteSelected", "Delete"), &UWUCharacterSelectWidget::HandleDeleteSelectedClicked)
						]

						+ SHorizontalBox::Slot()
						.FillWidth(1.0f)
						.Padding(FMargin(8.0f, 0.0f, 0.0f, 0.0f))
						[
							CreateActionButton(LOCTEXT("RefreshCharacters", "Refresh"), &UWUCharacterSelectWidget::HandleRefreshClicked)
						]
					]
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.HAlign(HAlign_Center)
			.Padding(FMargin(0.0f, 10.0f, 0.0f, 0.0f))
			[
				SNew(STextBlock)
				.Text_UObject(this, &UWUCharacterSelectWidget::GetStatusText)
				.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
				.ColorAndOpacity(FLinearColor(0.9f, 0.86f, 0.76f, 0.88f))
				.AutoWrapText(true)
			]
		];
}

TSharedRef<SWidget> UWUCharacterSelectWidget::CreateActionButton(const FText& Text, FReply(UWUCharacterSelectWidget::*Handler)())
{
	return SNew(SButton)
		.ContentPadding(FMargin(14.0f, 8.0f))
		.OnClicked_UObject(this, Handler)
		[
			SNew(STextBlock)
			.Text(Text)
			.Justification(ETextJustify::Center)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 14))
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
		bHasSelectedCharacterPreview = false;
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
		? CharacterSelectSelectedTint
		: FLinearColor(0.02f, 0.018f, 0.015f, 0.52f);

	return SNew(SButton)
		.ButtonColorAndOpacity(RowTint)
		.ContentPadding(FMargin(0.0f))
		.OnClicked_UObject(this, &UWUCharacterSelectWidget::HandleSelectClicked, Character.CharacterId)
		[
			SNew(SBorder)
			.BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
			.BorderBackgroundColor(RowTint)
			.Padding(FMargin(12.0f, 9.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(FText::FromString(Character.Name))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
					.ColorAndOpacity(CharacterSelectGoldText)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor::Black)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 2.0f, 0.0f, 0.0f))
				[
					SNew(STextBlock)
					.Text(FText::Format(
						LOCTEXT("CharacterRowDetail", "Level {0} {1}"),
						FText::AsNumber(Character.Level),
						GetRaceDisplayText(Character.Race)))
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					.ColorAndOpacity(CharacterSelectMutedText)
				]
			]
		];
}

void UWUCharacterSelectWidget::PreviewSelectedCharacter()
{
	if (CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen())
	{
		return;
	}

	const UWUClientSessionSubsystem* Session = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Session->GetSelectedCharacterId().IsEmpty())
	{
		bHasSelectedCharacterPreview = false;
		InvalidateLayoutAndVolatility();
		return;
	}

	for (const FWUBackendCharacterSummary& Character : Session->GetCharacters())
	{
		if (Character.CharacterId == Session->GetSelectedCharacterId())
		{
			PreviewCharacter(Character);
			return;
		}
	}

	bHasSelectedCharacterPreview = false;
	InvalidateLayoutAndVolatility();
}

void UWUCharacterSelectWidget::PreviewCharacter(const FWUBackendCharacterSummary& Character)
{
	AWULoginPlayerController* LoginPC = Cast<AWULoginPlayerController>(GetOwningPlayer());
	if (!LoginPC)
	{
		bHasSelectedCharacterPreview = false;
		InvalidateLayoutAndVolatility();
		return;
	}

	FWUCharacterCreateRequest PreviewRequest;
	PreviewRequest.CharacterName = Character.Name;
	PreviewRequest.Race = Character.Race;
	PreviewRequest.Sex = Character.Sex;
	PreviewRequest.SkinPresetIndex = FMath::Clamp(Character.Appearance.SkinPresetIndex, 0, 4);
	PreviewRequest.HeadPresetIndex = FMath::Clamp(Character.Appearance.HeadPresetIndex, 0, 4);
	PreviewRequest.HairStyleIndex = Character.Appearance.HairStyleIndex;
	PreviewRequest.HairColorIndex = Character.Appearance.HairColorIndex;
	PreviewRequest.EyeColorIndex = Character.Appearance.EyeColorIndex;
	PreviewRequest.BrowStyleIndex = Character.Appearance.BrowStyleIndex;
	PreviewRequest.BeardStyleIndex = Character.Appearance.BeardStyleIndex;

	LoginPC->PreviewCharacterCreateRequest(PreviewRequest);
	bHasSelectedCharacterPreview = true;
	InvalidateLayoutAndVolatility();
}

bool UWUCharacterSelectWidget::ShouldShowCharacterPreview() const
{
	const bool bCreatorPreviewOpen = CharacterCreatorWidget && CharacterCreatorWidget->IsCreatorOpen();
	return ResolvePreviewTexture() && (bCreatorPreviewOpen || bHasSelectedCharacterPreview);
}

const FWUBackendCharacterSummary* UWUCharacterSelectWidget::GetSelectedCharacter() const
{
	const UWUClientSessionSubsystem* Session = GetGameInstance() ? GetGameInstance()->GetSubsystem<UWUClientSessionSubsystem>() : nullptr;
	if (!Session || Session->GetSelectedCharacterId().IsEmpty())
	{
		return nullptr;
	}

	for (const FWUBackendCharacterSummary& Character : Session->GetCharacters())
	{
		if (Character.CharacterId == Session->GetSelectedCharacterId())
		{
			return &Character;
		}
	}

	return nullptr;
}

UTexture* UWUCharacterSelectWidget::ResolvePreviewTexture() const
{
	const AWULoginPlayerController* LoginPC = Cast<AWULoginPlayerController>(GetOwningPlayer());
	return LoginPC ? LoginPC->GetCharacterCreatorPreviewRenderTarget() : nullptr;
}

FText UWUCharacterSelectWidget::GetRaceDisplayText(EWUCharacterRace Race) const
{
	switch (Race)
	{
	case EWUCharacterRace::Pureblood:
		return LOCTEXT("RaceDisplayPureblood", "Pureblood");
	case EWUCharacterRace::Mudblood:
		return LOCTEXT("RaceDisplayMuggleBorn", "Muggle-born");
	case EWUCharacterRace::Halfblood:
	default:
		return LOCTEXT("RaceDisplayHalfblood", "Half-blood");
	}
}

FText UWUCharacterSelectWidget::GetRaceDescriptionText(EWUCharacterRace Race) const
{
	switch (Race)
	{
	case EWUCharacterRace::Pureblood:
		return LOCTEXT("RaceDescriptionPureblood", "Born into long-standing magical families and raised with deep knowledge of wizarding traditions, heritage, pride, and expectation.");
	case EWUCharacterRace::Mudblood:
		return LOCTEXT("RaceDescriptionMuggleBorn", "Born to a non-magical family and entering the wizarding world with curiosity, determination, resilience, learning, and discovery.");
	case EWUCharacterRace::Halfblood:
	default:
		return LOCTEXT("RaceDescriptionHalfblood", "Bridging magical and non-magical ancestry, with a broader perspective and the adaptability to move between worlds.");
	}
}

FText UWUCharacterSelectWidget::GetPathDisplayText(FName PathId) const
{
	if (PathId == FName(TEXT("Magizoologist")))
	{
		return LOCTEXT("SelectPathMagizoologist", "Path of the Magizoologist");
	}

	if (PathId == FName(TEXT("Philosopher")))
	{
		return LOCTEXT("SelectPathPhilosopher", "Path of the Philosopher");
	}

	if (PathId == FName(TEXT("CurseBreaker")))
	{
		return LOCTEXT("SelectPathCurseBreaker", "Path of the Curse Breaker");
	}

	if (PathId == FName(TEXT("MediwitchMediwizard")))
	{
		return LOCTEXT("SelectPathMediwitch", "Path of the Mediwitch / Mediwizard");
	}

	if (PathId == FName(TEXT("BlackMarket")))
	{
		return LOCTEXT("SelectPathBlackMarket", "Path of the Black Market");
	}

	if (PathId == FName(TEXT("DarkArts")))
	{
		return LOCTEXT("SelectPathDarkArts", "Path of the Dark Arts");
	}

	if (PathId == FName(TEXT("Seer")))
	{
		return LOCTEXT("SelectPathSeer", "Path of the Seer");
	}

	return LOCTEXT("SelectPathAuror", "Path of the Auror");
}

FText UWUCharacterSelectWidget::GetPathDescriptionText(FName PathId) const
{
	if (PathId == FName(TEXT("Magizoologist")))
	{
		return LOCTEXT("SelectPathMagizoologistDescription", "Caretakers and trackers of magical creatures, drawn beyond safe civilization to understand, rescue, and protect magical beasts.");
	}

	if (PathId == FName(TEXT("Philosopher")))
	{
		return LOCTEXT("SelectPathPhilosopherDescription", "Masters of magical theory, potions, alchemy, and experimentation who search for the hidden rules beneath magic.");
	}

	if (PathId == FName(TEXT("CurseBreaker")))
	{
		return LOCTEXT("SelectPathCurseBreakerDescription", "Explorers of ruins, tombs, forbidden places, ancient traps, and artifacts that demand both wit and nerve.");
	}

	if (PathId == FName(TEXT("MediwitchMediwizard")))
	{
		return LOCTEXT("SelectPathMediwitchDescription", "Healers and restorers who stand between magical injuries, curses, and the fragile line between life and death.");
	}

	if (PathId == FName(TEXT("BlackMarket")))
	{
		return LOCTEXT("SelectPathBlackMarketDescription", "Dealers in questionable goods, dubious artifacts, risky opportunities, and decisions where trust is always in short supply.");
	}

	if (PathId == FName(TEXT("DarkArts")))
	{
		return LOCTEXT("SelectPathDarkArtsDescription", "Students of forbidden and dangerous magic, drawn toward power, secrecy, temptation, and consequence.");
	}

	if (PathId == FName(TEXT("Seer")))
	{
		return LOCTEXT("SelectPathSeerDescription", "Interpreters of fate, prophecy, divination, and hidden truths that are rarely clear until it is too late.");
	}

	return LOCTEXT("SelectPathAurorDescription", "Defenders trained to confront dangerous magic and pursue justice where threats hide in plain sight.");
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
