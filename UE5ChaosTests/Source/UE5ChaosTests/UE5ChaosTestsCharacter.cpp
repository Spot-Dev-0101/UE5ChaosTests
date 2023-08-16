// Copyright Epic Games, Inc. All Rights Reserved.

#include "UE5ChaosTestsCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include <GeometryCollection/GeometryCollectionComponent.h>
#include <GeometryCollection/GeometryCollectionActor.h>
#include "Chaos/ParticleHandleFwd.h"
#include "Chaos/PBDRigidsEvolutionFwd.h"
#include "PhysicsCoreTypes.h"
#include "Chaos/ArrayCollectionArray.h"
#include "Chaos/RigidParticles.h"
#include "Chaos/Rotation.h"
#include "GeometryCollection/GeometryCollectionParticlesData.h" 
#include "Chaos/ChaosSolverActor.h"
#include "Chaos/Utilities.h"
#include "Chaos/Plane.h"
#include "Chaos/Box.h"
#include "Chaos/Sphere.h"
#include "Chaos/PerParticleGravity.h"
#include "Chaos/ImplicitObject.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "GeometryCollection/GeometryCollectionAlgo.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "GeometryCollection/GeometryCollectionObject.h"
#include "GeometryCollection/GeometryCollectionUtility.h"
#include "Math/Box.h"
#include "Physics/Experimental/PhysScene_Chaos.h"
#include "Physics/PhysicsInterfaceCore.h"
#include "PhysicsSolver.h"
#include "Math/Box.h"
#include "PhysicsEngine/BodyInstance.h"
#include "Kismet/GameplayStatics.h"
#include "GeometryCollection/GeometryCollectionDebugDrawComponent.h"


#define print(text) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.5, FColor::Blue,text)
#define printFString(text, fstring) if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT(text), fstring))

//////////////////////////////////////////////////////////////////////////
// AUE5ChaosTestsCharacter

AUE5ChaosTestsCharacter::AUE5ChaosTestsCharacter()
{


	PrimaryActorTick.TickGroup = TG_PostPhysics;
	

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	TargetBall = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetBall"));
	TargetBall->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	static ConstructorHelpers::FObjectFinder<UStaticMesh>SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	TargetBall->SetStaticMesh(SphereMeshAsset.Object);

	TargetBall->SetWorldScale3D(FVector(0.1f, 0.1f, 0.1f));

	TargetBall->SetCollisionProfileName(TEXT("OverlapAll"));

	SelectedPieceKey = "NONE";
}

void AUE5ChaosTestsCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}



void AUE5ChaosTestsCharacter::Tick(float DeltaTime) {
	
	if (SelectedPieceIndex != -99 && SelectedDynamicCollection != nullptr && SelectedGeomCollectionActor != nullptr || false) {

		//FVector RotatedMTL = SelectedDynamicCollection->Transform[SelectedPieceIndex].GetRotation().RotateVector(SelectedDynamicCollection->MassToLocal[SelectedPieceIndex].GetLocation());

	
		//for (int i = 0; i < SelectedDynamicCollection->Active.Num(); i++) {
		//	SelectedDynamicCollection->Active[i] = true;
		//	SelectedDynamicCollection->SimulatableParticles[i] = true;
		//}

		//FTransform OldTransform = SelectedDynamicCollection->Transform[SelectedPieceIndex];
		//FTransform NewTransform;
		//NewTransform.SetRotation(FRotator(0, 0, 0).Quaternion());
		

		//FVector TargetLocation = GetActorLocation() - SelectedDynamicCollection->MassToLocal[SelectedPieceIndex].GetLocation() - SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();
		//FVector StartLocation = CurrentPieceLocation;

		
		//FVector NewLocation = FMath::Lerp(CurrentPieceLocation, TargetLocation, 0.01f);

		//CurrentPieceLocation = NewLocation;

		//double Progress = FVector::Dist(CurrentPieceLocation, TargetLocation) / FVector::Dist(CurrentPieceOriginLocationLocal, TargetLocation);

		//print(FString::SanitizeFloat(Progress));

		//SelectedDynamicCollection->Transform[SelectedPieceIndex].BlendWith(NewTransform, 1-Progress);


		//SelectedDynamicCollection->Transform[SelectedPieceIndex].SetLocation(CurrentPieceLocation);
		
		print(CurrentPieceLocation.ToString());

		

	}

	if (SelectedPieceKey != "NONE") {
		int32 Index = ManipulatedPieceData[SelectedPieceKey].Index;
		FGeometryDynamicCollection* DynamicCollection = ManipulatedPieceData[SelectedPieceKey].DynamicCollection;
		AGeometryCollectionActor* GeomCollectionActor = ManipulatedPieceData[SelectedPieceKey].GeomCollectionActor;
		FVector CurrentLocation = ManipulatedPieceData[SelectedPieceKey].Location;
		FVector OriginLocationLocal = ManipulatedPieceData[SelectedPieceKey].OriginLocationLocal;

		

		FVector RotatedMTL = DynamicCollection->Transform[Index].GetRotation().RotateVector(DynamicCollection->MassToLocal[Index].GetLocation());

		FTransform OldTransform = DynamicCollection->Transform[Index];
		FTransform NewTransform;
		NewTransform.SetRotation(DynamicCollection->Transform[Index].GetRotation());

		FVector TargetLocation = GetActorLocation() - GeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation() - RotatedMTL;//DynamicCollection->MassToLocal[Index].GetLocation();
		FVector StartLocation = CurrentLocation;

		FVector NewLocation = FMath::Lerp(CurrentLocation, TargetLocation, 0.01f);

		CurrentLocation = NewLocation;
		ManipulatedPieceData[SelectedPieceKey].Location = CurrentLocation;
		

		double Progress = FVector::Dist(CurrentLocation, TargetLocation) / FVector::Dist(OriginLocationLocal, TargetLocation);

		//print(FString::SanitizeFloat(Progress));

		//ManipulatedPieceData[SelectedPieceKey].Transform.BlendWith(NewTransform, 1 - Progress);

		ManipulatedPieceData[SelectedPieceKey].Transform.SetLocation(CurrentLocation);// -((DynamicCollection->MassToLocal[Index].GetLocation()) * (1 - Progress)));

		//GeomCollectionActor->GetGeometryCollectionComponent()->GetBoundingBoxArrayCopyOnWrite()[Index] = GeomCollectionActor->GetGeometryCollectionComponent()->GetBoundingBoxArray()[Index].MoveTo(CurrentLocation);

		//print((DynamicCollection->MassToLocal[Index].GetLocation() * (1 - Progress)).ToString());

	}


	for (auto& elem : ManipulatedPieceData) {
		FPieceData Data = elem.Value;

		Data.DynamicCollection->Transform[Data.Index] = Data.Transform;

		for (int i = 0; i < Data.DynamicCollection->Active.Num(); i++) {
			Data.DynamicCollection->Active[i] = true;
		}

		//Data.GeomCollectionActor->GetGeometryCollectionComponent()->RecreatePhysicsState();
		//Data.GeomCollectionActor->GetGeometryCollectionComponent()->OnCreatePhysicsState();
		TArray<FTransform> TempDynamicTransforms;
		for (int i = 0; i < Data.DynamicCollection->Transform.Num(); i++) {
			TempDynamicTransforms.Add(Data.DynamicCollection->Transform[i]);
		}

		//Data.GeomCollectionActor->GetGeometryCollectionComponent()->RestTransforms = TempDynamicTransforms;
		//Data.GeomCollectionActor->GetGeometryCollectionComponent()->SetRestState(std::move(TempDynamicTransforms));
		//Data.GeomCollectionActor->GetGeometryCollectionComponent()->GetRestCollection()->GetGeometryCollection()->Transform = TempDynamicTransforms;
		//Data.GeomCollectionActor->GetGeometryCollectionComponent()->RecreatePhysicsState();

		//print("Rest: " + Data.GeomCollectionActor->GetGeometryCollectionComponent()->GetRestCollection()->GetGeometryCollection()->Transform[Data.Index].GetLocation().ToString());

	}

	TArray<AActor*> GeomActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGeometryCollectionActor::StaticClass(), GeomActors);


	//TArray<UActorComponent*> AllGeomActors = GetWorld()->Actor GetComponentsByClass(UGeometryCollectionComponent::StaticClass());
	//print("NUM: " + FString::SanitizeFloat(GeomActors.Num()));
	for (int i = 0; i < GeomActors.Num(); i++) {
		AGeometryCollectionActor* GeomActor = (AGeometryCollectionActor*)GeomActors[i];
		UGeometryCollectionComponent* GeomComp = GeomActor->GetGeometryCollectionComponent();
	
		TArray<FTransform> TempDynamicTransforms;
		for (int x = 0; x < GeomComp->GetDynamicCollection()->Transform.Num(); x++) {
			TempDynamicTransforms.Add(GeomComp->GetDynamicCollection()->Transform[x]);
		}

		GeomComp->RestTransforms = TempDynamicTransforms;
		GeomComp->SetRestState(std::move(TempDynamicTransforms));
		//GeomComp->GetRestCollection()->GetGeometryCollection()->Transform = TempDynamicTransforms;
		//GeomComp->RecreatePhysicsState();


   
	}

}

//////////////////////////////////////////////////////////////////////////
// Input

void AUE5ChaosTestsCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUE5ChaosTestsCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUE5ChaosTestsCharacter::Look);

		InputComponent->BindAxis(TEXT("PickupPiece"), this, &AUE5ChaosTestsCharacter::PickupPieceAxis);

	}

}

void AUE5ChaosTestsCharacter::PickupPieceAxis(float value)
{
	
	if (value > 0 && SelectedPieceKey == "NONE") {
		SelectedPieceIndex = GetLookingAtPiece();
		//print(SelectedPieceKey);
	}
	if (value <= 0) {
		//SelectedPieceIndex = -99;
		//SelectedDynamicCollection = nullptr;
		//SelectedGeomCollectionActor = nullptr;
		//SelectedSolver = nullptr;
		SelectedPieceKey = "NONE";
	}
}

int32 AUE5ChaosTestsCharacter::GetLookingAtPiece()
{

	int32 ClosestIndex = -99;

	FHitResult HitResult;
	FVector Start = FollowCamera->GetComponentLocation();
	FVector End = Start + (FollowCamera->GetForwardVector() * 1000);
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility);

	if (bHit == true) {
		//print(HitResult.GetActor()->GetName());
		if (HitResult.GetActor()->GetClass()->IsChildOf(AGeometryCollectionActor::StaticClass())) {
			//print("");

			AGeometryCollectionActor* GeomCollectionActor = (AGeometryCollectionActor*)HitResult.GetActor();

			using namespace Chaos;
			FPhysScene_Chaos* Scene = GeomCollectionActor->GetGeometryCollectionComponent()->GetInnerChaosScene();
			ensure(Scene);
			
			const Chaos::FPhysicsSolver* Solver = Scene->GetSolver();
			
			if (ensure(Solver))
			{
				//SelectedSolver = Scene->GetSolver();
				FGeometryDynamicCollection* DynamicCollection = GeomCollectionActor->GetGeometryCollectionComponent()->GetDynamicCollection();
				

				for (int i = 0; i < DynamicCollection->Active.Num(); i++) {
					DynamicCollection->Active[i] = true;
				}

				FVector HitLocation = HitResult.Location;
				double ClosestDistance = 99999999999;
				for (int i = 0; i < DynamicCollection->MassToLocal.Num(); i++) {//

					//FTransform OldBrickTransform = DynamicCollection->Transform[i];
					FTransform Transform = DynamicCollection->Transform[i];
					FString NewKey = HitResult.GetActor()->GetName() + "_" + FString::SanitizeFloat(i);
					if (ManipulatedPieceData.Contains(NewKey)) {
						Transform = ManipulatedPieceData[NewKey].Transform;
					}

					FVector RotatedMTL = Transform.GetRotation().RotateVector(DynamicCollection->MassToLocal[i].GetLocation());

					FVector BrickWorldLocation = Transform.GetLocation() + RotatedMTL + GeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();

					double Dist = FVector::Distance(HitLocation, BrickWorldLocation);
					if (Dist < ClosestDistance) {
						ClosestDistance = Dist;
						ClosestIndex = i;
					}

					//DynamicCollection->Transform[i] = OldBrickTransform;
				}





				FVector RotatedMTL = DynamicCollection->Transform[ClosestIndex].GetRotation().RotateVector(DynamicCollection->MassToLocal[ClosestIndex].GetLocation());

				FString NewKey = HitResult.GetActor()->GetName() + "_" + FString::SanitizeFloat(ClosestIndex);
				if (!ManipulatedPieceData.Contains(NewKey)) {
					FPieceData NewPieceData;
					NewPieceData.Index = ClosestIndex;
					NewPieceData.DynamicCollection = DynamicCollection;
					NewPieceData.GeomCollectionActor = GeomCollectionActor;
					NewPieceData.Location = DynamicCollection->Transform[ClosestIndex].GetLocation();
					NewPieceData.OriginLocation = DynamicCollection->Transform[ClosestIndex].GetLocation() + RotatedMTL + GeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();
					NewPieceData.OriginLocationLocal = DynamicCollection->Transform[ClosestIndex].GetLocation();
					NewPieceData.Transform = DynamicCollection->Transform[ClosestIndex];
					ManipulatedPieceData.Add(NewKey, NewPieceData);
				}
				SelectedPieceKey = NewKey;

				//SelectedDynamicCollection = DynamicCollection;
				//SelectedGeomCollectionActor = GeomCollectionActor;

				
				//CurrentPieceOriginLocation = SelectedDynamicCollection->Transform[ClosestIndex].GetLocation() + RotatedMTL + SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();
				//CurrentPieceOriginLocationLocal = SelectedDynamicCollection->Transform[ClosestIndex].GetLocation();
				//TargetBall->SetWorldLocation(CurrentPieceOriginLocation);
				//CurrentPieceLocation = SelectedDynamicCollection->Transform[ClosestIndex].GetLocation();
			}
		}
	}

	return ClosestIndex;
}

void AUE5ChaosTestsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AUE5ChaosTestsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}




