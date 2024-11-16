// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "CurveEditor.h"
#include "ICurveEditorExtension.h"

/**
 * 
 */
class FPACurveEditorExtension : public ICurveEditorExtension, public TSharedFromThis<FPACurveEditorExtension>
{
	
public:
	FPACurveEditorExtension(const TWeakPtr<FCurveEditor>& InCurveEditor)
	{
	}

	virtual ~FPACurveEditorExtension() override;

	// ICurveEditorExtension Interface
	virtual void BindCommands(TSharedRef<FUICommandList> CommandBindings) override;
	// ~ICurveEditorExtension
	
	static TSharedRef<ICurveEditorExtension> CreateCurveEditorExtension(TWeakPtr<FCurveEditor> InCurveEditor);
	static TSharedRef<FExtender> ExtendCurveEditorToolbarMenu(const TSharedRef<FUICommandList> CommandList);

private:
	static void FillToolbarTools(FToolBarBuilder& ToolbarBuilder);
	static TSharedRef<SWidget> GetMenuContent();
	static void CollectSelectedCurves();
	static void TestSelectedCurves();

	static TArray<FRichCurve> GetSelectedCurves();

	static TWeakPtr<FCurveEditor> WeakCurveEditor;
};
