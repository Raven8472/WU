// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "GameFramework/PlayerController.h"
#include "WULoginPlayerController.generated.h"

class ASceneCapture2D;
class AWUCharacterCreatorPreviewActor;
class UWUCharacterSelectWidget;
class UWULoginScreenWidget;
class UTextureRenderTarget2D;

UCLASS()
class WU_API AWULoginPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AWULoginPlayerController();

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void PreviewCharacterCreateRequest(const FWUCharacterCreateRequest& Request);

	UFUNCTION(BlueprintCallable, Category = "Character Creation")
	void RotateCharacterCreatorPreview(float YawDelta);

	UFUNCTION(BlueprintPure, Category = "Character Creation")
	UTextureRenderTarget2D* GetCharacterCreatorPreviewRenderTarget() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, Category = "WU|Login")
	TSubclassOf<UWULoginScreenWidget> LoginWidgetClass;

	UPROPERTY(EditAnywhere, Category = "WU|Login")
	TSubclassOf<UWUCharacterSelectWidget> CharacterSelectWidgetClass;

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	TSubclassOf<AWUCharacterCreatorPreviewActor> CharacterCreatorPreviewActorClass;

	UPROPERTY()
	TObjectPtr<UWULoginScreenWidget> LoginWidget;

	UPROPERTY()
	TObjectPtr<UWUCharacterSelectWidget> CharacterSelectWidget;

	UPROPERTY(Transient)
	TObjectPtr<AWUCharacterCreatorPreviewActor> CharacterCreatorPreviewActor;

	UPROPERTY(Transient)
	TObjectPtr<ASceneCapture2D> CharacterCreatorSceneCaptureActor;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> CharacterCreatorPreviewRenderTarget;

private:
	UFUNCTION()
	void ShowCharacterSelect();

	void ApplyMenuInputMode();
	void SetupCharacterCreatorPreviewRig();
	AWUCharacterCreatorPreviewActor* EnsureCharacterCreatorPreviewActor();
	void PositionCharacterCreatorPreviewCapture();
};
