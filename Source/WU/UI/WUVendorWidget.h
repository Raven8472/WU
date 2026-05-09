// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/WUInventoryTypes.h"
#include "Styling/SlateBrush.h"
#include "WUVendorWidget.generated.h"

class UTexture2D;

/**
 * Native vendor shell for prototype NPC shops.
 * Uses the local item catalog while backend purchase requests own the currency spend.
 */
UCLASS(Blueprintable)
class WU_API UWUVendorWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UWUVendorWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void ShowVendor(FName VendorTableId, FText VendorName, FText GreetingText);

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void HideVendor();

	UFUNCTION(BlueprintPure, Category = "Vendor")
	bool IsVendorOpen() const;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D VendorSize = FVector2D(380.0f, 260.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MutedLabelColor = FSlateColor(FLinearColor(0.60f, 0.55f, 0.46f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(0.92f, 0.94f, 0.98f, 1.0f));

private:

	EVisibility GetVendorVisibility() const;
	FText GetVendorTitleText() const;
	FText GetVendorGreetingText() const;
	FText GetVendorStatusText() const;
	EVisibility GetVendorStatusVisibility() const;
	const FWUVendorTable* ResolveActiveVendorTable() const;
	FName GetResolvedVendorTableId() const;
	TSharedRef<SWidget> CreateVendorItemsSection();
	TSharedRef<SWidget> CreateVendorItemRow(const FWUVendorItem& VendorItem);
	FReply HandleBuyClicked(FName ItemId);
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));
	UWUClientSessionSubsystem* GetSessionSubsystem() const;

	UFUNCTION()
	void HandleVendorPurchaseCompleted(const FWUBackendVendorPurchase& Purchase);

	UFUNCTION()
	void HandleSessionRequestFailed(const FString& ErrorMessage);

private:

	bool bVendorOpen = false;
	FName ActiveVendorTableId;
	FText ActiveVendorName;
	FText ActiveGreetingText;
	FText StatusText;
	FSlateBrush PanelBrush;
};
