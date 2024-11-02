// Fill out your copyright notice in the Description page of Project Settings.

#include "PACurveEditorExtension.h"
#include "PASettings.h"
#include "ProcAnim.h"
#include "RichCurveEditorModel.h"

#define LOCTEXT_NAMESPACE "FPACurveEditorExtension"

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
	
	return MenuBuilder.MakeWidget();
}

void FPACurveEditorExtension::CollectSelectedCurves() const
{
	const TSharedPtr<FCurveEditor> CurveEditor = WeakCurveEditor.Pin();
	UPACurveCollector* CurveCollection = FProcAnimModule::PASettings->PACurveCollector.Get();
	if (!CurveEditor.IsValid() || !CurveCollection)
	{
		return;
	}
	
	const FCurveEditorSelection &Selection = CurveEditor.Get()->GetSelection();
	TArray<FKeyHandle> OriginalKeyHandles;
	TArray<FKeyPosition> SelectedKeyPositions;
	for (const auto& SelectedCurve : Selection.GetAll())
	{
		if (const FCurveModel* CurveModel = CurveEditor->FindCurve(SelectedCurve.Key))
        {
			const int32 KeyCount = SelectedCurve.Value.Num();
			if (KeyCount < 3)
			{
				continue;
			}
			
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
                UE_LOG(LogTemp, Warning, TEXT("Curve has constant value: %s"), *CurveModel->GetLongDisplayName().ToString());
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
			CurveCollection->Modify();
			CurveCollection->Curves.Add(Curve);

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
					t += 0.01;
				}
			}
		}
	}
}
#undef LOCTEXT_NAMESPACE
