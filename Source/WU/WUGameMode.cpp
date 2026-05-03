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

void AWUGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}

	if (NewPlayer->GetPawn() || bStartPlayersAsSpectators || MustSpectate(NewPlayer) || !PlayerCanRestart(NewPlayer))
	{
		return;
	}

	const TSubclassOf<APawn> EffectivePlayerPawnClass = PlayerPawnClass ? PlayerPawnClass : DefaultPawnClass;
	if (!EffectivePlayerPawnClass)
	{
		return;
	}

	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.IsEmpty())
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

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = NewPlayer;
	SpawnParams.Instigator = nullptr;

	APawn* NewCharacter = GetWorld()->SpawnActor<APawn>(
		EffectivePlayerPawnClass,
		StartLocation + FallbackOffset,
		StartSpot->GetActorRotation(),
		SpawnParams
	);

	if (NewCharacter)
	{
		NewPlayer->Possess(NewCharacter);
	}
}
