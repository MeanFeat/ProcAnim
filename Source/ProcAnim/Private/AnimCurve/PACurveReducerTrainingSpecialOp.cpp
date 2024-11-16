// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveReducerTrainingSpecialOp.h"
#include "PACurveCollector.h"
#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "NeuralNet/MLNNTrainingProfile.h"

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
	TrainingData->ResetData();
	if(const UPACurveReducerDataProcessor* CurveReducer = Cast<UPACurveReducerDataProcessor>(NeuralNet->DataProcessor))
	{
		
#if 1
		TArray<FRichCurve> Curves = CurveCollector->Curves;
#else
		TArray<FRichCurve> Curves;
		
		for(int32 i = 0; i < 10; i++)
		{
			FRichCurve CrazyLongCurve;
			float Time = 0.0f;
			float Value = 0.0f;
			for(int32 j = 0; j < 5000; j++)
            {
                CrazyLongCurve.AddKey(Time, Value);
				Value += FMath::RandRange(-10.0f, 10.0f);
				Time += FMath::RandRange(0.03f, 0.2f);
				Time = FMath::RoundToFloat(Time * 30.0f) / 30.0f;
            }
			Curves.Add(CrazyLongCurve);
		}
#endif
		
		Eigen::MatrixXf Data, Labels;
		CurveReducer->PreProcessTrainingData(Curves, Data, Labels);
		TrainingData->AppendData(Data, Labels);
	}
}

