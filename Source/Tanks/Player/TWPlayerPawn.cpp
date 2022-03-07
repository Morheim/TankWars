// Fill out your copyright notice in the Description page of Project Settings.


#include "Tanks/Player/TWPlayerPawn.h"

#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Tanks/Guns/TWGun.h"

ATWPlayerPawn::ATWPlayerPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	
	BodyMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("BodyMeshComponent");
	SetRootComponent(BodyMeshComponent);

	TurretMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>("TurretMeshComponent");
	TurretMeshComponent->SetupAttachment(BodyMeshComponent);

	GunPivotComponent = CreateDefaultSubobject<UArrowComponent>("GunPivotComponent");
	GunPivotComponent->SetupAttachment(TurretMeshComponent);

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(BodyMeshComponent);

	FollowCamera = CreateDefaultSubobject<UCameraComponent>("FollowCamera");
	FollowCamera->SetupAttachment(SpringArmComponent);
}

void ATWPlayerPawn::BeginPlay()
{
	Super::BeginPlay();

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Gun = GetWorld()->SpawnActor<ATWGun>
	(DefaultGunClass, GunPivotComponent->GetComponentLocation(), GunPivotComponent->GetComponentRotation(), SpawnParams);

	Gun->AttachToComponent(GunPivotComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	
}

void ATWPlayerPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PerformMovement(DeltaTime);

	PerformTurretRotation();
}

void ATWPlayerPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ATWPlayerPawn::OnMoveForwardInput);
	PlayerInputComponent->BindAxis("MoveRight", this, &ATWPlayerPawn::OnMoveRightInput);

}

void ATWPlayerPawn::PerformMovement(float DeltaTime)
{
	//Perform (Forward)
	if(FMath::IsNearlyZero(MoveForwardInput))
	{
		CurrentMoveSpeed = 0.f;
	}
	else
	{
		CurrentMoveSpeed = FMath::FInterpConstantTo(CurrentMoveSpeed, MoveSpeed * FMath::Abs(MoveForwardInput), DeltaTime, MoveAcceleration);
    
    	const FVector moveDelta = GetActorForwardVector() * CurrentMoveSpeed * DeltaTime * FMath::Sign(MoveForwardInput);
    	SetActorLocation(GetActorLocation() + moveDelta);	
	}
	//Perform (Rotation)
	if(FMath::IsNearlyZero(MoveRightInput))
	{
		CurrentRotationSpeed = 0.f;
	}
	else
	{
		CurrentRotationSpeed = FMath::FInterpConstantTo(CurrentRotationSpeed, RotationSpeed * FMath::Abs(MoveRightInput), DeltaTime, RotationAcceleration);
		const FRotator rotationDelta = FRotator(0.f, CurrentRotationSpeed * DeltaTime * FMath::Sign(MoveRightInput), 0.f);
		SetActorRotation(GetActorRotation() + rotationDelta);
	}
}

void ATWPlayerPawn::PerformTurretRotation()
{
	if(auto controller = Cast<APlayerController>(GetController()))
	{
		float mouseX, mouseY;
		controller->GetMousePosition(mouseX, mouseY);
		FVector mouseLocation, mouseDirection;
		controller-> DeprojectScreenPositionToWorld(mouseX, mouseY, mouseLocation, mouseDirection);

		//GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Orange, FString::Printf(TEXT("Mouse X: %.2f, Y: %.2f"),mouseX, mouseY));
		
		const FVector actorLocation = GetActorLocation();

		FVector correctMouseDirection = mouseLocation - FollowCamera->GetComponentLocation();
		correctMouseDirection.Normalize();

		FVector correctMouseLocation = mouseLocation + (correctMouseDirection * 1000.f);
		correctMouseLocation.Z = actorLocation.Z;
		//DrawDebugLine(GetWorld(), actorLocation, correctMouseLocation,FColor::Red, false, 0.f, 0, 5.f);

		FRotator newTurretRotation = UKismetMathLibrary::FindLookAtRotation
			(TurretMeshComponent->GetComponentLocation(), correctMouseLocation);
		newTurretRotation.Roll = TurretMeshComponent->GetComponentRotation().Roll;
		newTurretRotation.Pitch = TurretMeshComponent->GetComponentRotation().Pitch;
		TurretMeshComponent->SetWorldRotation(newTurretRotation);
	}
}

void ATWPlayerPawn::OnMoveForwardInput(float AxisValue)
{
	MoveForwardInput = AxisValue;
}

void ATWPlayerPawn::OnMoveRightInput(float AxisValue)
{
	MoveRightInput = AxisValue;
}

