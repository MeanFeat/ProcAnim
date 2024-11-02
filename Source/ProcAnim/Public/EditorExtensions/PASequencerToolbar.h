// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"

/**
 * 
 */
class FPASequencerToolbar
{
public:
	FPASequencerToolbar();
	~FPASequencerToolbar();

	static void CreateSequencerToolbar(FToolBarBuilder& ToolbarBuilder);
	
private:

	static void AddObjectBindingExtensions(const TSharedPtr<FExtender> &ObjectBindingExtender);

	static TSharedRef<SWidget> GetMenuContent();

};
