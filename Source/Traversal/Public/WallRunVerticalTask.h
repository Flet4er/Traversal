// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h" 
#include <Traversal/TraversalCharacter.h>
#include "WallRunVerticalTask.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWallRunVerticalFinished);
#define ECC_WallRun ECC_GameTraceChannel2
/**
 * 
 */
UCLASS()
class TRAVERSAL_API UWallRunVerticalTask : public UAbilityTask
{
	GENERATED_BODY()

public:
	UWallRunVerticalTask();

	float MaxDistance;
	float RunSpeed;
	float WallDetection;

	UPROPERTY(BlueprintAssignable)
	FOnWallRunVerticalFinished OnFinished;

protected:

	virtual void TickTask(float DeltaTime) override;
	virtual void Activate() override;
	FVector StartingLocationg;

	void StopRunning();

	UPROPERTY()
	ATraversalCharacter* Character;
	UPROPERTY()
	UPawnMovementComponent* MoveComponent;

private:
	FHitResult FHit;
	FVector Start;
	FVector End;
	FCollisionQueryParams QueryParam;
	bool bHit = false;
	float CapsuleHalfHeight;
};
