// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GeometryCollection/GeometryCollectionActor.h"
#include <GeometryCollection/GeometryCollectionComponent.h>
#include "MyGeometryCollectionActor.generated.h"

/**
 * 
 */
UCLASS()
class UE5CHAOSTESTS_API AMyGeometryCollectionActor : public AGeometryCollectionActor
{
	GENERATED_BODY()


public:
	void SetPieceLocation(int32 Index, FVector Location);
	void SetPieceTransform(int32 Index, FTransform NewTransform);
	
};
