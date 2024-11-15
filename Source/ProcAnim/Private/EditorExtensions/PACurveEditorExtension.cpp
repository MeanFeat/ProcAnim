// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveEditorExtension.h"
#include "PACurveCollector.h"
#include "PACurveReducerDataProcessor.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "RichCurveEditorModel.h"
#include "NeuralNet/MLNeuralNet.h"

#define LOCTEXT_NAMESPACE "FPACurveEditorExtension"

class UPACurveReducerDataProcessor;

FPACurveEditorExtension::~FPACurveEditorExtension()
{
}

void FPACurveEditorExtension::BindCommands(TSharedRef<FUICommandList> CommandBindings)
{
}

TSharedRef<ICurveEditorExtension> FPACurveEditorExtension::CreateCurveEditorExtension(TWeakPtr<FCurveEditor> InCurveEditor)
{
	TSharedRef<FPACurveEditorExtension> Extender = MakeShared<FPACurveEditorExtension>(InCurveEditor);
	ICurveEditorModule& CurveEditorModule = FModuleManager::Get().LoadModuleChecked<ICurveEditorModule>("CurveEditor");
	const auto ToolbarExtender = ICurveEditorModule::FCurveEditorMenuExtender::CreateSP(Extender, &FPACurveEditorExtension::ExtendCurveEditorToolbarMenu);
	CurveEditorModule.GetAllToolBarMenuExtenders().Add(ToolbarExtender);
	return Extender;
}

TSharedRef<FExtender> FPACurveEditorExtension::ExtendCurveEditorToolbarMenu(const TSharedRef<FUICommandList> CommandList)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension("Filters",
	EExtensionHook::After,
	CommandList,
	FToolBarExtensionDelegate::CreateSP(this, &FPACurveEditorExtension::FillToolbarTools));

	return Extender;
}

void FPACurveEditorExtension::FillToolbarTools(FToolBarBuilder& ToolbarBuilder)
{
	ToolbarBuilder.BeginSection("ProcAnimCurveEditor");
	ToolbarBuilder.AddSeparator();
	ToolbarBuilder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateSP(this, &FPACurveEditorExtension::GetMenuContent),
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
			FExecuteAction::CreateSP(this, &FPACurveEditorExtension::CollectSelectedCurves),
			FCanExecuteAction::CreateStatic( [](){ return true; } )
		)
	);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("TestSelected", "TestSelected"),
		LOCTEXT("TestSelectedTooltip", "Test Selected Curves"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FPACurveEditorExtension::TestSelectedCurves),
			FCanExecuteAction::CreateStatic( [](){ return true; } )
		)
	);
	
	return MenuBuilder.MakeWidget();
}

void FPACurveEditorExtension::CollectSelectedCurves() const
{
	const TSharedPtr<FCurveEditor> CurveEditor = WeakCurveEditor.Pin();
	UPACurveCollector* CurveCollection = FProcAnimModule::PASettings->PACurveCollector.LoadSynchronous();
	if (!CurveEditor.IsValid() || !CurveCollection)
	{
		return;
	}

	TArray<FRichCurve> Curves = GetSelectedCurves();
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

void FPACurveEditorExtension::TestSelectedCurves() const
{
	const TSharedPtr<FCurveEditor> CurveEditor = WeakCurveEditor.Pin();
	const UMLNeuralNet* CurveReducerNeuralNet = FProcAnimModule::PASettings->PACurveReducerNeuralNet.LoadSynchronous();
	if (!CurveEditor.IsValid() || !CurveReducerNeuralNet)
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

	const float Interval = FProcAnimModule::PASettings->DefaultFrameInterval;
	const UPACurveReducerDataProcessor *DataProcessor = CurveReducerNeuralNet->DataProcessorClass->GetDefaultObject<UPACurveReducerDataProcessor>();
	int32 Correct = 0, Incorrect = 0;
	for(FRichCurve &Curve : Curves)
	{
		MatrixXf ProcessedInput = DataProcessor->PreprocessInput(Curve);
		MatrixXf Output = CurveReducerNeuralNet->Forward(ProcessedInput);
		TArray<float> OutputArray;
		
		OutputArray.Reserve(Output.cols());
		for(int32 i = 0; i < Output.cols(); i++)
		{
			OutputArray.Add(Output(0, i));
		}
		const FCurveReducerCurveParams Params(Curve);
		float t = Params.StartTime;
		for(const float Value : OutputArray)
		{
			const bool bIsKey = Curve.KeyExistsAtTime(t);
			if(Value > 0.75f)
			{
				if(bIsKey)
				{
					Correct++;
				}
				else
				{
					Incorrect++;
				}
			}
			else if(bIsKey)
			{
				Incorrect++;
			}
			
			t += Interval;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Correct: %d, Incorrect: %d Ratio: %f"), Correct, Incorrect, (float)Correct / float(Correct + Incorrect));
}

TArray<FRichCurve> FPACurveEditorExtension::GetSelectedCurves() const
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
			OriginalKeyHandles.Reset(KeyCount);
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
			
			CurveModel->GetKeys(*CurveEditor.Get(), MinKey, MaxKey, TNumericLimits<double>::Lowest(), TNumericLimits<double>::Max(), OriginalKeyHandles);
			
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
