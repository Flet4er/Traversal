// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunVerticalTask.h"
#include <Traversal/TraversalCharacter.h>
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetTraceUtils.h"


UWallRunVerticalTask::UWallRunVerticalTask()
{
	bTickingTask = true;
}

void UWallRunVerticalTask::TickTask(float DeltaTime)
{
	Super::TickTask(DeltaTime);
	if (!GetAvatarActor())
	{
		StopRunning();
		return;
	}

	Start = Character->GetActorLocation() - FVector(0, 0, CapsuleHalfHeight-10);
	End = Character->GetActorForwardVector() * WallDetection + Start;

	bHit = GetWorld()->LineTraceSingleByChannel(FHit, Start, End, ECollisionChannel::ECC_WallRun, QueryParam);
	//DrawDebugLineTraceSingle(GetWorld(), Start, End, EDrawDebugTrace::Persistent, bHit, FHit, FColor::Green, FColor::Green, 10);
	if (!bHit)
	{
		StopRunning();
	}



	float Distance = (GetAvatarActor()->GetActorLocation() - StartingLocationg).Length();
	if (Distance > MaxDistance)
	{
		StopRunning();
	}
}

void UWallRunVerticalTask::Activate()
{
	Super::Activate();

	if (GetAvatarActor())
	{
		Character = Cast<ATraversalCharacter>(GetAvatarActor());
		MoveComponent = Character->GetMovementComponent();
		//remember starting location for calculation of max distance run
		StartingLocationg = Character->GetActorLocation();
		CapsuleHalfHeight = Character->GetCharCapsuleHalfHeight();

		if (MoveComponent)
		{
			Character->GetCharacterMovement()->bIgnoreClientMovementErrorChecksAndCorrection = true;
			FVector Forward = Character->GetCharForwardVector();
			//recalculate velocity to vector
			MoveComponent->Velocity = FVector::UpVector * RunSpeed;
		}
	}
}

void UWallRunVerticalTask::StopRunning()
{
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		OnFinished.Broadcast();
		EndTask();
		UE_LOG(LogTemp, Log, TEXT("StopVerticalWallRunning"));
	}
}