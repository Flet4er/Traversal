// Copyright Epic Games, Inc. All Rights Reserved.

#include "TraversalCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ParkourAbility.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ParkourTypes.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// ATraversalCharacter

ATraversalCharacter::ATraversalCharacter()
{
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
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

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

	DefCharSprintSpeed = GetCharacterMovement()->MaxWalkSpeed;

	//GAS
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComp"));
	AttributeSet = CreateDefaultSubobject<UMyAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* ATraversalCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ATraversalCharacter::BeginPlay()
{
	Super::BeginPlay();

	//init Ability
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		//FGameplayAbilitySpec Spec(UParkourAbility::StaticClass());
		
		//AbilitySystemComponent->GiveAbility(Spec);
		//ParkourAbilityHandle = AbilitySystemComponent->GiveAbility(Spec);
	}
}



//////////////////////////////////////////////////////////////////////////
// Input

void ATraversalCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ATraversalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATraversalCharacter::JumpActivate);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATraversalCharacter::Move);
		EnhancedInputComponent->BindAction(W_Action, ETriggerEvent::Started, this, &ATraversalCharacter::W_pressed);
		EnhancedInputComponent->BindAction(W_Action, ETriggerEvent::Completed, this, &ATraversalCharacter::W_completed);
		EnhancedInputComponent->BindAction(W_Action, ETriggerEvent::Canceled, this, &ATraversalCharacter::W_canceled);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATraversalCharacter::Look);

		// Sprinting
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Ongoing, this, &ATraversalCharacter::SprintOngoing);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ATraversalCharacter::SprintStarted);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ATraversalCharacter::SprintCanceled);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATraversalCharacter::SprintCompleted);

		//Parkour
		EnhancedInputComponent->BindAction(SpecialAction, ETriggerEvent::Triggered, this, &ATraversalCharacter::ParkourTriggered);
		//Fast Stance
		EnhancedInputComponent->BindAction(FastStanceAction, ETriggerEvent::Ongoing, this, &ATraversalCharacter::FastStanceOngoing);
		EnhancedInputComponent->BindAction(FastStanceAction, ETriggerEvent::Canceled, this, &ATraversalCharacter::FastStanceCanceled);
		EnhancedInputComponent->BindAction(FastStanceAction, ETriggerEvent::Completed, this, &ATraversalCharacter::FastStanceCompleted);
		EnhancedInputComponent->BindAction(FastStanceAction, ETriggerEvent::Started, this, &ATraversalCharacter::FastStanceStarted);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATraversalCharacter::Move(const FInputActionValue& Value)
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
		if (GetIsOnWallRun()||GetIsAfterWallJump())
			return;
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ATraversalCharacter::Look(const FInputActionValue& Value)
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

void ATraversalCharacter::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
	{
		SetIsAfterWallJump(false);
	}
}

void ATraversalCharacter::CancelWallRun()
{
	AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
}


//Getters and Setters
#pragma region Getters_Setters
bool ATraversalCharacter::GetWKeyIsPressed()
{
	return bIsW_KeyPressed;
}

void ATraversalCharacter::SetWKeyIsPressed(bool IsPressed)
{
	bIsW_KeyPressed = IsPressed;
}

void ATraversalCharacter::SetIsSprinting(bool Sprinting)
{
	bIsSprinting = Sprinting;
}

bool ATraversalCharacter::GetIsSprinting()
{
	return(bIsSprinting);
}

float ATraversalCharacter::GetDefCharWalkSpeed()
{
	return(DefCharSprintSpeed);
}

float ATraversalCharacter::GetCharStepHeight()
{
	return GetCharacterMovement()->MaxStepHeight;
}

float ATraversalCharacter::GetCharCapsuleHalfHeight()
{
	return GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
}

FVector ATraversalCharacter::GetCharForwardVector()
{
	return GetCapsuleComponent()->GetForwardVector();
}

float ATraversalCharacter::GetCharSpeed()
{	
	return GetCharacterMovement()->Velocity.Length();
}

bool ATraversalCharacter::GetIsInFastStance()
{
	return bIsInFastStance;
}

void ATraversalCharacter::SetIsInFastStance(bool isInFastStance)
{
	bIsInFastStance = isInFastStance;
}

bool ATraversalCharacter::GetIsOnWallRun()
{
	return bIsOnWallRun;
}

void ATraversalCharacter::SetIsOnWallRun(bool isOnWallRun)
{
	bIsOnWallRun = isOnWallRun;
}

bool ATraversalCharacter::GetIsOnLeftWallRun()
{
	return bIsOnLeftWallRun;
}

void ATraversalCharacter::SetIsOnLeftWallRun(bool isOnLeftWallRun)
{
	bIsOnLeftWallRun = isOnLeftWallRun;
}

bool ATraversalCharacter::GetIsFalling()
{
	return GetCharacterMovement()->IsFalling();
}

void ATraversalCharacter::SetCharCollisionAndMovementMode(ECollisionEnabled::Type Collision, EMovementMode MoveMode)
{
	GetCapsuleComponent()->SetCollisionEnabled(Collision);
	GetCharacterMovement()->SetMovementMode(MoveMode, 0);
}

bool ATraversalCharacter::GetIsAfterWallJump()
{
	return bIsAfterWallJump;
}

void ATraversalCharacter::SetIsAfterWallJump(bool IsAfterWallJUmp)
{
	bIsAfterWallJump = IsAfterWallJUmp;
}

bool ATraversalCharacter::GetIsOnVerticalWallRun()
{
	return bIsOnVerticalWallRun;
}
void ATraversalCharacter::SetIsOnVerticalWallRun(bool isOnVerticalWallRun)
{
	bIsOnVerticalWallRun = isOnVerticalWallRun;
}
#pragma endregion Getters_Setters

//Input Delegate
#pragma region InputDelegate
void ATraversalCharacter::SprintOngoing(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = CharSprintSpeed;

		SetIsSprinting(true);

		if (GetIsInFastStance()&& GetWKeyIsPressed())
		{
			AbilitySystemComponent->TryActivateAbility(WallRunAbilityHandle);			
		}
		else
		{
			if(GetIsOnWallRun())
				AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
		}
	}
}
// FOR Ghosting keyboards or for broke ass me with shity keyboard
void ATraversalCharacter::SprintStarted(const FInputActionValue& Value)
{
	/*
	if (Controller != nullptr)
	{
		if (GetIsSprinting())
		{
			GetCharacterMovement()->MaxWalkSpeed = DefCharSprintSpeed;
			SetIsSprinting(false);

			return;
		}
		GetCharacterMovement()->MaxWalkSpeed = CharSprintSpeed;

		SetIsSprinting(true);

		if (GetIsInFastStance() && GetWKeyIsPressed())
		{
			AbilitySystemComponent->TryActivateAbility(WallRunAbilityHandle);
		}
		else
		{
			if (GetIsOnWallRun())
				AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
		}
	}
	*/
}

void ATraversalCharacter::SprintCanceled(const FInputActionValue& Value)
{	
	if (Controller != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefCharSprintSpeed;

		SetIsSprinting(false);

		if (GetIsOnWallRun())
			AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
	}
}
void ATraversalCharacter::SprintCompleted(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		GetCharacterMovement()->MaxWalkSpeed = DefCharSprintSpeed;

		SetIsSprinting(false);

		if (GetIsOnWallRun())
			AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
	}
	
}

void ATraversalCharacter::ParkourTriggered(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbility(ParkourAbilityHandle);
	}
}

void ATraversalCharacter::FastStanceOngoing(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		SetIsInFastStance(true);
	}
}
void ATraversalCharacter::FastStanceCanceled(const FInputActionValue& Value)
{
		SetIsInFastStance(false);

		if (GetIsOnWallRun())
			AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
}
void ATraversalCharacter::FastStanceCompleted(const FInputActionValue& Value)
{
		SetIsInFastStance(false);

		if (GetIsOnWallRun())
			AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
}
void ATraversalCharacter::FastStanceStarted(const FInputActionValue& Value)
{
		SetIsInFastStance(true);	
}

void ATraversalCharacter::JumpActivate(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
 		if (GetIsOnWallRun())
		{
 			FGameplayEventData Payload;
			FGameplayTag WallJumpAbilityTag;

			//wall jump after wallrun
			if (GetIsOnVerticalWallRun())
			{
				AbilitySystemComponent->TryActivateAbility(WallJumpAbilityHandle);
			}
			else if (GetIsOnLeftWallRun())
			{
				Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallJump.Right"));

				AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
			}
			else
			{
				Payload.EventTag = FGameplayTag::RequestGameplayTag(FName("Ability.WallJump.Left"));

				AbilitySystemComponent->HandleGameplayEvent(Payload.EventTag, &Payload);
			}
		}
		else
		{
		// wallJumpActivate
		AbilitySystemComponent->TryActivateAbility(WallJumpAbilityHandle);
		}
	}
}

void ATraversalCharacter::W_pressed(const FInputActionValue& Value)
{
	float MovementVector = Value.Get<float>();
	UE_LOG(LogTemp, Warning, TEXT("W input is %f"), MovementVector);
	// when on wall run use this because regular input is blocked
	if (GetIsOnWallRun())
		AddMovementInput(GetCharForwardVector(), MovementVector);

	SetWKeyIsPressed(true);
}

void ATraversalCharacter::W_completed(const FInputActionValue& Value)
{
	SetWKeyIsPressed(false);
	if (GetIsOnWallRun())
		AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
}

void ATraversalCharacter::W_canceled(const FInputActionValue& Value)
{
	SetWKeyIsPressed(false);
	if (GetIsOnWallRun())
		AbilitySystemComponent->CancelAbilityHandle(WallRunAbilityHandle);
}

#pragma endregion InputDelegate
