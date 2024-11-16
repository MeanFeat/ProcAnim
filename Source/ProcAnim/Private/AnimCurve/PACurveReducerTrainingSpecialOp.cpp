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
	if(const UPACurveReducerDataProcessor* CurveReducer = NeuralNet->DataProcessorClass->GetDefaultObject<UPACurveReducerDataProcessor>())
	{
		const TArray<FRichCurve> Curves = CurveCollector->Curves;
		
		Eigen::MatrixXf Data, Labels;
		CurveReducer->PreProcessTrainingData(Curves, Data, Labels);
		TrainingData->AppendData(Data, Labels);
	}
}

