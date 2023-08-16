// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGeometryCollectionActor.h"

void AMyGeometryCollectionActor::SetPieceTransform(int32 Index, FTransform NewTransform)
{

	GetGeometryCollectionComponent()->GetDynamicCollection()->Transform[Index] = NewTransform;

}
