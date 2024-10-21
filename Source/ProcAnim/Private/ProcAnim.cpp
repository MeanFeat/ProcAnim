// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcAnim.h"
#include "ICurveEditorModule.h"
#include "ISequencerModule.h"
#include "PASequencerToolbar.h"
#include "Editor/Sequencer/Private/Sequencer.h"

#define LOCTEXT_NAMESPACE "FProcAnimModule"

DECLARE_LOG_CATEGORY_EXTERN(LogProcAnim, Log, All);
DEFINE_LOG_CATEGORY(LogProcAnim);

TWeakPtr<ISequencer> FProcAnimModule::WeakSequencer = nullptr;
FDelegateHandle FProcAnimModule::OnSequencerCreatedDelegateHandle;

void FProcAnimModule::StartupModule() {
	
	// Sequencer
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	const auto OnSequencerCreatedDelegate = FOnSequencerCreated::FDelegate::CreateLambda([](const TSharedRef<ISequencer> &InSequencer) {
		FProcAnimModule::WeakSequencer = InSequencer.ToWeakPtr();
	});
	OnSequencerCreatedDelegateHandle = OnSequencerCreatedDelegate.GetHandle();
	SequencerModule.RegisterOnSequencerCreated(OnSequencerCreatedDelegate);

	const TSharedPtr<FExtender> SequencerToolbarExtender = MakeShareable(new FExtender);
	const TSharedPtr<FUICommandList> SequencerToolbarActions = MakeShared<FUICommandList>();

	SequencerToolbarExtender->AddToolBarExtension("CurveEditor", EExtensionHook::After, SequencerToolbarActions,
		FToolBarExtensionDelegate::CreateStatic(&FPASequencerToolbar::CreateSequencerToolbar));
	
	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);

	// Curve Editor
	ICurveEditorModule& CurveEditorModule = FModuleManager::Get().LoadModuleChecked<ICurveEditorModule>("CurveEditor");
	
}

void FProcAnimModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// TODO: Unregister Editor Extension
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnregisterOnSequencerCreated(OnSequencerCreatedDelegateHandle);
	
	ICurveEditorModule& CurveEditorModule = FModuleManager::Get().LoadModuleChecked<ICurveEditorModule>("CurveEditor");
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProcAnimModule, ProcAnim)