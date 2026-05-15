// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UObject/SoftObjectPtr.h"
#include "WUDayNightCycleActor.generated.h"

class ADirectionalLight;
class ASkyLight;

/**
 * Drives a 24-hour day/night lighting cycle from the server world clock.
 */
UCLASS(Blueprintable)
class WU_API AWUDayNightCycleActor : public AActor
{
	GENERATED_BODY()

public:
	AWUDayNightCycleActor();

	UFUNCTION(BlueprintCallable, Category = "WU|World Time")
	void RefreshServerWorldTime();

protected:
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WU|World Time|Lights")
	TObjectPtr<ADirectionalLight> SunLight;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WU|World Time|Lights")
	TObjectPtr<ADirectionalLight> MoonLight;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "WU|World Time|Lights")
	TObjectPtr<ASkyLight> SkyLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Lights")
	bool bAutoFindLights = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Server")
	bool bSyncServerTimeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky")
	bool bUseStarrySkyActorWhenAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky")
	bool bSpawnStarrySkyActorWhenAvailable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky")
	bool bDisableNativeLightDrivingWhenUsingStarrySky = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky")
	bool bFreezeStarrySkyInternalClock = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky")
	bool bDisableExternalDirectionalLightsWhenUsingStarrySky = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning")
	bool bApplyStarrySkyPrototypeTuning = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning")
	FString StarrySkyPreferredSkybox = TEXT("Milky Way");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyOverallBrightness = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyDayVisibility = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyDayIntensity = 0.85f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyNightIntensity = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning")
	bool bApplyStarrySkyNightSkyLightFill = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyDaySkyLightFillIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyNightSkyLightFillIntensity = 1.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning")
	bool bDisableStarrySkyLowerHemisphereBlack = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkySunLightIntensity = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyMoonLightIntensity = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyBrighterStarsIntensity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky|Tuning", meta = (ClampMin = 0.0f))
	float StarrySkyNebulaBrightness = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Starry Sky")
	TSoftClassPtr<AActor> StarrySkyActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sun")
	float SunYawDegrees = -35.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sun")
	float SunPitchOffsetDegrees = -90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sun", meta = (ClampMin = 0.0f))
	float DayLightIntensity = 8.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sun", meta = (ClampMin = 0.0f))
	float NightLightIntensity = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sun")
	FLinearColor DayLightColor = FLinearColor(1.0f, 0.95f, 0.86f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sun")
	FLinearColor NightLightColor = FLinearColor(0.32f, 0.42f, 0.78f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Moon")
	bool bDriveMoonLight = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Moon")
	bool bUseSunLightAsNightFillWhenNoMoon = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Moon", meta = (ClampMin = 0.0f))
	float MoonLightIntensity = 1.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sky", meta = (ClampMin = 0.0f))
	float DaySkyLightIntensity = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sky", meta = (ClampMin = 0.0f))
	float NightSkyLightIntensity = 0.65f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sky")
	bool bRecaptureSkyLight = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Sky", meta = (ClampMin = 1.0f, Units = "s"))
	float SkyRecaptureIntervalSeconds = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WU|World Time|Blend", meta = (ClampMin = 0.01f, ClampMax = 1.0f))
	float DawnDuskSoftness = 0.22f;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void ResolveLights();
	void ResolveStarrySkyActor();
	void ApplyStarrySkyPrototypeTuning();
	void ApplyStarrySkySkyLightFill(float WorldDayProgress);
	void DisableExternalDirectionalLights();
	void ApplyTimeOfDay(float WorldDayProgress, float DeltaSeconds);
	void ApplyStarrySkyTimeOfDay(float WorldDayProgress);
	float GetWorldDayProgress() const;
	float CalculateDayAlpha(float WorldDayProgress) const;

	UPROPERTY(Transient)
	TObjectPtr<AActor> StarrySkyActor;

	float SkyRecaptureAccumulatorSeconds = 0.0f;
	bool bLoggedMissingStarrySkyTimeProperty = false;
};
