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
	
	if (SelectedPieceIndex != -99 && SelectedDynamicCollection != nullptr && SelectedGeomCollectionActor != nullptr) {

		FVector RotatedMTL = SelectedDynamicCollection->Transform[SelectedPieceIndex].GetRotation().RotateVector(SelectedDynamicCollection->MassToLocal[SelectedPieceIndex].GetLocation());

		//TargetBall->SetWorldLocation(SelectedDynamicCollection->Transform[SelectedPieceIndex].GetLocation() + RotatedMTL + SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation());

		for (int i = 0; i < SelectedDynamicCollection->Active.Num(); i++) {
			SelectedDynamicCollection->Active[i] = true;
			SelectedDynamicCollection->SimulatableParticles[i] = true;
		}

		//ClosestIndex = 10;
		FTransform OldTransform = SelectedDynamicCollection->Transform[SelectedPieceIndex];
		FTransform NewTransform;
		NewTransform.SetRotation(FRotator(0, 0, 0).Quaternion());
		//NewTransform.SetLocation(GetActorLocation());
		


		//FVector BrickRelative = SelectedDynamicCollection->MassToLocal[ClosestIndex].GetLocation() + DynamicCollection->Transform[ClosestIndex].GetLocation();

		FVector TargetLocation = GetActorLocation() - SelectedDynamicCollection->MassToLocal[SelectedPieceIndex].GetLocation() - SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();
		FVector StartLocation = CurrentPieceLocation;

		
		FVector NewLocation = FMath::Lerp(CurrentPieceLocation, TargetLocation, 0.01f);

		CurrentPieceLocation = NewLocation;

		double Progress = FVector::Dist(CurrentPieceLocation, TargetLocation) / FVector::Dist(CurrentPieceOriginLocationLocal, TargetLocation);

		print(FString::SanitizeFloat(Progress));

		SelectedDynamicCollection->Transform[SelectedPieceIndex].BlendWith(NewTransform, 1-Progress);

		//TargetBall->SetWorldLocation(CurrentPieceLocation);
		//print("T: " + CurrentPieceLocation.ToString());
		//print("B: " + (TargetBall->GetComponentLocation() - SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation()).ToString());

		SelectedDynamicCollection->Transform[SelectedPieceIndex].SetLocation(CurrentPieceLocation);
		//OldTransform.SetLocation(CurrentPieceLocation);

		//SelectedDynamicCollection->Transform[SelectedPieceIndex] = OldTransform;
		
		print(CurrentPieceLocation.ToString());

		//SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetPhysicsProxy()->PushToPhysicsState();
		//SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetPhysicsProxy()->PushStateOnGameThread(SelectedSolver);
		
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
	
	if (value > 0 && SelectedPieceIndex == -99) {
		SelectedPieceIndex = GetLookingAtPiece();
		print(FString::SanitizeFloat(SelectedPieceIndex));
	}
	if (value <= 0) {
		SelectedPieceIndex = -99;
		SelectedDynamicCollection = nullptr;
		SelectedGeomCollectionActor = nullptr;
		SelectedSolver = nullptr;
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
		print(HitResult.GetActor()->GetName());
		if (HitResult.GetActor()->GetClass()->IsChildOf(AGeometryCollectionActor::StaticClass())) {
			//print("");

			AGeometryCollectionActor* GeomCollectionActor = (AGeometryCollectionActor*)HitResult.GetActor();

			using namespace Chaos;
			FPhysScene_Chaos* Scene = GeomCollectionActor->GetGeometryCollectionComponent()->GetInnerChaosScene();
			ensure(Scene);
			
			const Chaos::FPhysicsSolver* Solver = Scene->GetSolver();
			
			if (ensure(Solver))
			{
				SelectedSolver = Scene->GetSolver();
				FGeometryDynamicCollection* DynamicCollection = GeomCollectionActor->GetGeometryCollectionComponent()->GetDynamicCollection();
				

				for (int i = 0; i < DynamicCollection->Active.Num(); i++) {
					DynamicCollection->Active[i] = true;
				}

				FVector HitLocation = HitResult.Location;
				double ClosestDistance = 99999999999;
				for (int i = 0; i < DynamicCollection->MassToLocal.Num(); i++) {//

					FTransform OldBrickTransform = DynamicCollection->Transform[i];

					FVector RotatedMTL = DynamicCollection->Transform[i].GetRotation().RotateVector(DynamicCollection->MassToLocal[i].GetLocation());

					FVector BrickWorldLocation = DynamicCollection->Transform[i].GetLocation() + RotatedMTL + GeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();

					double Dist = FVector::Distance(HitLocation, BrickWorldLocation);
					if (Dist < ClosestDistance) {
						ClosestDistance = Dist;
						ClosestIndex = i;
					}

					DynamicCollection->Transform[i] = OldBrickTransform;
				}

				SelectedDynamicCollection = DynamicCollection;
				SelectedGeomCollectionActor = GeomCollectionActor;

				FVector RotatedMTL = SelectedDynamicCollection->Transform[ClosestIndex].GetRotation().RotateVector(SelectedDynamicCollection->MassToLocal[ClosestIndex].GetLocation());
				CurrentPieceOriginLocation = SelectedDynamicCollection->Transform[ClosestIndex].GetLocation() + RotatedMTL + SelectedGeomCollectionActor->GetGeometryCollectionComponent()->GetComponentLocation();
				CurrentPieceOriginLocationLocal = SelectedDynamicCollection->Transform[ClosestIndex].GetLocation();
				TargetBall->SetWorldLocation(CurrentPieceOriginLocation);
				CurrentPieceLocation = SelectedDynamicCollection->Transform[ClosestIndex].GetLocation();
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




