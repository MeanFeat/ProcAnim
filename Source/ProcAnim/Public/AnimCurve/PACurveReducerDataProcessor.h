// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NeuralNet/MLNeuralNetDataProcessor.h"
#include "PACurveReducerDataProcessor.generated.h"

using namespace Eigen;

class FCurveReducerCurveParams
{
public:
	
	FCurveReducerCurveParams(const FRichCurve &InputCurve)
		: Curve(InputCurve)
    {
		StartFrame = InputCurve.Keys[0];
		EndFrame = InputCurve.Keys.Last();

		StartTime = StartFrame.Time;
		EndTime = EndFrame.Time;
    }

	FRichCurve Curve;
	
	FRichCurveKey StartFrame;
	FRichCurveKey EndFrame;

	float StartTime;
	float EndTime;
};

UCLASS(Abstract)
class PROCANIM_API UPACurveReducerDataProcessor : public UMLNeuralNetDataProcessor, public TMLNeuralNetDataProcessor<FRichCurve, FRichCurve>
{
	GENERATED_BODY()

public:
	
	virtual MatrixXf PreprocessInput(const FRichCurve &InputCurve) const override;

	virtual void PreProcessTrainingData(const TArray<FRichCurve> &InputCurves, MatrixXf &OutData, MatrixXf &OutLabels) const override;

	virtual FRichCurve PostProcessOutput(const MatrixXf &OutputData) const override;

	virtual MatrixXf CalculateLabels(const FRichCurve& InputCurve) const {
		return MatrixXf();
	}

	virtual int32 GetOutputSize() const
	{
		return 0;
	}
	
	// uneven numbers only; center index is removed
	UPROPERTY(EditAnywhere)
	int32 WindowSize = 9;

};


UCLASS()
class PROCANIM_API UPAKeyDetectionDataProcessor : public UPACurveReducerDataProcessor
{
	GENERATED_BODY()

public:

	virtual MatrixXf CalculateLabels(const FRichCurve& InputCurve) const override;
	
	virtual int32 GetOutputSize() const override
	{
		return 1;
	}
};


UCLASS()
class PROCANIM_API UPATangentApproximationProcessor : public UPACurveReducerDataProcessor
{
	GENERATED_BODY()

public:
	
	virtual MatrixXf CalculateLabels(const FRichCurve& InputCurve) const override;

	virtual void PreProcessTrainingData(const TArray<FRichCurve> &InputCurves, MatrixXf &OutData, MatrixXf &OutLabels) const override;
	
	virtual int32 GetOutputSize() const  override
	{
		return 4;
	}

	void ConvertKeyToData(const FRichCurveKey &Key, VectorXf &OutData) const;

	void ConvertDataToKey(const VectorXf &Data, FRichCurveKey &OutKey) const;
	
	UPROPERTY(EditAnywhere)
	float TangentCompression = 0.025f;
};