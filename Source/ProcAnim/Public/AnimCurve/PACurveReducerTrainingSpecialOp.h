// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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
};
