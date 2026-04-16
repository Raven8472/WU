// Copyright Epic Games, Inc. All Rights Reserved.

#include "WUCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "WU.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "DrawDebugHelpers.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AWUCharacter::AWUCharacter()
{
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bReplicates = true;

	Health = 100.0f;
	bIsDead = false;
	bHasReleased = false;
	DeathWidget = nullptr;
}

void AWUCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWUCharacter, Health);
	DOREPLIFETIME(AWUCharacter, bIsDead);
	DOREPLIFETIME(AWUCharacter, DeathLocation);
	DOREPLIFETIME(AWUCharacter, bHasReleased);
}

void AWUCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		Health = 100.0f;
		bIsDead = false;
		bHasReleased = false;
	}

	DeathWidget = nullptr;
}

void AWUCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AWUCharacter::Move);

		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AWUCharacter::Look);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AWUCharacter::Look);
		//attack
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AWUCharacter::StartAttack);
		//release
		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &AWUCharacter::RequestRelease);
	}

	else
	{
		UE_LOG(LogWU, Error, TEXT("'%s' Failed to find an Enhanced Input component!"), *GetNameSafe(this));
	}
}

void AWUCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	DoMove(MovementVector.X, MovementVector.Y);
}

void AWUCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AWUCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AWUCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AWUCharacter::DoJumpStart()
{
	Jump();
}

void AWUCharacter::DoJumpEnd()
{
	StopJumping();
}

void AWUCharacter::StartAttack()
{
	if (bIsDead)
	{
		return;
	}

	ServerAttack();
}

void AWUCharacter::ServerAttack_Implementation()
{
	if (bIsDead)
	{
		return;
	}

	PerformAttackTrace();
}

void AWUCharacter::PerformAttackTrace()
{
	const FVector Start = GetActorLocation();
	const FVector End = Start + (GetActorForwardVector() * 200.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Pawn,
		Params
	);

	DrawDebugLine(GetWorld(), Start, End, bHit ? FColor::Red : FColor::Green, false, 1.5f, 0, 2.0f);

	if (bHit)
	{
		AWUCharacter* HitCharacter = Cast<AWUCharacter>(Hit.GetActor());

		if (HitCharacter && HitCharacter != this)
		{
			const bool bDamageApplied = HitCharacter->ApplyDamage(10.0f);

			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					2.0f,
					bDamageApplied ? FColor::Red : FColor::Yellow,
					bDamageApplied ? TEXT("Hit player and applied damage") : TEXT("Hit dead player - no damage")
				);
			}
		}
	}
}

bool AWUCharacter::ApplyDamage(float Amount)
{
	if (!HasAuthority())
	{
		return false;
	}

	if (bIsDead || Health <= 0.0f)
	{
		return false;
	}

	Health -= Amount;
	Health = FMath::Max(Health, 0.0f);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Red,
			FString::Printf(TEXT("%s Health = %.1f"), *GetName(), Health)
		);
	}

	if (Health <= 0.0f)
	{
		bIsDead = true;
		bHasReleased = false;
		DeathLocation = GetActorLocation();

		GetCharacterMovement()->DisableMovement();

		GetWorldTimerManager().SetTimer(
			ReleaseTimerHandle,
			this,
			&AWUCharacter::HandleAutoRelease,
			15.0f,
			false
		);

		if (IsLocallyControlled() && DeathWidgetClass && !DeathWidget)
		{
			DeathWidget = CreateWidget<UUserWidget>(GetWorld(), DeathWidgetClass);

			if (DeathWidget)
			{
				DeathWidget->AddToViewport();
			}
		}

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Purple, TEXT("Player died - 15s to release"));
		}
	}

	return true;
}

void AWUCharacter::HandleAutoRelease()
{
	if (bIsDead && !bHasReleased)
	{
		ReleaseToGraveyard();
	}
}

void AWUCharacter::ReleaseToGraveyard()
{
	if (!HasAuthority())
	{
		return;
	}

	bHasReleased = true;

	// Released players can move again to run back to their corpse
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);

	FVector GraveyardLocation = GetActorLocation(); // fallback

	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), FoundActors);

	for (AActor* Actor : FoundActors)
	{
		if (Actor && Actor->GetName().Contains(TEXT("BP_Graveyard")))
		{
			GraveyardLocation = Actor->GetActorLocation();
			break;
		}
	}

	SetActorLocation(GraveyardLocation, false, nullptr, ETeleportType::TeleportPhysics);
	ForceNetUpdate();

	if (DeathWidget)
	{
		DeathWidget->RemoveFromParent();
		DeathWidget = nullptr;
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Blue,
			TEXT("Released to Graveyard")
		);
	}
}

void AWUCharacter::RequestRelease()
{
	if (!bIsDead)
	{
		return;
	}

	// If dead but not yet released, go to graveyard
	if (!bHasReleased)
	{
		ServerRequestRelease();
		return;
	}

	// If already released, allow revive when close enough to corpse
	const float DistanceToCorpse = FVector::Dist(GetActorLocation(), DeathLocation);

	if (DistanceToCorpse <= 200.0f)
	{
		ServerRequestRelease();
	}
}

void AWUCharacter::ServerRequestRelease_Implementation()
{
	if (!bIsDead)
	{
		return;
	}

	// First press while dead: release to graveyard
	if (!bHasReleased)
	{
		ReleaseToGraveyard();
		return;
	}

	// Already released: revive if close enough to corpse
	const float DistanceToCorpse = FVector::Dist(GetActorLocation(), DeathLocation);

	if (DistanceToCorpse <= 200.0f)
	{
		ReviveAtCorpse();
	}
}

void AWUCharacter::ReviveAtCorpse()
{
	if (!HasAuthority())
	{
		return;
	}

	Health = 100.0f;
	bIsDead = false;
	bHasReleased = false;

	SetActorLocation(DeathLocation, false, nullptr, ETeleportType::TeleportPhysics);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	ForceNetUpdate();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			3.0f,
			FColor::Green,
			TEXT("Revived at corpse")
		);
	}
}