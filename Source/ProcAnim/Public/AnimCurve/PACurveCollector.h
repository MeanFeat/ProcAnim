// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "PACurveCollector.generated.h"

/**
 * 
 */
UCLASS()
class PROCANIM_API UPACurveCollector : public UDataAsset
{
	GENERATED_BODY()

public: 

	UPROPERTY(EditAnywhere, Category = "Curves")
	TArray<FRichCurve> Curves;
};
