// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"

MatrixXf UPACurveReducerDataProcessor::PreprocessInput(const FRichCurve& InputCurve) const
{
	const FCurveReducerCurveParams CurveParams(InputCurve);

	const double Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	const int32 LeadSampleIndex = FMath::DivideAndRoundDown(WindowSize, 2);
	const float WingLength = Interval * float(LeadSampleIndex);

	const int32 SampleCount = ((CurveParams.EndTime - CurveParams.StartTime) / Interval) + 1;
	const int32 SampleSize = (WindowSize - 1) * 3;
	MatrixXf Result(SampleSize, SampleCount);
	TArray<float> EvalWindow;
	for(int32 c = 0; c < SampleCount; c++)
	{
		const float t = CurveParams.StartTime + c * Interval;
		EvalWindow.Empty();
		float WindowTime = t - WingLength;
		for(int32 i = 0; i < WindowSize; i++)
		{
			const float Eval = InputCurve.Eval(WindowTime);
			EvalWindow.Add(Eval);
			WindowTime += Interval;
		}

		const float LeadValue = EvalWindow[LeadSampleIndex];
		
		// get the largest absolute value in the window
		float MaxAbsValue = FLT_EPSILON;
		for(const float Value : EvalWindow)
        {
	        const float AbsValue = FMath::Abs(Value - LeadValue);
            MaxAbsValue = FMath::Max(AbsValue, MaxAbsValue);
        }
		
		EvalWindow.RemoveAt(LeadSampleIndex);
		
		TArray<float> Normals;
		TArray<float> SquaredNormals;
		for(float& Value : EvalWindow)
        {
            Value = (Value - LeadValue);
			const float Normal = Value / MaxAbsValue;
			Normals.Add(Normal);
			SquaredNormals.Add(Normal * Normal);
        }

		TArray<float> FinalData = EvalWindow;
		FinalData.Append(Normals);
		FinalData.Append(SquaredNormals);
		
		Result.col(c) = Map<VectorXf>(FinalData.GetData(), SampleSize);
	}
	return Result;
}

MatrixXf UPACurveReducerDataProcessor::CalculateLabels(const FRichCurve& InputCurve)
{
	const FCurveReducerCurveParams CurveParams(InputCurve);
	const double Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	
	const int32 SampleCount = ((CurveParams.EndTime - CurveParams.StartTime) / Interval) + 1;
	MatrixXf Result = MatrixXf::Zero(1, SampleCount);

	for (const auto Key : InputCurve.Keys)
	{
		const float Time = Key.Time;
		const int32 Index = FMath::RoundToInt32((Time - CurveParams.StartTime) / Interval);
		Result(Index) = 1.f;
	}
	return Result;
}

void UPACurveReducerDataProcessor::PreProcessTrainingData(const TArray<FRichCurve>& InputCurves, MatrixXf& OutData, MatrixXf& OutLabels) const
{
	OutData = MatrixXf();
	OutLabels = MatrixXf();
	for(const FRichCurve &Curve : InputCurves)
	{
		MatrixXf Data = PreprocessInput(Curve);
		MatrixXf Labels = CalculateLabels(Curve);

		check(Data.cols() == Labels.cols())

		const int32 ColCount = OutData.cols() + Data.cols();
		
		MatrixXf ResultInput = MatrixXf(Data.rows(), ColCount);
		ResultInput << OutData, Data;
		OutData = ResultInput;
	
		MatrixXf ResultLabels = MatrixXf(Labels.rows(), ColCount);
		ResultLabels << OutLabels, Labels;
		OutLabels = ResultLabels;
	}
}

FRichCurve UPACurveReducerDataProcessor::PostProcessOutput(const MatrixXf& OutputData) const
{
	return FRichCurve();
}
