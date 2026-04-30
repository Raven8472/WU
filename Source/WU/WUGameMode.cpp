// Copyright Epic Games, Inc. All Rights Reserved.

#include "WUGameMode.h"
#include "WUCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

AWUGameMode::AWUGameMode()
{
	DefaultPawnClass = nullptr;

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			5.0f,
			FColor::Green,
			TEXT("WUGameMode Active")
		);
	}
}

void AWUGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (!NewPlayer)
	{
		return;
	}

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.IsEmpty())
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.Instigator = nullptr;

	if (!PlayerPawnClass)
	{
		return;
	}

	PlayerStarts.Sort([](const AActor& Left, const AActor& Right)
	{
		return Left.GetName() < Right.GetName();
	});

	const int32 SpawnIndex = SpawnedPlayerCount++;
	const int32 StartIndex = SpawnIndex % PlayerStarts.Num();
	const AActor* StartSpot = PlayerStarts[StartIndex];
	const FVector StartLocation = StartSpot->GetActorLocation();
	const FVector FallbackOffset = PlayerStarts.Num() == 1
		? StartSpot->GetActorRightVector() * (static_cast<float>(SpawnIndex) * FallbackSpawnSpacing)
		: FVector::ZeroVector;

	APawn* NewCharacter = GetWorld()->SpawnActor<APawn>(
		PlayerPawnClass,
		StartLocation + FallbackOffset,
		StartSpot->GetActorRotation(),
		SpawnParams
	);

	if (NewCharacter)
	{
		NewPlayer->Possess(NewCharacter);
	}
}
