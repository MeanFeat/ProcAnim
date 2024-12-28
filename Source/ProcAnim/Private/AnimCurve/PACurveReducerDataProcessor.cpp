// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "Common/MLESLibrary.h"

#define LOCTEXT_NAMESPACE "UPACurveReducerDataProcessor"

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
                Delta *= 0.5f;
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

MatrixXf UPAKeyDetectionDataProcessor::CalculateLabels(const FRichCurve& InputCurve) const
{
	const FCurveReducerCurveParams CurveParams(InputCurve);
	const double Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	
	const int32 SampleCount = ((CurveParams.EndTime - CurveParams.StartTime) / Interval) + 1;
	MatrixXf Result = MatrixXf::Zero(GetOutputSize(), SampleCount);
	for (const FRichCurveKey Key : InputCurve.Keys)
	{
		const float Time = Key.Time;
		const int32 Index = FMath::RoundToInt32((Time - CurveParams.StartTime) / Interval);
		Result(0, Index) = 1.f;
	}
	return Result;
}

MatrixXf UPATangentApproximationProcessor::CalculateLabels(const FRichCurve& InputCurve) const
{
	MatrixXf Result = MatrixXf::Zero(GetOutputSize(), InputCurve.GetNumKeys());
	for (int32 i = 0; i < InputCurve.GetNumKeys(); i++)
    {
        const FRichCurveKey Key = InputCurve.Keys[i];
        VectorXf OutData(GetOutputSize());
        ConvertKeyToData(Key, OutData);
        Result.col(i) = OutData;
    }
	UE_LOG(LogTemp, Warning, TEXT("Labels: \n%s"), *FMLESLibrary::EigenMatrixToString(Result));
	return Result;
}

void UPATangentApproximationProcessor::ConvertKeyToData(const FRichCurveKey& Key, VectorXf& OutData) const
{
	int i = 0;
	const float CompressedArriveTangent = Key.ArriveTangent * TangentCompression;
	OutData(i++) = CompressedArriveTangent;
	OutData(i++) = Key.ArriveTangentWeight;
	const float CompressedLeaveTangent = Key.LeaveTangent * TangentCompression;
	OutData(i++) = CompressedLeaveTangent;
	OutData(i++) = Key.LeaveTangentWeight;
}

void UPATangentApproximationProcessor::ConvertDataToKey(const VectorXf& Data, FRichCurveKey& OutKey) const
{
	OutKey.InterpMode = RCIM_Cubic;
	int i = 0;
	OutKey.ArriveTangent = Data(i++) / TangentCompression;
	OutKey.ArriveTangentWeight = Data(i++);
	OutKey.LeaveTangent = Data(i++) / TangentCompression;
	OutKey.LeaveTangentWeight = Data(i++);
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

void UPATangentApproximationProcessor::PreProcessTrainingData(const TArray<FRichCurve>& InputCurves, MatrixXf& OutData, MatrixXf& OutLabels) const
{
	OutData = MatrixXf();
	OutLabels = MatrixXf();
	FScopedSlowTask SlowTask(InputCurves.Num(), LOCTEXT("Generating Training Data", "Generating Training Data"));
	SlowTask.MakeDialog();
	for(const FRichCurve &Curve : InputCurves)
	{
		const MatrixXf RawData = PreprocessInput(Curve);
		const MatrixXf KeyLabels = CalculateLabels(Curve);

		TArray<int32> KeyIndexes;
		for (const FRichCurveKey Key : Curve.Keys)
		{
			KeyIndexes.Add( int32(Key.Time / FProcAnimModule::PASettings->DefaultFrameInterval));
		}

		MatrixXf KeyData = MatrixXf::Zero(RawData.rows(), Curve.Keys.Num());
		int32 CurrentColIndex = 0;
		for (int32 i = 0; i < RawData.cols(); i++)
		{
			if (KeyIndexes.Contains(i))
			{
				KeyData.col(CurrentColIndex) = RawData.col(i);
				CurrentColIndex++;
			}
		}
		
		check(KeyData.cols() == KeyLabels.cols())

		const int32 ColCount = OutData.cols() + KeyData.cols();
		
		MatrixXf ResultInput = MatrixXf(KeyData.rows(), ColCount);
		ResultInput << OutData, KeyData;
		OutData = ResultInput;
	
		MatrixXf ResultLabels = MatrixXf(KeyLabels.rows(), ColCount);
		ResultLabels << OutLabels, KeyLabels;
		OutLabels = ResultLabels;
		SlowTask.EnterProgressFrame(1);
	}
}

FRichCurve UPACurveReducerDataProcessor::PostProcessOutput(const MatrixXf& OutputData) const
{
	return FRichCurve();
}

#undef LOCTEXT_NAMESPACE