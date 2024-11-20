// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PACurveCollector.h"
#include "NeuralNet/MLNNTrainintProfileSpecialOperation.h"
#include "PACurveReducerTrainingSpecialOp.generated.h"

/**
 * 
 */
UCLASS()
class PROCANIM_API UPACurveReducerTrainingSpecialOp : public UMLNNTrainingProfileSpecialOperation
{
	GENERATED_BODY()

public:
	virtual void Operation() const override;

	UPROPERTY(EditAnywhere)
	int32 NumCurves = 100;

	UPROPERTY(EditAnywhere)
	int32 NumKeys = 20;

	UPROPERTY(EditAnywhere)
	bool bResetData = true;

	UPROPERTY(EditAnywhere)
	bool AddToTestData = false;
};
