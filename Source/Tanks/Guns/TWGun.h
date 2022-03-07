#pragma once

#include "GameFramework/Actor.h"

#include "TWGun.generated.h"

UCLASS()
class ATWGun : public AActor
{
	GENERATED_BODY()
	
public:
	ATWGun();
public:
	UPROPERTY(VisibleAnywhere)
	USceneComponent* TransformComponent;
	
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* GunMesh;
	
};

