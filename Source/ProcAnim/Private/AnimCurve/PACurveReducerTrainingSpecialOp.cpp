// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveReducerTrainingSpecialOp.h"
#include "PACurveCollector.h"
#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "NeuralNet/MLNNTrainingProfile.h"

#define LOCTEXT_NAMESPACE "UPACurveReducerTrainingSpecialOp"

void UPACurveReducerTrainingSpecialOp::Operation() const
{
	const UMLNNTrainingProfile* TP = Cast<UMLNNTrainingProfile>(GetOuter());
	const UPACurveCollector* CurveCollector = FProcAnimModule::PASettings->PACurveCollector.LoadSynchronous();
	UMLNNTrainingData* TrainingData = TP->TrainingData.LoadSynchronous();
	
	const UMLNeuralNet* NeuralNet = TP->NeuralNetwork.LoadSynchronous();
	if(!TP ||!CurveCollector || !TrainingData || !NeuralNet)
	{
		return;
	}
	UMLNNTrainingData* TestData = TP->TestData.LoadSynchronous();
	if (bResetData)
    {
		if (AddToTestData)
		{
			TestData->ResetData();
		}
		else
		{
			TrainingData->ResetData();
		}
    }
	if(const UPACurveReducerDataProcessor* CurveReducer = Cast<UPACurveReducerDataProcessor>(NeuralNet->DataProcessor))
	{
		TArray<FRichCurve> Curves;
		if (bGenerateCurves)
		{
			FScopedSlowTask SlowTask(NumCurves, LOCTEXT("Generating Training Data", "Generating Training Data"));
			SlowTask.MakeDialog();
			for(int32 i = 0; i < NumCurves; i++)
			{
				FRichCurve CrazyLongCurve;
				float Time = 0.0f;
				float Value = 0.0f;
				for(int32 j = 0; j < NumKeys; j++)
				{
					CrazyLongCurve.AddKey(Time, Value);
					Value += FMath::RandRange(-20.0f, 20.0f);
					FRichCurveKey &Key = CrazyLongCurve.GetKey(CrazyLongCurve.FindKey(Time));
					Key.InterpMode = ERichCurveInterpMode::RCIM_Cubic;
					Key.TangentMode = ERichCurveTangentMode::RCTM_Auto;
					Key.ArriveTangent = FMath::RandRange(-180.0f, 180.0f);
					Key.LeaveTangent = -Key.ArriveTangent;
					Key.TangentWeightMode = ERichCurveTangentWeightMode::RCTWM_WeightedNone;
					Key.TangentWeightMode = ERichCurveTangentWeightMode::RCTWM_WeightedNone;
					Time += FMath::RandRange(0.03333f, 1.f);
					Time = FMath::RoundToFloat(Time * 30.0f) / 30.0f;
				}
				CrazyLongCurve.AutoSetTangents();
				Curves.Add(CrazyLongCurve);
				SlowTask.EnterProgressFrame(1);
			}
		}
		else
		{
			Curves = CurveCollector->Curves;
		}
		Eigen::MatrixXf Data, Labels;
		CurveReducer->PreProcessTrainingData(Curves, Data, Labels);
		if (AddToTestData)
		{
			if (TestData)
			{
				TestData->AppendData(Data, Labels);
				TestData->Modify();
			}
		}
		else
		{
			TrainingData->AppendData(Data, Labels);
			TrainingData->Modify();
		}
	}
}

#undef LOCTEXT_NAMESPACE
