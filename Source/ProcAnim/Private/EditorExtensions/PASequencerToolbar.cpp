// Fill out your copyright notice in the Description page of Project Settings.

#include "PASequencerToolbar.h"
#include "ProcAnim.h"

#define LOCTEXT_NAMESPACE "FProcAnimSequecerToolbar"

FPASequencerToolbar::FPASequencerToolbar()
{
}

FPASequencerToolbar::~FPASequencerToolbar()
{
}

void FPASequencerToolbar::AddObjectBindingExtensions(const TSharedPtr<FExtender>& ObjectBindingExtender)
{
	const TSharedPtr<FUICommandList> ObjectBindingActions = MakeShared<FUICommandList>();
}

void FPASequencerToolbar::CreateSequencerToolbar(FToolBarBuilder& ToolbarBuilder)
{
	if(!FProcAnimModule::GetSequencer())
	{
		return;
	}
	ToolbarBuilder.BeginSection("ProcAnimSequencerTools");
	ToolbarBuilder.AddComboButton(
		FUIAction(),
		FOnGetContent::CreateStatic(&FPASequencerToolbar::GetMenuContent),
		TAttribute<FText>(),
		LOCTEXT("ProcAnimSequencerToolsTooltip", "Open Proc Anim Sequencer Tools Menu"),
		FSlateIcon("ProcAnimEditorStyle", "ProcAnimEditor.ToolbarMenu.Small", "ProcAnimEditor.ToolbarMenu.Small")
	);
}

TSharedRef<SWidget> FPASequencerToolbar::GetMenuContent()
{
	const TSharedPtr<const FUICommandList> InCommandList;
	FMenuBuilder MenuBuilder(true, InCommandList);

	MenuBuilder.AddMenuEntry(
		LOCTEXT("Hello", "Hello"),
		LOCTEXT("HelloTooltip", "Hello Tooltip"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateLambda([]()
			{
				UE_LOG(LogTemp, Warning, TEXT("Hello"));
			})
		)
	);
	
	return MenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE