// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ISequencerModule.h"
#include "Modules/ModuleManager.h"

class ISequencer;

class FProcAnimModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static TSharedPtr<ISequencer> GetSequencer() {return WeakSequencer.Pin();}
	
private:
	static void CreateSequencerToolbar(FToolBarBuilder& ToolbarBuilder);
	
	static TWeakPtr<ISequencer> WeakSequencer;

	static FDelegateHandle OnSequencerCreatedDelegateHandle;
};
