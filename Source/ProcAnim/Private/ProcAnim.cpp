// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcAnim.h"
#include "ISequencerModule.h"
#include "PASequencerToolbar.h"
#include "Editor/Sequencer/Private/Sequencer.h"

#define LOCTEXT_NAMESPACE "FProcAnimModule"

DECLARE_LOG_CATEGORY_EXTERN(LogProcAnim, Log, All);
DEFINE_LOG_CATEGORY(LogProcAnim);

TWeakPtr<ISequencer> FProcAnimModule::WeakSequencer = nullptr;
FDelegateHandle FProcAnimModule::OnSequencerCreatedDelegateHandle;

void FProcAnimModule::StartupModule() {
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	const auto OnSequencerCreatedDelegate = FOnSequencerCreated::FDelegate::CreateLambda([](const TSharedRef<ISequencer> &InSequencer) {
		FProcAnimModule::WeakSequencer = InSequencer.ToWeakPtr();
	});
	OnSequencerCreatedDelegateHandle = OnSequencerCreatedDelegate.GetHandle();
	SequencerModule.RegisterOnSequencerCreated(OnSequencerCreatedDelegate);


	// set up the toolbar
	const TSharedPtr<FExtender> SequencerToolbarExtender = MakeShareable(new FExtender);
	const TSharedPtr<FUICommandList> SequencerToolbarActions = MakeShared<FUICommandList>();

	SequencerToolbarExtender->AddToolBarExtension("CurveEditor", EExtensionHook::After, SequencerToolbarActions,
		FToolBarExtensionDelegate::CreateStatic(&FProcAnimModule::CreateSequencerToolbar));
	
	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);
	
}

void FProcAnimModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnregisterOnSequencerCreated(OnSequencerCreatedDelegateHandle);
}

void FProcAnimModule::CreateSequencerToolbar(FToolBarBuilder& ToolbarBuilder)
{
	if(!GetSequencer())
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

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProcAnimModule, ProcAnim)