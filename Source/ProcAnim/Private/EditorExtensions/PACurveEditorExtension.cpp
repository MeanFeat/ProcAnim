﻿// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveEditorExtension.h"
#include "PACurveCollector.h"
#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "RichCurveEditorModel.h"
#include "Animation/AnimationSettings.h"
#include "Common/MLESLibrary.h"
#include "NeuralNet/MLNeuralNet.h"

#define LOCTEXT_NAMESPACE "FPACurveEditorExtension"

class UPACurveReducerDataProcessor;

TWeakPtr<FCurveEditor> FPACurveEditorExtension::WeakCurveEditor = nullptr;

FPACurveEditorExtension::~FPACurveEditorExtension()
{
}

void FPACurveEditorExtension::BindCommands(TSharedRef<FUICommandList> CommandBindings)
{
}

TSharedRef<ICurveEditorExtension> FPACurveEditorExtension::CreateCurveEditorExtension(TWeakPtr<FCurveEditor> InCurveEditor)
{
	TSharedRef<FPACurveEditorExtension> EditorExtension = MakeShared<FPACurveEditorExtension>(InCurveEditor);
	WeakCurveEditor = InCurveEditor;
	return EditorExtension;
}

TSharedRef<FExtender> FPACurveEditorExtension::ExtendCurveEditorToolbarMenu(const TSharedRef<FUICommandList> CommandList)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension("Filters",
	EExtensionHook::After,
	CommandList,
	FToolBarExtensionDelegate::CreateStatic(&FPACurveEditorExtension::FillToolbarTools));
	return Extender;
}

void FPACurveEditorExtension::FillToolbarTools(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("ProcAnimCurveEditor");
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateStatic(&FPACurveEditorExtension::GetMenuContent),
		TAttribute<FText>(),
		LOCTEXT("ProcAnimSequencerToolsTooltip", "Open Proc Anim Sequencer Tools Menu"),
		FSlateIcon("ProcAnimEditorStyle", "ProcAnimEditor.ToolbarMenu.Small", "ProcAnimEditor.ToolbarMenu.Small")
	);
}

TSharedRef<SWidget> FPACurveEditorExtension::GetMenuContent()
{
	const TSharedPtr<const FUICommandList> InCommandList;
	FMenuBuilder MenuBuilder(true, InCommandList);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("CollectSelected", "CollectSelected"),
		LOCTEXT("CollectSelectedTooltip", "Collect Selected Curves"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&FPACurveEditorExtension::CollectSelectedCurves),
			FCanExecuteAction::CreateStatic( [](){ return true; } )
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("TestSelected", "TestSelected"),
		LOCTEXT("TestSelectedTooltip", "Test Selected Curves"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateStatic(&FPACurveEditorExtension::TestSelectedCurves),
			FCanExecuteAction::CreateStatic( [](){ return true; } )
		)
	);
	
	return MenuBuilder.MakeWidget();
}

void FPACurveEditorExtension::CollectSelectedCurves()
{
	const TSharedPtr<FCurveEditor> CurveEditor = WeakCurveEditor.Pin();
	UPACurveCollector* CurveCollection = FProcAnimModule::PASettings->PACurveCollector.LoadSynchronous();
	if (!CurveEditor.IsValid() || !CurveCollection)
	{
		return;
	}

	TArray<FRichCurve> Curves = GetSelectedCurves();
	for (auto &Curve : Curves)
	{
		Curve.RemoveRedundantAutoTangentKeys(0.01f);
	}
	for (int32 i = Curves.Num() - 1; i >= 0; i--)
	{
		if (Curves[i].Keys.Num() < 3 || Curves[i].IsConstant())
		{
			Curves.RemoveAt(i);
		}
	}
	
	if(Curves.Num() > 0)
	{
		CurveCollection->Modify();
		CurveCollection->Curves.Append(Curves);
	}
}

void FPACurveEditorExtension::TestSelectedCurves()
{
	const TSharedPtr<FCurveEditor> CurveEditor = WeakCurveEditor.Pin();
	const UMLNeuralNet* KeyDetector = FProcAnimModule::PASettings->KeyDetectionNeuralNet.LoadSynchronous();
	const UMLNeuralNet* TangentApprox = FProcAnimModule::PASettings->TangentApproximationNeuralNet.LoadSynchronous();
	if (!CurveEditor.IsValid()
		|| !KeyDetector || !KeyDetector->DataProcessor || !KeyDetector->GetNet()
		|| !TangentApprox || !TangentApprox->DataProcessor || !TangentApprox->GetNet())
	{
		return;
	}
	
	TArray<FRichCurve> Curves = GetSelectedCurves();
	for (int32 i = Curves.Num() - 1; i >= 0; i--)
	{
		if (Curves[i].IsConstant())
		{
			Curves.RemoveAt(i);
		}
	}

	const double Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	const UPACurveReducerDataProcessor *DataProcessor = Cast<UPACurveReducerDataProcessor>(KeyDetector->DataProcessor);
	const UPATangentApproximationProcessor *TangentDataProcessor = Cast<UPATangentApproximationProcessor>(TangentApprox->DataProcessor);
	int32 Correct = 0, Incorrect = 0;
	for(const FRichCurve &Curve : Curves)
	{
		if (Curve.IsConstant())
		{
			continue;
		}
		MatrixXf ProcessedInput = DataProcessor->PreprocessInput(Curve);
		MatrixXf Output = KeyDetector->Forward(ProcessedInput);

		auto CopyOfKeys = Curve.GetCopyOfKeys();
		FString Results = "";
		TArray<float> ResultTimes;
		for (int32 i = 0; i < Output.cols(); i++)
		{
			const float Value = Output(0, i);
			if (Value > 0.95f)
			{
				FRichCurveKey Key;
				const float StartTime = + Curve.Keys[0].Time;
				const MatrixXf TangentOutput = TangentApprox->Forward(ProcessedInput.col(i));
				TangentDataProcessor->ConvertDataToKey(TangentOutput, Key);
				const double ResultTime = (double(i) * Interval);
				UE_LOG(LogTemp, Warning, TEXT("Neural Arrive Tangent: %f, Arrive Tangent Weight: %f, Leave Tangent: %f, Leave Tangent Weight: %f"), Key.ArriveTangent, Key.ArriveTangentWeight, Key.LeaveTangent, Key.LeaveTangentWeight);
				if (const FRichCurveKey* ActualKey = CopyOfKeys.FindByPredicate([ResultTime, StartTime](const FRichCurveKey &K) { return FMath::IsNearlyEqual(K.Time, ResultTime + StartTime, 0.01); }))
				{
					UE_LOG(LogTemp, Warning, TEXT("Actual Arrive Tangent: %f, Arrive Tangent Weight: %f, Leave Tangent: %f, Leave Tangent Weight: %f"), ActualKey->ArriveTangent, ActualKey->ArriveTangentWeight, ActualKey->LeaveTangent, ActualKey->LeaveTangentWeight);
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Actual Key not found for time: %f"), ResultTime);
				}
				ResultTimes.Add(ResultTime);
				Results += FString::SanitizeFloat(ResultTime) + ", ";
			}
		}
		UE_LOG(LogTemp, Log, TEXT("Value: \n%s"), *FMLESLibrary::EigenMatrixToString(Output));
		//UE_LOG(LogTemp, Warning, TEXT("ResultTimes: %s"), *Results);

		const float StartKeyTime = Curve.Keys[0].Time;
		FString Actual = "";
		/*for (const FRichCurveKey &Key : Curve.Keys)
		{
			const float RelativeKeyTime = Key.Time - StartKeyTime;
			Actual += FString::SanitizeFloat(RelativeKeyTime) + ", ";
			bool IsCorrect = false;
			for (const float RT : ResultTimes)
			{
				if (FMath::IsNearlyEqual(RelativeKeyTime, RT, 0.01))
				{
					IsCorrect = true;
					break;
				}
			}
			if (IsCorrect)
			{
				Correct++;
			}
			else
			{
				Incorrect++;
			}
		}*/

		for (const float &Result : ResultTimes)
		{
			const float KeyTime = Result + StartKeyTime;
			if (KeyTime == 0.f)
			{
				continue;
			}
			if (Curve.KeyExistsAtTime(KeyTime))
            {
                Correct++;
            }
            else
            {
                Incorrect++;
            }
		}
		
		//UE_LOG(LogTemp, Warning, TEXT("ActualTimes: %s"), *Actual);
	}
	UE_LOG(LogTemp, Warning, TEXT("Correct: %d, Incorrect: %d Ratio: %f"), Correct, Incorrect, (float)Correct / float(Correct + Incorrect));
}

TArray<FRichCurve> FPACurveEditorExtension::GetSelectedCurves()
{
	TArray<FRichCurve> Curves;
	const TSharedPtr<FCurveEditor> CurveEditor = WeakCurveEditor.Pin();
	check(CurveEditor.IsValid());
	
	const FCurveEditorSelection &Selection = CurveEditor.Get()->GetSelection();
	TArray<FKeyHandle> OriginalKeyHandles;
	TArray<FKeyPosition> SelectedKeyPositions;
	for (const auto& SelectedCurve : Selection.GetAll())
	{
		if (const FCurveModel* CurveModel = CurveEditor->FindCurve(SelectedCurve.Key))
        {
			const int32 KeyCount = SelectedCurve.Value.Num();
			const FKeyHandleSet KeyHandleSet = SelectedCurve.Value;
			OriginalKeyHandles.Empty();
			OriginalKeyHandles.Append(KeyHandleSet.AsArray().GetData(), KeyCount);

			SelectedKeyPositions.SetNumUninitialized(OriginalKeyHandles.Num());
			CurveModel->GetKeyPositions(OriginalKeyHandles, SelectedKeyPositions);
			
			double MinKey = TNumericLimits<double>::Max(), MaxKey = TNumericLimits<double>::Lowest();
			double MinVal = TNumericLimits<double>::Max(), MaxVal = TNumericLimits<double>::Lowest();
			for (const FKeyPosition Key : SelectedKeyPositions)
			{
				MinKey = FMath::Min(Key.InputValue, MinKey);
				MaxKey = FMath::Max(Key.InputValue, MaxKey);
				MinVal = FMath::Min(Key.OutputValue, MinVal);
				MaxVal = FMath::Max(Key.OutputValue, MaxVal);
			}
			
			if(MinVal == MaxVal)
            {
                //UE_LOG(LogTemp, Warning, TEXT("Curve has constant value: %s"), *CurveModel->GetLongDisplayName().ToString());
                continue;
            }
			
			//CurveModel->GetKeys(*CurveEditor.Get(), MinKey, MaxKey, TNumericLimits<double>::Lowest(), TNumericLimits<double>::Max(), OriginalKeyHandles);
			
			TArray<FKeyAttributes> KeyAttributes;
			KeyAttributes.SetNum(OriginalKeyHandles.Num());
			CurveModel->GetKeyAttributes(OriginalKeyHandles, KeyAttributes);
			
			FRichCurve Curve;
			for(int32 i = 0; i < SelectedKeyPositions.Num(); i++)
            {
	            const FKeyPosition KeyPosition = SelectedKeyPositions[i];
                Curve.AddKey(KeyPosition.InputValue, KeyPosition.OutputValue);
				FKeyAttributes KeyAttribute = KeyAttributes[i];
				FRichCurveKey& Key = Curve.Keys.Last();
				if(KeyAttribute.HasInterpMode())
                {
                    Key.InterpMode = KeyAttribute.GetInterpMode();
                }
				if(KeyAttribute.HasTangentMode())
                {
                    Key.TangentMode = KeyAttribute.GetTangentMode();
                }
				if(KeyAttribute.HasArriveTangent())
                {
                    Key.ArriveTangent = KeyAttribute.GetArriveTangent();
                }
				if(KeyAttribute.HasLeaveTangent())
                {
                    Key.LeaveTangent = KeyAttribute.GetLeaveTangent();
                }
				if(KeyAttribute.HasTangentWeightMode())
				{
					Key.TangentWeightMode = KeyAttribute.GetTangentWeightMode();
				}
				if(KeyAttribute.HasArriveTangentWeight())
				{
					Key.ArriveTangentWeight = KeyAttribute.GetArriveTangentWeight();
				}
				if(KeyAttribute.HasLeaveTangentWeight())
                {
					Key.LeaveTangentWeight = KeyAttribute.GetLeaveTangentWeight();
                }
            }
			
			if(FProcAnimModule::PASettings->VerifyCurvesOnAdd)
			{
				double t = MinKey;
				while(t < MaxKey)
				{
					double ModelValue;
					CurveModel->Evaluate(t, ModelValue);
					double CurveValue = Curve.Eval(t);
					if (FMath::IsNearlyEqual(ModelValue, CurveValue, 0.01) == false)
					{
						UE_LOG(LogTemp, Warning, TEXT("Curve evaluation mismatch (%s): Time: %f ModelValue: %f, CurveValue: %f"), *CurveModel->GetLongDisplayName().ToString(), t, ModelValue, CurveValue);
					}
					t += FProcAnimModule::PASettings->DefaultFrameInterval;
				}
			}
			
			Curves.Add(Curve);
		}
	}
	return Curves;
}
#undef LOCTEXT_NAMESPACE
