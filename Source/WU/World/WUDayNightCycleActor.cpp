// Copyright Epic Games, Inc. All Rights Reserved.

#include "World/WUDayNightCycleActor.h"

#include "Backend/WUClientSessionSubsystem.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/GameInstance.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/UnrealType.h"
#include "WU.h"

namespace
{
	bool IsStarrySkyActor(const AActor* Actor)
	{
		if (!Actor)
		{
			return false;
		}

		const UClass* ActorClass = Actor->GetClass();
		if (!ActorClass)
		{
			return false;
		}

		const FString ClassName = ActorClass->GetName();
		const FString ClassPath = ActorClass->GetPathName();
		return ClassName.Contains(TEXT("BP_Starry_Sky"))
			|| ClassPath.Contains(TEXT("/Game/Starry_Sky/Blueprint/BP_Starry_Sky"));
	}

	FString NormalizePropertyName(FString Name)
	{
		Name.ReplaceInline(TEXT("_"), TEXT(""));
		Name.ReplaceInline(TEXT(" "), TEXT(""));
		Name.ReplaceInline(TEXT("'"), TEXT(""));
		return Name.ToLower();
	}

	bool PropertyMatchesAnyName(const FProperty* Property, const TArray<FName>& CandidateNames)
	{
		if (!Property)
		{
			return false;
		}

		const FString PropertyName = NormalizePropertyName(Property->GetName());
		const FString DisplayName = NormalizePropertyName(Property->GetMetaData(TEXT("DisplayName")));

		for (const FName& CandidateName : CandidateNames)
		{
			const FString Candidate = NormalizePropertyName(CandidateName.ToString());
			if (PropertyName == Candidate || DisplayName == Candidate)
			{
				return true;
			}
		}

		return false;
	}

	FProperty* FindProperty(UObject* Object, const TArray<FName>& CandidateNames)
	{
		if (!Object)
		{
			return nullptr;
		}

		for (TFieldIterator<FProperty> It(Object->GetClass()); It; ++It)
		{
			FProperty* Property = *It;
			if (!PropertyMatchesAnyName(Property, CandidateNames))
			{
				continue;
			}

			return Property;
		}

		return nullptr;
	}

	FProperty* FindNumericProperty(UObject* Object, const TArray<FName>& CandidateNames)
	{
		FProperty* Property = FindProperty(Object, CandidateNames);
		if (!Property)
		{
			return nullptr;
		}

		if (Property->IsA<FFloatProperty>()
			|| Property->IsA<FDoubleProperty>()
			|| Property->IsA<FIntProperty>())
		{
			return Property;
		}

		return nullptr;
	}

	bool SetNumericProperty(UObject* Object, const TArray<FName>& CandidateNames, double Value)
	{
		FProperty* Property = FindNumericProperty(Object, CandidateNames);
		if (!Property)
		{
			return false;
		}

		if (FFloatProperty* FloatProperty = CastField<FFloatProperty>(Property))
		{
			FloatProperty->SetPropertyValue_InContainer(Object, static_cast<float>(Value));
			return true;
		}

		if (FDoubleProperty* DoubleProperty = CastField<FDoubleProperty>(Property))
		{
			DoubleProperty->SetPropertyValue_InContainer(Object, Value);
			return true;
		}

		if (FIntProperty* IntProperty = CastField<FIntProperty>(Property))
		{
			IntProperty->SetPropertyValue_InContainer(Object, FMath::RoundToInt(Value));
			return true;
		}

		return false;
	}

	int64 FindEnumValueByDisplayName(const UEnum* Enum, const TArray<FString>& DesiredDisplayNames)
	{
		if (!Enum)
		{
			return INDEX_NONE;
		}

		for (int32 Pass = 0; Pass < 2; ++Pass)
		{
			for (int32 Index = 0; Index < Enum->NumEnums(); ++Index)
			{
				if (Enum->ContainsExistingMax() && Index == Enum->NumEnums() - 1)
				{
					continue;
				}

				const FString NormalizedDisplayName = NormalizePropertyName(Enum->GetDisplayNameTextByIndex(Index).ToString());
				const FString NormalizedName = NormalizePropertyName(Enum->GetNameStringByIndex(Index));

				for (const FString& DesiredDisplayName : DesiredDisplayNames)
				{
					const FString NormalizedDesiredName = NormalizePropertyName(DesiredDisplayName);
					const bool bMatches = Pass == 0
						? (NormalizedDisplayName == NormalizedDesiredName || NormalizedName == NormalizedDesiredName)
						: (NormalizedDisplayName.Contains(NormalizedDesiredName) || NormalizedName.Contains(NormalizedDesiredName));

					if (bMatches)
					{
						return Enum->GetValueByIndex(Index);
					}
				}
			}
		}

		return INDEX_NONE;
	}

	bool SetEnumProperty(UObject* Object, const TArray<FName>& CandidateNames, const TArray<FString>& DesiredDisplayNames)
	{
		FProperty* Property = FindProperty(Object, CandidateNames);
		if (!Property)
		{
			return false;
		}

		if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			const int64 EnumValue = FindEnumValueByDisplayName(EnumProperty->GetEnum(), DesiredDisplayNames);
			if (EnumValue == INDEX_NONE)
			{
				return false;
			}

			EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(EnumProperty->ContainerPtrToValuePtr<void>(Object), EnumValue);
			return true;
		}

		if (FByteProperty* ByteProperty = CastField<FByteProperty>(Property))
		{
			const int64 EnumValue = FindEnumValueByDisplayName(ByteProperty->Enum, DesiredDisplayNames);
			if (EnumValue == INDEX_NONE)
			{
				return false;
			}

			ByteProperty->SetPropertyValue_InContainer(Object, static_cast<uint8>(EnumValue));
			return true;
		}

		return false;
	}
}

AWUDayNightCycleActor::AWUDayNightCycleActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
	StarrySkyActorClass = TSoftClassPtr<AActor>(FSoftObjectPath(TEXT("/Game/Starry_Sky/Blueprint/BP_Starry_Sky.BP_Starry_Sky_C")));
}

void AWUDayNightCycleActor::BeginPlay()
{
	Super::BeginPlay();

	ResolveStarrySkyActor();
	ResolveLights();

	if (bSyncServerTimeOnBeginPlay)
	{
		RefreshServerWorldTime();
	}

	ApplyTimeOfDay(GetWorldDayProgress(), 0.0f);
}

void AWUDayNightCycleActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ApplyTimeOfDay(GetWorldDayProgress(), DeltaSeconds);
}

void AWUDayNightCycleActor::RefreshServerWorldTime()
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			Session->SyncWorldTime();
		}
	}
}

void AWUDayNightCycleActor::ResolveLights()
{
	if (!bAutoFindLights)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* DirectionalLight = *It;
		if (!DirectionalLight)
		{
			continue;
		}

		if (!SunLight)
		{
			SunLight = DirectionalLight;
		}
		else if (!MoonLight && DirectionalLight != SunLight)
		{
			MoonLight = DirectionalLight;
			break;
		}
	}

	if (!SkyLight)
	{
		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			SkyLight = *It;
			break;
		}
	}
}

void AWUDayNightCycleActor::ResolveStarrySkyActor()
{
	StarrySkyActor = nullptr;
	bLoggedMissingStarrySkyTimeProperty = false;

	if (!bUseStarrySkyActorWhenAvailable)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (IsStarrySkyActor(Actor))
		{
			StarrySkyActor = Actor;
			ApplyStarrySkyPrototypeTuning();
			DisableExternalDirectionalLights();
			return;
		}
	}

	if (!bSpawnStarrySkyActorWhenAvailable || StarrySkyActorClass.IsNull())
	{
		return;
	}

	UClass* LoadedStarrySkyClass = StarrySkyActorClass.LoadSynchronous();
	if (!LoadedStarrySkyClass || !LoadedStarrySkyClass->IsChildOf(AActor::StaticClass()))
	{
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, FVector::ZeroVector);
	StarrySkyActor = World->SpawnActorDeferred<AActor>(
		LoadedStarrySkyClass,
		SpawnTransform,
		this,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (StarrySkyActor)
	{
		ApplyStarrySkyPrototypeTuning();
		StarrySkyActor->FinishSpawning(SpawnTransform);
		DisableExternalDirectionalLights();
	}
}

void AWUDayNightCycleActor::ApplyStarrySkyPrototypeTuning()
{
	AActor* StarrySky = StarrySkyActor.Get();
	if (!StarrySky || !bApplyStarrySkyPrototypeTuning)
	{
		return;
	}

	SetEnumProperty(
		StarrySky,
		{ TEXT("Select"), TEXT("Select "), TEXT("Select Skybox") },
		{ StarrySkyPreferredSkybox });

	SetNumericProperty(StarrySky, { TEXT("Overall Brightness") }, StarrySkyOverallBrightness);
	SetNumericProperty(StarrySky, { TEXT("Day visibility"), TEXT("Day Visibility"), TEXT("Day_visibility") }, StarrySkyDayVisibility);
	SetNumericProperty(StarrySky, { TEXT("Day Intensity") }, StarrySkyDayIntensity);
	SetNumericProperty(StarrySky, { TEXT("Night Intensity") }, StarrySkyNightIntensity);
	SetNumericProperty(StarrySky, { TEXT("SunLight intensity"), TEXT("Sun Light Intensity") }, StarrySkySunLightIntensity);
	SetNumericProperty(StarrySky, { TEXT("MoonLight intensity"), TEXT("Moon Light Intensity") }, StarrySkyMoonLightIntensity);
	SetNumericProperty(StarrySky, { TEXT("Brighter stars intensity") }, StarrySkyBrighterStarsIntensity);
	SetNumericProperty(StarrySky, { TEXT("Nebula brightness") }, StarrySkyNebulaBrightness);
}

void AWUDayNightCycleActor::ApplyStarrySkySkyLightFill(float WorldDayProgress)
{
	AActor* StarrySky = StarrySkyActor.Get();
	if (!StarrySky || !bApplyStarrySkyNightSkyLightFill)
	{
		return;
	}

	const float DayAlpha = CalculateDayAlpha(FMath::Frac(WorldDayProgress));
	const float FillIntensity = FMath::Lerp(StarrySkyNightSkyLightFillIntensity, StarrySkyDaySkyLightFillIntensity, DayAlpha);

	TArray<USkyLightComponent*> SkyLightComponents;
	StarrySky->GetComponents(SkyLightComponents);
	for (USkyLightComponent* SkyLightComponent : SkyLightComponents)
	{
		if (!SkyLightComponent)
		{
			continue;
		}

		SkyLightComponent->SetVisibility(true, true);
		SkyLightComponent->SetIntensity(FillIntensity);
		SkyLightComponent->SetLowerHemisphereColor(FLinearColor(0.09f, 0.10f, 0.14f, 1.0f));
		if (bDisableStarrySkyLowerHemisphereBlack)
		{
			SkyLightComponent->bLowerHemisphereIsBlack = false;
		}
	}
}

void AWUDayNightCycleActor::DisableExternalDirectionalLights()
{
	if (!bDisableExternalDirectionalLightsWhenUsingStarrySky || !StarrySkyActor)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		ADirectionalLight* DirectionalLight = *It;
		if (!DirectionalLight || DirectionalLight == StarrySkyActor.Get() || DirectionalLight->GetOwner() == StarrySkyActor.Get())
		{
			continue;
		}

		if (ULightComponent* LightComponent = DirectionalLight->GetLightComponent())
		{
			LightComponent->SetVisibility(false, true);
			LightComponent->SetIntensity(0.0f);
		}
	}
}

void AWUDayNightCycleActor::ApplyTimeOfDay(float WorldDayProgress, float DeltaSeconds)
{
	const float WrappedProgress = FMath::Frac(WorldDayProgress);

	if (StarrySkyActor)
	{
		ApplyStarrySkyTimeOfDay(WrappedProgress);
		ApplyStarrySkySkyLightFill(WrappedProgress);
		if (bDisableNativeLightDrivingWhenUsingStarrySky)
		{
			return;
		}
	}

	const float SunPitch = SunPitchOffsetDegrees + WrappedProgress * 360.0f;
	const float DayAlpha = CalculateDayAlpha(WrappedProgress);

	if (SunLight)
	{
		const bool bUseSunAsNightFill = bDriveMoonLight && bUseSunLightAsNightFillWhenNoMoon && !MoonLight;
		const float SunFillPitch = bUseSunAsNightFill
			? FMath::Lerp(SunPitch + 180.0f, SunPitch, DayAlpha)
			: SunPitch;
		SunLight->SetActorRotation(FRotator(SunFillPitch, SunYawDegrees, 0.0f));

		if (ULightComponent* LightComponent = SunLight->GetLightComponent())
		{
			const float NightIntensity = bUseSunAsNightFill ? MoonLightIntensity : NightLightIntensity;
			LightComponent->SetIntensity(FMath::Lerp(NightIntensity, DayLightIntensity, DayAlpha));
			LightComponent->SetLightColor(FLinearColor::LerpUsingHSV(NightLightColor, DayLightColor, DayAlpha));
		}
	}

	if (bDriveMoonLight && MoonLight)
	{
		MoonLight->SetActorRotation(FRotator(SunPitch + 180.0f, SunYawDegrees + 180.0f, 0.0f));

		if (ULightComponent* LightComponent = MoonLight->GetLightComponent())
		{
			LightComponent->SetIntensity(FMath::Lerp(MoonLightIntensity, 0.0f, DayAlpha));
			LightComponent->SetLightColor(NightLightColor);
		}
	}

	if (SkyLight)
	{
		if (USkyLightComponent* LightComponent = SkyLight->GetLightComponent())
		{
			LightComponent->SetIntensity(FMath::Lerp(NightSkyLightIntensity, DaySkyLightIntensity, DayAlpha));

			if (bRecaptureSkyLight && SkyRecaptureIntervalSeconds > 0.0f && DeltaSeconds > 0.0f)
			{
				SkyRecaptureAccumulatorSeconds += DeltaSeconds;
				if (SkyRecaptureAccumulatorSeconds >= SkyRecaptureIntervalSeconds)
				{
					SkyRecaptureAccumulatorSeconds = 0.0f;
					LightComponent->RecaptureSky();
				}
			}
		}
	}
}

void AWUDayNightCycleActor::ApplyStarrySkyTimeOfDay(float WorldDayProgress)
{
	AActor* StarrySky = StarrySkyActor.Get();
	if (!StarrySky)
	{
		ResolveStarrySkyActor();
		StarrySky = StarrySkyActor.Get();
	}

	if (!StarrySky)
	{
		return;
	}

	static const TArray<FName> TimeOfDayPropertyNames =
	{
		TEXT("Time of day"),
		TEXT("TimeOfDay"),
		TEXT("Time_of_day"),
		TEXT("Time")
	};

	static const TArray<FName> EarthRotationSpeedPropertyNames =
	{
		TEXT("Earth's rotation speed"),
		TEXT("EarthsRotationSpeed"),
		TEXT("Earth_Rotation_Speed"),
		TEXT("EarthRotationSpeed")
	};

	const double HoursOfDay = static_cast<double>(FMath::Frac(WorldDayProgress)) * 24.0;
	const bool bSetTime = SetNumericProperty(StarrySky, TimeOfDayPropertyNames, HoursOfDay);

	if (!bSetTime && !bLoggedMissingStarrySkyTimeProperty)
	{
		bLoggedMissingStarrySkyTimeProperty = true;
		UE_LOG(LogWU, Warning, TEXT("WU day/night could not find the Starry Sky 'Time of day' property on %s."), *StarrySky->GetName());
	}

	if (bFreezeStarrySkyInternalClock)
	{
		SetNumericProperty(StarrySky, EarthRotationSpeedPropertyNames, 0.0);
	}
}

float AWUDayNightCycleActor::GetWorldDayProgress() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const UWUClientSessionSubsystem* Session = GameInstance->GetSubsystem<UWUClientSessionSubsystem>())
		{
			return Session->GetEstimatedWorldDayProgress();
		}
	}

	const double SecondsOfDay = FDateTime::Now().GetTimeOfDay().GetTotalSeconds();
	return static_cast<float>(SecondsOfDay / 86400.0);
}

float AWUDayNightCycleActor::CalculateDayAlpha(float WorldDayProgress) const
{
	const float SolarSine = FMath::Sin((WorldDayProgress - 0.25f) * UE_TWO_PI);
	const float Softness = FMath::Max(0.01f, DawnDuskSoftness);
	const float Blend = FMath::Clamp((SolarSine + Softness) / (Softness * 2.0f), 0.0f, 1.0f);
	return Blend * Blend * (3.0f - 2.0f * Blend);
}
