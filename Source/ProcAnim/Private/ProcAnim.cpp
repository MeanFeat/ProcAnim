// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProcAnim.h"
#include "ISequencerModule.h"
#include "Editor/Sequencer/Private/Sequencer.h"

#define LOCTEXT_NAMESPACE "FProcAnimModule"

DECLARE_LOG_CATEGORY_EXTERN(LogProcAnim, Log, All);
DEFINE_LOG_CATEGORY(LogProcAnim);

TWeakPtr<ISequencer> FProcAnimModule::WeakSequencer = nullptr;

void FProcAnimModule::StartupModule() {
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	ISequencerModule& SequencerModule = FModuleManager::LoadModuleChecked<ISequencerModule>("Sequencer");
	const auto OnSequencerCreatedDelegate = FOnSequencerCreated::FDelegate::CreateLambda([](const TSharedRef<ISequencer> &InSequencer) {
		FProcAnimModule::WeakSequencer = InSequencer.ToWeakPtr();
	});
	SequencerModule.RegisterOnSequencerCreated(OnSequencerCreatedDelegate);
}

void FProcAnimModule::ShutdownModule() {
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FProcAnimModule, ProcAnim)