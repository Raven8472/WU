// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "WUOverheadNameVisibilityComponent.generated.h"

class UWidgetComponent;

UCLASS(ClassGroup=(UI), meta=(BlueprintSpawnableComponent))
class WU_API UWUOverheadNameVisibilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UWUOverheadNameVisibilityComponent();

	void SetOverheadNameComponent(UWidgetComponent* NewOverheadNameComponent);

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nameplate", meta = (ClampMin = 0.0, Units = "cm"))
	float MaxVisibleDistance = 5600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nameplate")
	bool bRequireLineOfSight = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nameplate")
	bool bHideForLocalPawn = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Nameplate")
	TEnumAsByte<ECollisionChannel> LineOfSightTraceChannel = ECC_Visibility;

private:
	UPROPERTY()
	TObjectPtr<UWidgetComponent> OverheadNameComponent;

	bool ShouldShowNameplate() const;
	void ApplyNameplateVisibility(bool bShouldShow);
};
