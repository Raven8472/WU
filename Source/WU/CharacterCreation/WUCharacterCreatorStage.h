// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CharacterCreation/WUCharacterCreationTypes.h"
#include "GameFramework/Actor.h"
#include "WUCharacterCreatorStage.generated.h"

class UArrowComponent;
class USceneComponent;

/**
 * Editor-placed anchor for a character creation background scene.
 * Drop one in each authored diorama and set BloodStatus to route previews there.
 */
UCLASS(Blueprintable)
class WU_API AWUCharacterCreatorStage : public AActor
{
	GENERATED_BODY()

public:
	AWUCharacterCreatorStage();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage")
	EWUCharacterRace BloodStatus = EWUCharacterRace::Halfblood;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage|Streaming")
	bool bPrestreamNearbyMeshes = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage|Streaming", meta = (EditCondition = "bPrestreamNearbyMeshes", ClampMin = "0.0", Units = "cm"))
	float PrestreamRadius = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage|Streaming", meta = (EditCondition = "bPrestreamNearbyMeshes", ClampMin = "1.0", Units = "s"))
	float PrestreamDurationSeconds = 300.0f;

	UFUNCTION(BlueprintPure, Category = "WU|Character Creation Stage")
	FTransform GetCharacterSpawnTransform() const;

	UFUNCTION(BlueprintPure, Category = "WU|Character Creation Stage")
	FVector GetCameraLocation() const;

	UFUNCTION(BlueprintPure, Category = "WU|Character Creation Stage")
	FVector GetLookAtLocation() const;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage")
	TObjectPtr<USceneComponent> StageRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage")
	TObjectPtr<UArrowComponent> CharacterSpawnPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage")
	TObjectPtr<UArrowComponent> CameraPoint;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "WU|Character Creation Stage")
	TObjectPtr<UArrowComponent> LookAtPoint;

private:
	void PrestreamStageMeshes() const;
};
