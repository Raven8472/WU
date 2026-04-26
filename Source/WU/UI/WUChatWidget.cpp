// Copyright Epic Games, Inc. All Rights Reserved.

#include "UI/WUChatWidget.h"
#include "WUPlayerController.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "WUChatWidget"

UWUChatWidget::UWUChatWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(false);

	static ConstructorHelpers::FObjectFinder<UTexture2D> PanelAsset(TEXT("/Game/UI/HUD/CorePack/T_HUD_ChatPanel_9Slice.T_HUD_ChatPanel_9Slice"));
	if (PanelAsset.Succeeded())
	{
		PanelTexture = PanelAsset.Object;
	}
}

TSharedRef<SWidget> UWUChatWidget::RebuildWidget()
{
	ConfigureImageBrush(PanelBrush, PanelTexture, ChatSize, FMargin(0.24f));
	ConfigureColorBrush(InputBackgroundBrush, FLinearColor(0.025f, 0.02f, 0.018f, 0.94f), FVector2D(ChatSize.X - 20.0f, 28.0f));

	TSharedRef<SWidget> Widget =
		SNew(SBox)
		.WidthOverride(ChatSize.X)
		.HeightOverride(ChatSize.Y)
		.Visibility_UObject(this, &UWUChatWidget::GetChatVisibility)
		[
			SNew(SBorder)
			.BorderImage(&PanelBrush)
			.Padding(FMargin(10.0f, 8.0f))
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					SAssignNew(MessageScrollBox, SScrollBox)
					.Orientation(Orient_Vertical)
					.ScrollBarVisibility(EVisibility::Collapsed)

					+ SScrollBox::Slot()
					[
						SAssignNew(MessageListBox, SVerticalBox)
					]
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.0f, 6.0f, 0.0f, 0.0f))
				[
					SNew(SBorder)
					.BorderImage(&InputBackgroundBrush)
					.Padding(FMargin(7.0f, 2.0f))
					.Visibility_UObject(this, &UWUChatWidget::GetInputVisibility)
					[
						SAssignNew(ChatInputBox, SEditableTextBox)
						.HintText(LOCTEXT("InputHint", "/say"))
						.OnTextCommitted_UObject(this, &UWUChatWidget::HandleTextCommitted)
						.SelectAllTextWhenFocused(false)
						.ClearKeyboardFocusOnCommit(false)
						.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
					]
				]
			]
		];

	RefreshMessages();
	return Widget;
}

void UWUChatWidget::AddChatMessage(const FText& SenderName, const FText& Message)
{
	if (Message.IsEmpty())
	{
		return;
	}

	Messages.Add({ SenderName, Message });

	while (Messages.Num() > MaxStoredMessages)
	{
		Messages.RemoveAt(0);
	}

	RefreshMessages();
}

void UWUChatWidget::OpenInput()
{
	bInputOpen = true;
	InvalidateLayoutAndVolatility();

	if (ChatInputBox)
	{
		ChatInputBox->SetText(FText::GetEmpty());

		if (FSlateApplication::IsInitialized())
		{
			FSlateApplication::Get().SetKeyboardFocus(ChatInputBox, EFocusCause::SetDirectly);
		}
	}
}

void UWUChatWidget::CloseInput()
{
	bInputOpen = false;
	InvalidateLayoutAndVolatility();

	if (ChatInputBox)
	{
		ChatInputBox->SetText(FText::GetEmpty());
	}

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
	}
}

bool UWUChatWidget::IsInputOpen() const
{
	return bInputOpen;
}

EVisibility UWUChatWidget::GetChatVisibility() const
{
	return bInputOpen || !Messages.IsEmpty() ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed;
}

EVisibility UWUChatWidget::GetInputVisibility() const
{
	return bInputOpen ? EVisibility::Visible : EVisibility::Collapsed;
}

void UWUChatWidget::HandleTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		if (AWUPlayerController* PC = Cast<AWUPlayerController>(GetOwningPlayer()))
		{
			PC->SubmitChatMessage(Text.ToString());
		}

		CloseInput();
		return;
	}

	if (CommitType == ETextCommit::OnCleared)
	{
		CloseInput();
	}
}

void UWUChatWidget::RefreshMessages()
{
	if (!MessageListBox)
	{
		return;
	}

	MessageListBox->ClearChildren();

	for (const FWUChatMessage& ChatMessage : Messages)
	{
		MessageListBox->AddSlot()
		.AutoHeight()
		.Padding(FMargin(0.0f, 0.0f, 0.0f, 3.0f))
		[
			CreateMessageRow(ChatMessage)
		];
	}

	if (MessageScrollBox)
	{
		MessageScrollBox->ScrollToEnd();
	}
}

TSharedRef<SWidget> UWUChatWidget::CreateMessageRow(const FWUChatMessage& ChatMessage) const
{
	return SNew(STextBlock)
		.Text(FText::Format(LOCTEXT("ChatLineFormat", "{0}: {1}"), ChatMessage.SenderName, ChatMessage.Message))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 13))
		.ColorAndOpacity(MessageColor)
		.ShadowOffset(FVector2D(1.0f, 1.0f))
		.ShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f))
		.AutoWrapText(true);
}

void UWUChatWidget::ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin)
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

void UWUChatWidget::ConfigureColorBrush(FSlateBrush& Brush, const FLinearColor& Color, const FVector2D& ImageSize)
{
	Brush.TintColor = FSlateColor(Color);
	Brush.ImageSize = ImageSize;
	Brush.DrawAs = ESlateBrushDrawType::Box;
}

#undef LOCTEXT_NAMESPACE
