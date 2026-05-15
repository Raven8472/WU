// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUCharacterCreatorWidget.h"
#include "WULoginPlayerController.h"
#include "WUPlayerController.h"
#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
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
			.Padding(FMargin(16.0f, 14.0f))
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
				.FillHeight(1.0f)
				.Padding(FMargin(0.0f, 12.0f, 0.0f, 12.0f))
				[
					SNew(SScrollBox)

					+ SScrollBox::Slot()
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 0.0f, 0.0f, 6.0f))
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
						.Padding(FMargin(0.0f, 14.0f, 0.0f, 6.0f))
						[
							CreateHeaderText(LOCTEXT("RaceLabel", "Blood Status"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								CreateChoiceButton(LOCTEXT("Halfblood", "Half-blood"), [this]()
								{
									return CurrentRequest.Race == EWUCharacterRace::Halfblood;
								}, [this]()
								{
									SetRace(EWUCharacterRace::Halfblood);
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("Pureblood", "Pureblood"), [this]()
								{
									return CurrentRequest.Race == EWUCharacterRace::Pureblood;
								}, [this]()
								{
									SetRace(EWUCharacterRace::Pureblood);
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("MuggleBorn", "Muggle-born"), [this]()
								{
									return CurrentRequest.Race == EWUCharacterRace::Mudblood;
								}, [this]()
								{
									SetRace(EWUCharacterRace::Mudblood);
									return FReply::Handled();
								})
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 16.0f, 0.0f, 6.0f))
						[
							CreateHeaderText(LOCTEXT("PathLabel", "Path"))
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								CreateChoiceButton(LOCTEXT("AurorPath", "Auror"), [this]()
								{
									return SelectedPathId == FName(TEXT("Auror"));
								}, [this]()
								{
									SetPath(FName(TEXT("Auror")));
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("MagizoologistPath", "Magizoologist"), [this]()
								{
									return SelectedPathId == FName(TEXT("Magizoologist"));
								}, [this]()
								{
									SetPath(FName(TEXT("Magizoologist")));
									return FReply::Handled();
								})
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								CreateChoiceButton(LOCTEXT("PhilosopherPath", "Philosopher"), [this]()
								{
									return SelectedPathId == FName(TEXT("Philosopher"));
								}, [this]()
								{
									SetPath(FName(TEXT("Philosopher")));
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("CurseBreakerPath", "Curse Breaker"), [this]()
								{
									return SelectedPathId == FName(TEXT("CurseBreaker"));
								}, [this]()
								{
									SetPath(FName(TEXT("CurseBreaker")));
									return FReply::Handled();
								})
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								CreateChoiceButton(LOCTEXT("MediwizardPath", "Mediwizard"), [this]()
								{
									return SelectedPathId == FName(TEXT("MediwitchMediwizard"));
								}, [this]()
								{
									SetPath(FName(TEXT("MediwitchMediwizard")));
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("BlackMarketPath", "Black Market"), [this]()
								{
									return SelectedPathId == FName(TEXT("BlackMarket"));
								}, [this]()
								{
									SetPath(FName(TEXT("BlackMarket")));
									return FReply::Handled();
								})
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								CreateChoiceButton(LOCTEXT("DarkArtsPath", "Dark Arts"), [this]()
								{
									return SelectedPathId == FName(TEXT("DarkArts"));
								}, [this]()
								{
									SetPath(FName(TEXT("DarkArts")));
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("SeerPath", "Seer"), [this]()
								{
									return SelectedPathId == FName(TEXT("Seer"));
								}, [this]()
								{
									SetPath(FName(TEXT("Seer")));
									return FReply::Handled();
								})
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 16.0f, 0.0f, 6.0f))
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
								CreateChoiceButton(LOCTEXT("Male", "Male"), [this]()
								{
									return CurrentRequest.Sex == EWUCharacterSex::Male;
								}, [this]()
								{
									SetSex(EWUCharacterSex::Male);
									return FReply::Handled();
								})
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
							[
								CreateChoiceButton(LOCTEXT("Female", "Female"), [this]()
								{
									return CurrentRequest.Sex == EWUCharacterSex::Female;
								}, [this]()
								{
									SetSex(EWUCharacterSex::Female);
									return FReply::Handled();
								})
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 16.0f, 0.0f, 0.0f))
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
						.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
						[
							CreateStepperRow(
								LOCTEXT("EyeColor", "Eye Color"),
								TAttribute<FText>::CreateLambda([this]()
								{
									switch (CurrentRequest.EyeColorIndex)
									{
									case 0:
										return LOCTEXT("EyeColorBlue", "Blue");
									case 1:
										return LOCTEXT("EyeColorBrown", "Brown");
									case 2:
										return LOCTEXT("EyeColorGreen", "Green");
									case 3:
										return LOCTEXT("EyeColorPale", "Pale");
									default:
										return LOCTEXT("EyeColorUnknown", "Unknown");
									}
								}),
								[this]() { CycleEyeColor(-1); },
								[this]() { CycleEyeColor(1); }
							)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
						[
							CreateStepperRow(
								LOCTEXT("Brows", "Brows"),
								TAttribute<FText>::CreateLambda([this]()
								{
									return CurrentRequest.Sex == EWUCharacterSex::Male
										? FText::Format(LOCTEXT("BrowsValue", "Style {0}"), FText::AsNumber(CurrentRequest.BrowStyleIndex + 1))
										: LOCTEXT("BrowsUnavailable", "N/A");
								}),
								[this]() { CycleBrowStyle(-1); },
								[this]() { CycleBrowStyle(1); }
							)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(FMargin(0.0f, 8.0f, 0.0f, 0.0f))
						[
							CreateStepperRow(
								LOCTEXT("Beard", "Beard"),
								TAttribute<FText>::CreateLambda([this]()
								{
									if (CurrentRequest.Sex != EWUCharacterSex::Male)
									{
										return LOCTEXT("BeardUnavailable", "N/A");
									}

									return CurrentRequest.BeardStyleIndex == 0
										? LOCTEXT("BeardNone", "None")
										: FText::Format(LOCTEXT("BeardValue", "Style {0}"), FText::AsNumber(CurrentRequest.BeardStyleIndex));
								}),
								[this]() { CycleBeardStyle(-1); },
								[this]() { CycleBeardStyle(1); }
							)
						]
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					[
						CreateButton(LOCTEXT("Cancel", "Cancel"), [this]()
						{
							HideCreator();
							return FReply::Handled();
						})
					]

					+ SHorizontalBox::Slot()
					.FillWidth(1.0f)
					.Padding(FMargin(6.0f, 0.0f, 0.0f, 0.0f))
					[
						CreateButton(LOCTEXT("CreateCharacter", "Create Character"), [this]()
						{
							return HandleCreateClicked();
						})
					]
				]
			]
		];
}

void UWUCharacterCreatorWidget::ShowCreator()
{
	CurrentRequest.SkinPresetIndex = FMath::Clamp(CurrentRequest.SkinPresetIndex, 0, 4);
	CurrentRequest.HeadPresetIndex = CurrentRequest.SkinPresetIndex;
	if (SelectedPathId.IsNone())
	{
		SelectedPathId = FName(TEXT("Auror"));
	}
	CurrentRequest.PathId = SelectedPathId.ToString();
	bCreatorOpen = true;
	RefreshPreview();
	InvalidateLayoutAndVolatility();
}

void UWUCharacterCreatorWidget::HideCreator()
{
	const bool bWasOpen = bCreatorOpen;
	bCreatorOpen = false;
	InvalidateLayoutAndVolatility();

	if (bWasOpen)
	{
		OnCreatorClosed.Broadcast();
	}
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

FName UWUCharacterCreatorWidget::GetSelectedPathId() const
{
	return SelectedPathId;
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
		return;
	}

	if (AWULoginPlayerController* LoginPC = Cast<AWULoginPlayerController>(GetOwningPlayer()))
	{
		LoginPC->PreviewCharacterCreateRequest(CurrentRequest);
	}
}

void UWUCharacterCreatorWidget::SetRace(EWUCharacterRace NewRace)
{
	CurrentRequest.Race = NewRace;
	RefreshPreview();
	InvalidateLayoutAndVolatility();
}

void UWUCharacterCreatorWidget::SetSex(EWUCharacterSex NewSex)
{
	CurrentRequest.Sex = NewSex;
	CurrentRequest.SkinPresetIndex = FMath::Clamp(CurrentRequest.SkinPresetIndex, 0, 4);
	CurrentRequest.HeadPresetIndex = CurrentRequest.SkinPresetIndex;
	CurrentRequest.HairStyleIndex = 0;
	CurrentRequest.BrowStyleIndex = 0;
	CurrentRequest.BeardStyleIndex = 0;
	RefreshPreview();
	InvalidateLayoutAndVolatility();
}

void UWUCharacterCreatorWidget::SetPath(FName NewPathId)
{
	SelectedPathId = NewPathId.IsNone() ? FName(TEXT("Auror")) : NewPathId;
	CurrentRequest.PathId = SelectedPathId.ToString();
	InvalidateLayoutAndVolatility();
}

void UWUCharacterCreatorWidget::CycleSkinPreset(int32 Delta)
{
	CurrentRequest.SkinPresetIndex = FMath::Clamp(CurrentRequest.SkinPresetIndex + Delta, 0, 4);
	CurrentRequest.HeadPresetIndex = CurrentRequest.SkinPresetIndex;
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

void UWUCharacterCreatorWidget::CycleEyeColor(int32 Delta)
{
	CurrentRequest.EyeColorIndex = FMath::Clamp(CurrentRequest.EyeColorIndex + Delta, 0, 3);
	RefreshPreview();
}

void UWUCharacterCreatorWidget::CycleBrowStyle(int32 Delta)
{
	if (CurrentRequest.Sex != EWUCharacterSex::Male)
	{
		CurrentRequest.BrowStyleIndex = 0;
		return;
	}

	CurrentRequest.BrowStyleIndex = FMath::Clamp(CurrentRequest.BrowStyleIndex + Delta, 0, 3);
	RefreshPreview();
}

void UWUCharacterCreatorWidget::CycleBeardStyle(int32 Delta)
{
	if (CurrentRequest.Sex != EWUCharacterSex::Male)
	{
		CurrentRequest.BeardStyleIndex = 0;
		return;
	}

	CurrentRequest.BeardStyleIndex = FMath::Clamp(CurrentRequest.BeardStyleIndex + Delta, 0, 6);
	RefreshPreview();
}

void UWUCharacterCreatorWidget::RotatePreview(float YawDelta) const
{
	if (AWUPlayerController* PC = Cast<AWUPlayerController>(GetOwningPlayer()))
	{
		PC->RotateCharacterCreatorPreview(YawDelta);
		return;
	}

	if (AWULoginPlayerController* LoginPC = Cast<AWULoginPlayerController>(GetOwningPlayer()))
	{
		LoginPC->RotateCharacterCreatorPreview(YawDelta);
	}
}

void UWUCharacterCreatorWidget::SubmitCreateRequest()
{
	CurrentRequest.SkinPresetIndex = FMath::Clamp(CurrentRequest.SkinPresetIndex, 0, 4);
	CurrentRequest.HeadPresetIndex = CurrentRequest.SkinPresetIndex;

	if (OnCreateRequested.IsBound())
	{
		OnCreateRequested.Broadcast(CurrentRequest);
		return;
	}

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

TSharedRef<SWidget> UWUCharacterCreatorWidget::CreateChoiceButton(const FText& Text, TFunction<bool()> IsSelected, TFunction<FReply()> Handler) const
{
	const TSharedRef<TFunction<bool()>> IsSelectedPredicate = MakeShared<TFunction<bool()>>(MoveTemp(IsSelected));
	const FLinearColor SelectedTint(0.55f, 0.37f, 0.08f, 0.92f);
	const FLinearColor IdleTint(0.08f, 0.07f, 0.06f, 0.72f);
	const FLinearColor SelectedText(1.0f, 0.95f, 0.76f, 1.0f);
	const FLinearColor IdleText(0.86f, 0.82f, 0.74f, 1.0f);

	return SNew(SButton)
		.OnClicked_Lambda(MoveTemp(Handler))
		.ContentPadding(FMargin(9.0f, 7.0f))
		.ButtonColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([IsSelectedPredicate, SelectedTint, IdleTint]()
		{
			return (*IsSelectedPredicate)() ? FSlateColor(SelectedTint) : FSlateColor(IdleTint);
		}))
		[
			SNew(STextBlock)
			.Text(Text)
			.Justification(ETextJustify::Center)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 12))
			.ColorAndOpacity(TAttribute<FSlateColor>::CreateLambda([IsSelectedPredicate, SelectedText, IdleText]()
			{
				return (*IsSelectedPredicate)() ? FSlateColor(SelectedText) : FSlateColor(IdleText);
			}))
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
