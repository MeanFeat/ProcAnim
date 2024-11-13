// Fill out your copyright notice in the Description page of Project Settings.


#include "PASettings.h"

void UPASettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if(PropertyChangedEvent.Property != nullptr)
	{
		const FName PropertyName = PropertyChangedEvent.Property->GetFName();
		if(PropertyName == GET_MEMBER_NAME_CHECKED(UPASettings, DefaultFPS))
		{
			DefaultFrameInterval = 1.f/DefaultFPS;
		}
	}
}
