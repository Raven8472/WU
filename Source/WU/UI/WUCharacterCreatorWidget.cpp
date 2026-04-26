// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUCharacterCreatorWidget.h"
#include "WUPlayerController.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUCharacterCreatorWidget"

UWUCharacterCreatorWidget::UWUCharacterCreatorWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_Panel_Large_9Slice.T_HUD_Panel_Large_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

TSharedRef<SWidget> UWUCharacterCreatorWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, CreatorSize, FMargin(0.24f));

	return SNew(SBox)
		.WidthOverride(CreatorSize.X)
		.HeightOverride(CreatorSize.Y)
		.Visibility_UObject(this, &UWUCharacterCreatorWidget::GetCreatorVisibility)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(18.0f, 16.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CreatorTitle", "Create Character"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 21))
					.ColorAndOpacity(LabelColor)
					.ShadowOffset(FVector2D(1.0f, 1.0f))
					.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 14.0f, 0.0f, 6.0f))
				[
					CreateHeaderText(LOCTEXT("NameLabel", "Name"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(NameInputBox, SEditableTextBox)
					.Text_Lambda([this]()
					{
						return FText::FromString(CurrentRequest.CharacterName);
					})
					.HintText(LOCTEXT("NameHint", "Character name"))
					.OnTextChanged_UObject(this, &UWUCharacterCreatorWidget::HandleNameChanged)
					.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 16.0f, 0.0f, 6.0f))
				[
					CreateHeaderText(LOCTEXT("RaceLabel", "Race"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						CreateButton(LOCTEXT("Halfblood", "Halfblood"), [this]()
						{
							SetRace(EWUCharacterRace::Halfblood);
							return FReply::Handled();
						})
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateButton(LOCTEXT("Pureblood", "Pureblood"), [this]()
						{
							SetRace(EWUCharacterRace::Pureblood);
							return FReply::Handled();
						})
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateButton(LOCTEXT("Mudblood", "Mudblood"), [this]()
						{
							SetRace(EWUCharacterRace::Mudblood);
							return FReply::Handled();
						})
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 14.0f, 0.0f, 6.0f))
				[
					CreateHeaderText(LOCTEXT("SexLabel", "Body"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						CreateButton(LOCTEXT("Male", "Male"), [this]()
						{
							SetSex(EWUCharacterSex::Male);
							return FReply::Handled();
						})
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateButton(LOCTEXT("Female", "Female"), [this]()
						{
							SetSex(EWUCharacterSex::Female);
							return FReply::Handled();
						})
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
				[
					CreateStepperRow(
						LOCTEXT("SkinPreset", "Skin"),
						TAttribute<FText>::CreateLambda([this]()
						{
							return FText::Format(LOCTEXT("SkinValue", "Preset {0}"), FText::AsNumber(CurrentRequest.SkinPresetIndex + 1));
						}),
						[this]() { CycleSkinPreset(-1); },
						[this]() { CycleSkinPreset(1); }
					)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					CreateStepperRow(
						LOCTEXT("HairStyle", "Hair"),
						TAttribute<FText>::CreateLambda([this]()
						{
							return FText::Format(LOCTEXT("HairValue", "Style {0}"), FText::AsNumber(CurrentRequest.HairStyleIndex + 1));
						}),
						[this]() { CycleHairStyle(-1); },
						[this]() { CycleHairStyle(1); }
					)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
				[
					CreateStepperRow(
						LOCTEXT("HairColor", "Hair Color"),
						TAttribute<FText>::CreateLambda([this]()
						{
							return FText::Format(LOCTEXT("HairColorValue", "Color {0}"), FText::AsNumber(CurrentRequest.HairColorIndex + 1));
						}),
						[this]() { CycleHairColor(-1); },
						[this]() { CycleHairColor(1); }
					)
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 18.0f, 0.0f, 0.0f))
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						CreateButton(LOCTEXT("RotateLeft", "Rotate Left"), [this]()
						{
							RotatePreview(-20.0f);
							return FReply::Handled();
						})
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateButton(LOCTEXT("RotateRight", "Rotate Right"), [this]()
						{
							RotatePreview(20.0f);
							return FReply::Handled();
						})
					]
				]

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SNullWidget::NullWidget
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					CreateButton(LOCTEXT("CreateCharacter", "Create Character"), [this]()
					{
						return HandleCreateClicked();
					})
				]
			]
		];
}

void UWUCharacterCreatorWidget::ShowCreator()
{
	bCreatorOpen = true;
	RefreshPreview();
	InvalidateLayoutAndVolatility();
}

void UWUCharacterCreatorWidget::HideCreator()
{
	bCreatorOpen = false;
	InvalidateLayoutAndVolatility();
}

void UWUCharacterCreatorWidget::ToggleCreator()
{
	bCreatorOpen ? HideCreator() : ShowCreator();
}

bool UWUCharacterCreatorWidget::IsCreatorOpen() const
{
	return bCreatorOpen;
}

FWUCharacterCreateRequest UWUCharacterCreatorWidget::GetCurrentRequest() const
{
	return CurrentRequest;
}

EVisibility UWUCharacterCreatorWidget::GetCreatorVisibility() const
{
	return bCreatorOpen ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

void UWUCharacterCreatorWidget::RefreshPreview() const
{
	if (AWUPlayerController* PC = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		PC->PreviewCharacterCreateRequest(CurrentRequest);
	}
}

void UWUCharacterCreatorWidget::SetRace(EWUCharacterRace NewRace)
{
	CurrentRequest.Race = NewRace;
	RefreshPreview();
}

void UWUCharacterCreatorWidget::SetSex(EWUCharacterSex NewSex)
{
	CurrentRequest.Sex = NewSex;
	CurrentRequest.HairStyleIndex = 0;
	RefreshPreview();
}

void UWUCharacterCreatorWidget::CycleSkinPreset(int32 Delta)
{
	CurrentRequest.SkinPresetIndex = FMath::Clamp(CurrentRequest.SkinPresetIndex + Delta, 0, 2);
	RefreshPreview();
}

void UWUCharacterCreatorWidget::CycleHairStyle(int32 Delta)
{
	const int32 MaxHairIndex = CurrentRequest.Sex == EWUCharacterSex::Female ? 6 : 4;
	CurrentRequest.HairStyleIndex = FMath::Clamp(CurrentRequest.HairStyleIndex + Delta, 0, MaxHairIndex);
	RefreshPreview();
}

void UWUCharacterCreatorWidget::CycleHairColor(int32 Delta)
{
	CurrentRequest.HairColorIndex = FMath::Clamp(CurrentRequest.HairColorIndex + Delta, 0, 3);
	RefreshPreview();
}

void UWUCharacterCreatorWidget::RotatePreview(float YawDelta) const
{
	if (AWUPlayerController* PC = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		PC->RotateCharacterCreatorPreview(YawDelta);
	}
}

void UWUCharacterCreatorWidget::SubmitCreateRequest()
{
	if (AWUPlayerController* PC = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		PC->SubmitCharacterCreateRequest(CurrentRequest);
	}
}

FReply UWUCharacterCreatorWidget::HandleCreateClicked()
{
	SubmitCreateRequest();
	return FReply::Handled();
}

void UWUCharacterCreatorWidget::HandleNameChanged(const FText& Text)
{
	CurrentRequest.CharacterName = Text.ToString().TrimStartAndEnd();
}

TSharedRef<SWidget> UWUCharacterCreatorWidget::CreateHeaderText(const FText& Text) const
{
	return SNew(STextBlock)
		.Text(Text)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 13))
		.ColorAndOpacity(LabelColor)
		.ShadowOffset(FVector2D(1.0f, 1.0f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
}

TSharedRef<SWidget> UWUCharacterCreatorWidget::CreateValueText(TAttribute<FText> Text) const
{
	return SNew(STextBlock)
		.Text(Text)
		.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
		.ColorAndOpacity(ValueColor)
		.ShadowOffset(FVector2D(1.0f, 1.0f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.75f));
}

TSharedRef<SWidget> UWUCharacterCreatorWidget::CreateButton(const FText& Text, TFunction<FReply()> Handler) const
{
	return SNew(SButton)
		.OnClicked_Lambda(MoveTemp(Handler))
		.ContentPadding(FMargin(8.0f, 5.0f))
		[
			SNew(STextBlock)
			.Text(Text)
			.Justification(ETextJustify::Center)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.ColorAndOpacity(ValueColor)
		];
}

TSharedRef<SWidget> UWUCharacterCreatorWidget::CreateStepperRow(const FText& Label, TAttribute<FText> ValueText, TFunction<void()> PreviousHandler, TFunction<void()> NextHandler) const
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.VAlign(VAlign_Center)
		[
			CreateHeaderText(Label)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			CreateButton(LOCTEXT("Previous", "<"), [PreviousHandler = MoveTemp(PreviousHandler)]()
			{
				PreviousHandler();
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(FMargin(8.0f, 0.0f))
		[
			CreateValueText(ValueText)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			CreateButton(LOCTEXT("Next", ">"), [NextHandler = MoveTemp(NextHandler)]()
			{
				NextHandler();
				return FReply::Handled();
			})
		];
}

void UWUCharacterCreatorWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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

#undef LOCTEXT_NAMESPACE
