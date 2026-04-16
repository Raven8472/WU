// Copyright Epic Games, Inc. All Rights Reserved.

#include "WUGameMode.h"
#include "WUCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

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

	AActor* StartSpot = UGameplayStatics::GetActorOfClass(this, APlayerStart::StaticClass());

	if (!StartSpot)
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

	APawn* NewCharacter = GetWorld()->SpawnActor<APawn>(
		PlayerPawnClass,
		StartSpot->GetActorLocation(),
		StartSpot->GetActorRotation(),
		SpawnParams
	);

	if (NewCharacter)
	{
		NewPlayer->Possess(NewCharacter);
	}
}