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
		: WeakCurveEditor(InCurveEditor)
	{
	}

	virtual ~FPACurveEditorExtension() override;

	// ICurveEditorExtension Interface
	virtual void BindCommands(TSharedRef<FUICommandList> CommandBindings) override;
	// ~ICurveEditorExtension
	
	static TSharedRef<ICurveEditorExtension> CreateCurveEditorExtension(TWeakPtr<FCurveEditor> InCurveEditor);
	TSharedRef<FExtender> ExtendCurveEditorToolbarMenu(const TSharedRef<FUICommandList> CommandList);

private:
	void FillToolbarTools(FToolBarBuilder& ToolbarBuilder);
	TSharedRef<SWidget> GetMenuContent();
	void CollectSelectedCurves() const;
	void TestSelectedCurves() const;

	TArray<FRichCurve> GetSelectedCurves() const;

	TWeakPtr<FCurveEditor> WeakCurveEditor;
};
