// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "PACurveCollector.h"
#include "UObject/Object.h"
#include "PASettings.generated.h"

/**
 * 
 */
UCLASS(config = ProcAnimSettings, DefaultConfig)
class PROCANIM_API UPASettings : public UObject
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, Config, Category = "Curve Collector")
	TSoftObjectPtr<UPACurveCollector> PACurveCollector = nullptr;
	
	UPROPERTY(EditAnywhere, Config, Category = "Curve Collector")
	bool VerifyCurvesOnAdd = true;
	
};
