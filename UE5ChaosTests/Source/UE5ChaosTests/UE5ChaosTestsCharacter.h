// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "GeometryCollectionProxyData.h"
#include "GeometryCollection/GeometryCollectionActor.h" 
#include "PhysicsProxy/GeometryCollectionPhysicsProxy.h" 
#include "MyGeometryCollectionActor.h"
#include "UE5ChaosTestsCharacter.generated.h"

USTRUCT(BlueprintType)
struct FPieceData
{
	GENERATED_BODY()

	int32 Index;
	FTransform Transform;
	FGeometryDynamicCollection* DynamicCollection;
	AGeometryCollectionActor* GeomCollectionActor;
	FVector OriginLocation;
	FVector Location;
	FVector OriginLocationLocal;

};

UCLASS(config=Game)
class AUE5ChaosTestsCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	

public:
	AUE5ChaosTestsCharacter();
	

protected:

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);
			

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	// To add mapping context
	virtual void BeginPlay();

	void PickupPieceAxis(float value);

	int32 GetLookingAtPiece();

	int32 SelectedPieceIndex;

	FGeometryDynamicCollection* SelectedDynamicCollection;
	AGeometryCollectionActor* SelectedGeomCollectionActor;
	FVector CurrentPieceOriginLocation;
	FVector CurrentPieceLocation;
	FVector CurrentPieceOriginLocationLocal;
	//Chaos::FPBDRigidsSolver* SelectedSolver;
	FString SelectedPieceKey;
	TMap<FString, FPieceData> ManipulatedPieceData; //Key is Collection name + piece index

public:

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	/** Look Ball */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
		UStaticMeshComponent* TargetBall;
};

