// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "WUChatWidget.generated.h"

class SEditableTextBox;
class SScrollBox;
class SVerticalBox;
class UTexture2D;

USTRUCT(BlueprintType)
struct FWUChatMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FText SenderName;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FText Message;
};

/**
 * Native player chat widget for the HUD.
 * Displays local chat messages and exposes a focused input line when chat is opened.
 */
UCLASS(Blueprintable)
class WU_API UWUChatWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUChatWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddChatMessage(const FText& SenderName, const FText& Message);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void OpenInput();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void CloseInput();

	UFUNCTION(BlueprintPure, Category = "Chat")
	bool IsInputOpen() const;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D ChatSize = FVector2D(520.0f, 220.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	int32 MaxStoredMessages = 64;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor SenderColor = FSlateColor(FLinearColor(0.95f, 0.78f, 0.45f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MessageColor = FSlateColor(FLinearColor(0.94f, 0.91f, 0.84f, 1.0f));

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	EVisibility GetChatVisibility() const;
	EVisibility GetInputVisibility() const;
	void HandleTextCommitted(const FText& Text, ETextCommit::Type CommitType);
	void RefreshMessages();
	TSharedRef<SWidget> CreateMessageRow(const FWUChatMessage& ChatMessage) const;

	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));
	void ConfigureColorBrush(FSlateBrush& Brush, const FLinearColor& Color, const FVector2D& ImageSize);

private:

	bool bInputOpen = false;
	TArray<FWUChatMessage> Messages;

	TSharedPtr<SVerticalBox> MessageListBox;
	TSharedPtr<SScrollBox> MessageScrollBox;
	TSharedPtr<SEditableTextBox> ChatInputBox;

	FSlateBrush PanelBrush;
	FSlateBrush InputBackgroundBrush;
};
