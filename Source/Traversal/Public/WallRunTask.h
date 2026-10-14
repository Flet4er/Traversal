// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h" 
#include <Traversal/TraversalCharacter.h>
#include "WallRunTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallRunFinished);
#define ECC_WallRun ECC_GameTraceChannel2
/**
 *
 */
UCLASS()
class TRAVERSAL_API UWallRunTask : public UAbilityTask
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWallRunTask();
	float MaxDistance;
	float ClimbSpeed;
	float RunSpeed;
	float WallDetection;
	bool bIsLeftRun;

	UPROPERTY(BlueprintAssignable)
	FOnWallRunFinished OnFinished;
protected:
	virtual void TickTask(float DeltaTime) override;
	virtual void Activate() override;
	FVector StartingLocationg;

	UPROPERTY()
	ATraversalCharacter* Character;
	UPROPERTY()
	UPawnMovementComponent* MoveComponent;

	void StopRunning();
private:

	FHitResult FHitRight;
	FHitResult FHitLeft;
	FVector Start;
	FVector EndLeft;
	FVector EndRight;
	FCollisionQueryParams QueryParam;
	bool bHitRight = false;
	bool bHitLeft = false;	

};
