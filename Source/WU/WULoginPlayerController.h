// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "GameFramework/PlayerController.h"
#include "WULoginPlayerController.generated.h"

class ASceneCapture2D;
class AWUCharacterCreatorPreviewActor;
class AWUCharacterCreatorStage;
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

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FIntPoint CharacterCreatorPreviewRenderTargetSize = FIntPoint(2560, 1440);

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float CharacterCreatorPreviewSharpen = 0.45f;

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation", meta = (ClampMin = "1.0", Units = "s"))
	float CharacterCreatorPreviewTextureStreamSeconds = 300.0f;

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FVector CharacterCreatorPreviewStageLocation = FVector(0.0f, 0.0f, 100.0f);

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FRotator CharacterCreatorPreviewStageRotation = FRotator(0.0f, 180.0f, 0.0f);

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FVector CharacterCreatorPreviewCameraOffset = FVector(230.0f, -470.0f, 145.0f);

	UPROPERTY(EditAnywhere, Category = "UI|Character Creation")
	FVector CharacterCreatorPreviewLookAtOffset = FVector(0.0f, 0.0f, 92.0f);

	EWUCharacterRace ActiveCharacterCreatorPreviewRace = EWUCharacterRace::Halfblood;

	bool bHasActiveCharacterCreatorPreviewRace = false;

private:
	UFUNCTION()
	void ShowCharacterSelect();

	UFUNCTION()
	void EnterGame();

	UFUNCTION(Server, Reliable)
	void ServerRequestEnterGame();

	void RequestEnterGameOnServer();
	void RemoveMenuWidgets();
	void ApplyMenuInputMode();
	void SetupCharacterCreatorPreviewRig();
	AWUCharacterCreatorPreviewActor* EnsureCharacterCreatorPreviewActor();
	const AWUCharacterCreatorStage* FindCharacterCreatorStage(EWUCharacterRace Race) const;
	FTransform ResolveCharacterCreatorSpawnTransform(EWUCharacterRace Race) const;
	void ApplyCharacterCreatorStage(EWUCharacterRace Race, bool bForce = false);
	void PositionCharacterCreatorPreviewCapture();
};
