// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"

/**
 * 
 */
class PROCANIM_API FPASequencerToolbar
{
public:
	FPASequencerToolbar();
	~FPASequencerToolbar();

	static TSharedRef<SWidget> GetMenuContent();
	
private:

	static void AddObjectBindingExtensions(const TSharedPtr<FExtender> &ObjectBindingExtender);
};
