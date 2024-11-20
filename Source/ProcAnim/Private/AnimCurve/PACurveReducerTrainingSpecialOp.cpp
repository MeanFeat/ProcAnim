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
		
#if 0
		TArray<FRichCurve> Curves = CurveCollector->Curves;
#else
		TArray<FRichCurve> Curves;

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
				Value += FMath::RandRange(-10.0f, 10.0f);
				Time += FMath::RandRange(0.03f, 0.2f);
				Time = FMath::RoundToFloat(Time * 30.0f) / 30.0f;
            }
			Curves.Add(CrazyLongCurve);
			SlowTask.EnterProgressFrame(1);
		}
#endif
		Eigen::MatrixXf Data, Labels;
		CurveReducer->PreProcessTrainingData(Curves, Data, Labels);
		if (AddToTestData)
		{
			if (TestData)
			{
				TestData->AppendData(Data, Labels);
			}
		}
		else
		{
			TrainingData->AppendData(Data, Labels);
		}
	}
}

#undef LOCTEXT_NAMESPACE