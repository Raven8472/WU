// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Backend/WUClientSessionSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/WUInventoryTypes.h"
#include "NPC/WUNpcTypes.h"
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
	void ShowVendor(const FWUNpcProfile& NpcProfile, FText VendorName, FText GreetingText);

	UFUNCTION(BlueprintCallable, Category = "Vendor")
	void HideVendor();

	UFUNCTION(BlueprintPure, Category = "Vendor")
	bool IsVendorOpen() const;

protected:

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance")
	FVector2D VendorSize = FVector2D(460.0f, 560.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Textures")
	TObjectPtr<UTexture2D> PanelTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor LabelColor = FSlateColor(FLinearColor(0.96f, 0.85f, 0.65f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor MutedLabelColor = FSlateColor(FLinearColor(0.60f, 0.55f, 0.46f, 1.0f));

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Appearance|Colors")
	FSlateColor ValueColor = FSlateColor(FLinearColor(0.92f, 0.94f, 0.98f, 1.0f));

private:

	enum class EVendorPanelMode : uint8
	{
		Dialogue,
		Merchant,
		Buyback
	};

	static constexpr int32 ItemsPerMerchantPage = 10;

	EVisibility GetVendorVisibility() const;
	FText GetVendorTitleText() const;
	FText GetVendorGreetingText() const;
	FText GetVendorStatusText() const;
	EVisibility GetVendorStatusVisibility() const;
	const FWUVendorTable* ResolveActiveVendorTable() const;
	FName GetResolvedVendorTableId() const;
	void RefreshActiveContent();
	TSharedRef<SWidget> CreateActiveContent();
	TSharedRef<SWidget> CreateHeaderSection();
	TSharedRef<SWidget> CreateDialogueSection();
	TSharedRef<SWidget> CreateMerchantSection();
	TSharedRef<SWidget> CreateBuybackSection();
	TSharedRef<SWidget> CreateTabButton(FText Label, EVendorPanelMode TargetMode) const;
	TSharedRef<SWidget> CreateVendorItemCard(const FWUVendorItem& VendorItem);
	TSharedRef<SWidget> CreateRepairSection();
	TSharedRef<SWidget> CreatePageControls() const;
	const FSlateBrush* GetItemIconBrush(const FWUInventoryItem& Item, const FVector2D& ImageSize);
	bool HasMerchantItems() const;
	int32 GetMerchantPageCount() const;
	int32 GetClampedMerchantPageIndex() const;
	FText GetMerchantPageText() const;
	FReply HandleBuyClicked(FName ItemId);
	FReply HandleBrowseClicked();
	FReply HandleMerchantTabClicked();
	FReply HandleBuybackTabClicked();
	FReply HandlePreviousPageClicked();
	FReply HandleNextPageClicked();
	FReply HandleRepairClicked(bool bRepairAll);
	FReply HandleQuestClicked();
	FReply HandleGoodbyeClicked();
	void ConfigureImageBrush(FSlateBrush& Brush, UTexture2D* Texture, const FVector2D& ImageSize, const FMargin& Margin = FMargin(0.0f));
	UWUClientSessionSubsystem* GetSessionSubsystem() const;

	UFUNCTION()
	void HandleVendorPurchaseCompleted(const FWUBackendVendorPurchase& Purchase);

	UFUNCTION()
	void HandleSessionRequestFailed(const FString& ErrorMessage);

private:

	bool bVendorOpen = false;
	EVendorPanelMode ActiveMode = EVendorPanelMode::Dialogue;
	FName ActiveVendorTableId;
	FWUNpcProfile ActiveNpcProfile;
	FText ActiveVendorName;
	FText ActiveGreetingText;
	FText StatusText;
	int32 MerchantPageIndex = 0;
	TSharedPtr<SBox> ActiveContentBox;
	FSlateBrush PanelBrush;
	TMap<FString, FSlateBrush> IconBrushCache;
	TMap<FString, TObjectPtr<UTexture2D>> IconTextureCache;
};
