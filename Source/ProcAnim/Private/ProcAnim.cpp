// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcAnim.h"
#include "ICurveEditorModule.h"
#include "ISequencerModule.h"
#include "ISettingsModule.h"
#include "PACurveEditorExtension.h"
#include "PASequencerToolbar.h"
#include "PASettings.h"
#include "Editor/Sequencer/Private/Sequencer.h"

UPASettings* FProcAnimModule::PASettings = nullptr;
TWeakPtr<ISequencer> FProcAnimModule::WeakSequencer = nullptr;

#define LOCTEXT_NAMESPACE "FProcAnimModule"
DECLARE_LOG_CATEGORY_EXTERN(LogProcAnim, Log, All);
DEFINE_LOG_CATEGORY(LogProcAnim);


void FProcAnimModule::StartupModule() {
	
	// Sequencer
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	const auto OnSequencerCreatedDelegate = FOnSequencerCreated::FDelegate::CreateLambda([](const TSharedRef<ISequencer> &InSequencer) {
		WeakSequencer = InSequencer.ToWeakPtr();
	});
	SequencerExtensionHandle = OnSequencerCreatedDelegate.GetHandle();
	SequencerModule.RegisterOnSequencerCreated(OnSequencerCreatedDelegate);

	const TSharedPtr<FExtender> SequencerToolbarExtender = MakeShareable(new FExtender);
	const TSharedPtr<FUICommandList> SequencerToolbarActions = MakeShared<FUICommandList>();

	SequencerToolbarExtender->AddToolBarExtension("CurveEditor", EExtensionHook::After, SequencerToolbarActions,
		FToolBarExtensionDelegate::CreateStatic(&FPASequencerToolbar::CreateSequencerToolbar));
	
	SequencerModule.GetToolBarExtensibilityManager()->AddExtender(SequencerToolbarExtender);

	// Curve Editor
	ICurveEditorModule& CurveEditorModule = FModuleManager::Get().LoadModuleChecked<ICurveEditorModule>("CurveEditor");
	CurveEditorExtensionHandle = CurveEditorModule.RegisterEditorExtension(FOnCreateCurveEditorExtension::CreateStatic(&FPACurveEditorExtension::CreateCurveEditorExtension));

	const auto ToolbarExtender = ICurveEditorModule::FCurveEditorMenuExtender::CreateStatic(&FPACurveEditorExtension::ExtendCurveEditorToolbarMenu);
	auto& MenuExtenders = CurveEditorModule.GetAllToolBarMenuExtenders();
	MenuExtenders.Add(ToolbarExtender);

	// Settings
	PASettings = GetMutableDefault<UPASettings>();
	
#if WITH_EDITORONLY_DATA 
	if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings(
			"Project",
			"ProcAnim",
			"ProcAnimSettings",
			LOCTEXT("RuntimeSettingsName", "Procedural Animation Parameters"),
			LOCTEXT("RuntimeSettingsDescription", "Globally accessible data for use in procedural animation"),
			PASettings
		);
	}
#endif
	
}

void FProcAnimModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// TODO: Unregister Editor Extension
	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnregisterOnSequencerCreated(SequencerExtensionHandle);
	
	ICurveEditorModule& CurveEditorModule = FModuleManager::Get().LoadModuleChecked<ICurveEditorModule>("CurveEditor");
	CurveEditorModule.UnregisterEditorExtension(CurveEditorExtensionHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProcAnimModule, ProcAnim)