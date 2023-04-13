// Fill out your copyright notice in the Description page of Project Settings.


#include "Saver.h"

USaver::USaver()
{
}

void USaver::addToSaveData(const FVector2D threshold, const bool useThreshold)
{
	FThresholdSave newEntry;
	newEntry.Threshold = threshold;
	newEntry.UseThreshold = useThreshold;
	ThresholdsToBeSaved.Add(newEntry);
}
