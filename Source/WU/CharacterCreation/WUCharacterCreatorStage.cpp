// Copyright Epic Games, Inc. All Rights Reserved.

#include "CharacterCreation/WUCharacterCreatorStage.h"
#include "Components/ArrowComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"

AWUCharacterCreatorStage::AWUCharacterCreatorStage()
{
	PrimaryActorTick.bCanEverTick = false;

	StageRoot = CreateDefaultSubobject<USceneComponent>(TEXT("StageRoot"));
	SetRootComponent(StageRoot);

	CharacterSpawnPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("CharacterSpawnPoint"));
	CharacterSpawnPoint->SetupAttachment(StageRoot);
	CharacterSpawnPoint->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	CharacterSpawnPoint->ArrowColor = FColor(235, 190, 70);

	CameraPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("CameraPoint"));
	CameraPoint->SetupAttachment(StageRoot);
	CameraPoint->SetRelativeLocation(FVector(230.0f, -470.0f, 145.0f));
	CameraPoint->ArrowColor = FColor(70, 150, 235);

	LookAtPoint = CreateDefaultSubobject<UArrowComponent>(TEXT("LookAtPoint"));
	LookAtPoint->SetupAttachment(StageRoot);
	LookAtPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 92.0f));
	LookAtPoint->ArrowColor = FColor(90, 220, 130);
}

void AWUCharacterCreatorStage::BeginPlay()
{
	Super::BeginPlay();

	if (bPrestreamNearbyMeshes)
	{
		PrestreamStageMeshes();
	}
}

FTransform AWUCharacterCreatorStage::GetCharacterSpawnTransform() const
{
	return CharacterSpawnPoint ? CharacterSpawnPoint->GetComponentTransform() : GetActorTransform();
}

FVector AWUCharacterCreatorStage::GetCameraLocation() const
{
	return CameraPoint ? CameraPoint->GetComponentLocation() : GetActorLocation();
}

FVector AWUCharacterCreatorStage::GetLookAtLocation() const
{
	return LookAtPoint ? LookAtPoint->GetComponentLocation() : GetActorLocation();
}

void AWUCharacterCreatorStage::PrestreamStageMeshes() const
{
	UWorld* World = GetWorld();
	if (!World || PrestreamRadius <= 0.0f)
	{
		return;
	}

	const FVector StageLocation = GetActorLocation();
	const float RadiusSquared = FMath::Square(PrestreamRadius);

	for (TActorIterator<AActor> ActorIt(World); ActorIt; ++ActorIt)
	{
		AActor* Actor = *ActorIt;
		if (!Actor)
		{
			continue;
		}

		TArray<UMeshComponent*> MeshComponents;
		Actor->GetComponents(MeshComponents);

		for (UMeshComponent* MeshComponent : MeshComponents)
		{
			if (!MeshComponent)
			{
				continue;
			}

			const FVector ComponentLocation = MeshComponent->Bounds.Origin;
			if (FVector::DistSquared(ComponentLocation, StageLocation) > RadiusSquared)
			{
				continue;
			}

			MeshComponent->bForceMipStreaming = true;
			MeshComponent->SetTextureForceResidentFlag(true);
			MeshComponent->PrestreamTextures(PrestreamDurationSeconds, false);
			MeshComponent->PrestreamMeshLODs(PrestreamDurationSeconds);
		}
	}
}
