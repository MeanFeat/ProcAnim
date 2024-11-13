// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PASettings.generated.h"

class UPACurveCollector;
class UMLNeuralNet;
/**
 * 
 */
UCLASS(config = ProcAnimSettings, DefaultConfig)
class PROCANIM_API UPASettings : public UObject
{
	GENERATED_BODY()

public:

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditAnywhere, Config, Category = "Curve Reducer")
	TSoftObjectPtr<UMLNeuralNet> PACurveReducerNeuralNet = nullptr;

	UPROPERTY(EditAnywhere, Config, Category = "Curve Reducer|Collector")
	TSoftObjectPtr<UPACurveCollector> PACurveCollector = nullptr;
	
	UPROPERTY(EditAnywhere, Config, Category = "Curve Reducer|Collector")
	bool VerifyCurvesOnAdd = true;

	UPROPERTY(EditAnywhere, Config, Category = "Animation Data")
	float DefaultFPS = 30.f;

	// todo: should we ust use UAnimationSettings::DefaultFrameRate?
	UPROPERTY(VisibleAnywhere, Category = "Animation Data")
	float DefaultFrameInterval = 1.f/DefaultFPS;
	
};
