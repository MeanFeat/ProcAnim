// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISequencerModule.h"
#include "PACurveCollector.h"
#include "Modules/ModuleManager.h"

class UPASettings;
class ISequencer;

class FProcAnimModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static TSharedPtr<ISequencer> GetSequencer() {return WeakSequencer.Pin();}
	
	static UPASettings *PASettings;

private:
	
	static TWeakPtr<ISequencer> WeakSequencer;

	FDelegateHandle SequencerExtensionHandle;
	FDelegateHandle CurveEditorExtensionHandle;
};
