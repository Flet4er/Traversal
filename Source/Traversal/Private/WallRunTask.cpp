// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunTask.h"
#include <Traversal/TraversalCharacter.h>
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetTraceUtils.h"


// Sets default values for this component's properties
UWallRunTask::UWallRunTask()
{
	bTickingTask = true;
}

void UWallRunTask::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);

	if (!GetAvatarActor())
	{
		StopRunning();
		return;
	}

	Start = Character->GetActorLocation();
	EndRight = Character->GetActorRightVector() * WallDetection + Start;
	EndLeft = Character->GetActorRightVector() * WallDetection * -1 + Start;

	if (bIsLeftRun)
	{
		//LeftTrace
		bHitLeft = GetWorld()->LineTraceSingleByChannel(FHitLeft, Start, EndLeft, ECollisionChannel::ECC_WallRun, QueryParam);
		//DrawDebugLineTraceSingle(GetWorld(), Start, EndLeft, EDrawDebugTrace::Persistent, bHitLeft, FHitLeft, FColor::Green, FColor::Green, 10);
		if (!bHitLeft)
		{
			StopRunning();
		}
	}
	else
	{
		//RightTrace
		bHitRight = GetWorld()->LineTraceSingleByChannel(FHitRight, Start, EndRight, ECollisionChannel::ECC_WallRun, QueryParam);
		//DrawDebugLineTraceSingle(GetWorld(), Start, EndRight, EDrawDebugTrace::Persistent, bHitRight, FHitRight, FColor::Green, FColor::Green, 10);
		if (!bHitRight)
		{
			StopRunning();
		}
	}


	float Distance = (GetAvatarActor()->GetActorLocation() - StartingLocationg).Length();
	if (Distance > MaxDistance)
	{
		StopRunning();
	}
}

void UWallRunTask::Activate()
{
	Super::Activate();
	if (GetAvatarActor())
	{		
		Character = Cast<ATraversalCharacter>(GetAvatarActor());
		MoveComponent = Character->GetMovementComponent();
		//remember starting location for calculation of max distance run
		StartingLocationg = Character->GetActorLocation();

		if (MoveComponent)
		{			
			Character->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection=true;
			FVector Forward = Character->GetCharForwardVector();
			//recalculate velocity to vector
			MoveComponent->Velocity = (Forward * RunSpeed) + (FVector::UpVector * ClimbSpeed);
		}
	}
}

void UWallRunTask::StopRunning()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinished.Broadcast();
		EndTask();
		UE_LOG(LogTemp, Log, TEXT("StopWallRunning"));
	}
}


