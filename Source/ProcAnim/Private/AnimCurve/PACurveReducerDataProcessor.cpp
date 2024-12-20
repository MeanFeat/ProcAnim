// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "Common/MLESLibrary.h"

#define LOCTEXT_NAMESPACE "UPACurveReducerDataProcessor"

int32 UPACurveReducerDataProcessor::OutputSize = 1;

MatrixXf UPACurveReducerDataProcessor::PreprocessInput(const FRichCurve& InputCurve) const
{
	const FCurveReducerCurveParams CurveParams(InputCurve);

	const double Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	const int32 LeadSampleIndex = FMath::DivideAndRoundDown(WindowSize, 2);
	const float WingLength = Interval * float(LeadSampleIndex);

	const int32 SampleCount = ((CurveParams.EndTime - CurveParams.StartTime) / Interval) + 1;
	const int32 SampleSize = ((WindowSize - 1) * 3);
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
			SquaredNormals.Add((Normal * Normal * 2.f) - 1.f);
        }

		TArray<float> Deltas;
		for(int32 i = 0; i < EvalWindow.Num(); i++)
		{
			float Delta = i > 0	 ? EvalWindow[i] - EvalWindow[i - 1] : 0.f;
			Delta += i < EvalWindow.Num() - 1 ? EvalWindow[i + 1] - EvalWindow[i] : 0.f;
			if (i > 0 && i < EvalWindow.Num() - 1)
            {
                Delta /= 2.f;
            }
			Deltas.Add(Delta);
		}
		
		TArray<float> FinalData = Deltas;
		FinalData.Append(Normals);
		FinalData.Append(SquaredNormals);
		
		check(FinalData.Num() == SampleSize)
		Result.col(c) = Map<VectorXf>(FinalData.GetData(), SampleSize);
	}
	return Result;
}

MatrixXf UPACurveReducerDataProcessor::CalculateLabels(const FRichCurve& InputCurve) const
{
	const FCurveReducerCurveParams CurveParams(InputCurve);
	const double Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	
	const int32 SampleCount = ((CurveParams.EndTime - CurveParams.StartTime) / Interval) + 1;
	MatrixXf Result = MatrixXf::Zero(OutputSize, SampleCount);
	//Result.row(0) = VectorXf::Ones(SampleCount) * -1.f;
	for (const FRichCurveKey Key : InputCurve.Keys)
	{
		const float Time = Key.Time;
		const int32 Index = FMath::RoundToInt32((Time - CurveParams.StartTime) / Interval);
		VectorXf OutData(OutputSize);
		ConvertKeyToData(Key, OutData);
		Result.col(Index) = OutData;
	}
	return Result;
}

void UPACurveReducerDataProcessor::ConvertKeyToData(const FRichCurveKey& Key, VectorXf& OutData) const
{
	int i = 0;
	OutData(i++) = 1.f;
	/*const float CompressedArriveTangent = Key.ArriveTangent * TangentCompression;
	OutData(i++) = CompressedArriveTangent;
	OutData(i++) = Key.ArriveTangentWeight;
	const float CompressedLeaveTangent = Key.LeaveTangent * TangentCompression;
	OutData(i++) = CompressedLeaveTangent;
	OutData(i++) = Key.LeaveTangentWeight;*/
}

void UPACurveReducerDataProcessor::ConvertDataToKey(const VectorXf& Data, FRichCurveKey& OutKey) const
{
	OutKey.InterpMode = RCIM_Cubic;
	int i = 1;
	OutKey.ArriveTangent = 0.f;// Data(i++) / TangentCompression;
	OutKey.ArriveTangentWeight = 0.f;// Data(i++);
	OutKey.LeaveTangent =0.f;// Data(i++) / TangentCompression;
	OutKey.LeaveTangentWeight =0.f;// Data(i++);
}

void UPACurveReducerDataProcessor::PreProcessTrainingData(const TArray<FRichCurve>& InputCurves, MatrixXf& OutData, MatrixXf& OutLabels) const
{
	OutData = MatrixXf();
	OutLabels = MatrixXf();
	FScopedSlowTask SlowTask(InputCurves.Num(), LOCTEXT("Generating Training Data", "Generating Training Data"));
	SlowTask.MakeDialog();
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
		SlowTask.EnterProgressFrame(1);
	}
}

FRichCurve UPACurveReducerDataProcessor::PostProcessOutput(const MatrixXf& OutputData) const
{
	return FRichCurve();
}

#undef LOCTEXT_NAMESPACE