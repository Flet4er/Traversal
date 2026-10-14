// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ParkourTypes.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EWallRunDirection : uint8
{
    None UMETA(DisplayName = "None"),
    Left UMETA(DisplayName = "Left"),
    Right UMETA(DisplayName = "Right"),
    Forward UMETA(DisplayName = "Forward"),
    Backward UMETA(DisplayName = "Backward")
};
