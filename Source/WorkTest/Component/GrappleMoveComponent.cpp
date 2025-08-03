// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/GrappleMoveComponent.h"


#include "TextureAssetActions.h"
#include "Chaos/CollisionResolutionUtil.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h" // GetLifeTime~



// Sets default values for this component's properties
UGrappleMoveComponent::UGrappleMoveComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//bReplicates = true;
	// ...
}


// Called when the game starts
void UGrappleMoveComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// ...
	Owner = Cast<AAnby>(GetOwner());
	PC = Cast<APlayerController>(Owner->GetController());
	/*
	//StartPoint = Owner->GetMesh()->GetSocketLocation("as"); // 시작 위치 확인
	EMovementMode mode  = Owner->GetCharacterMovement()->MovementMode;
	FString ModeName;
	switch (mode)
	{
	case MOVE_Walking:        ModeName = TEXT("Walking"); break;
	case MOVE_NavWalking:     ModeName = TEXT("NavWalking"); break;
	case MOVE_Falling:        ModeName = TEXT("Falling"); break;
	case MOVE_Swimming:       ModeName = TEXT("Swimming"); break;
	case MOVE_Flying:         ModeName = TEXT("Flying"); break;
	case MOVE_Custom:         ModeName = TEXT("Custom"); break;
	case MOVE_None:           ModeName = TEXT("None"); break;
	default:                  ModeName = TEXT("Unknown"); break;
	}
	
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green,
		FString::Printf(TEXT("MovementMode: %s"), *ModeName));*/
}


// Called every frame
void UGrappleMoveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	//DrawDebugLine(GetWorld(), Owner->GetActorLocation(), GrappleTargetLocation, FColor::Black, false, 0.2f);
/*
	if (!bIsHooked || !bIsGrappling || !Owner || !Owner->HasAuthority()) return; // 이동은 서버에서만 실행되도록
	if (!bIsHooked) return; 
	if (bIsGrappling)
	{
		if (bCancelGrapple)
		{
			bIsGrappling =  false;
			return;
		}
		if (!IsValid(Owner)) return;
		
		GEngine->AddOnScreenDebugMessage(1, 0.2f, FColor::White,TEXT("Moving"));
		
		FVector CurrentLocation = Owner->GetActorLocation();
		FVector Direction = (GrappleTargetLocation - CurrentLocation).GetSafeNormal();
		FVector NewLocation = CurrentLocation + (Direction * GrappleSpeed * DeltaTime);
		
		//ElapsedTime += DeltaTime;
		//float AccelerationFactor = ElapsedTime * ElapsedTime; // t^2
		//float SpeedThisFrame = GrappleSpeed * AccelerationFactor;
		//FVector NewLocation = CurrentLocation + (Direction * SpeedThisFrame * DeltaTime);
			
		// 도착 체크
		if (FVector::Dist(NewLocation, GrappleTargetLocation) < 100.f)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::White,TEXT("도착"));
		}
		else
		{
			//DrawDebugLine(GetWorld(), CurrentLocation, GrappleTargetLocation, FColor::Black, false, 0.2f);
			Owner->SetActorLocation(NewLocation); // 현재 클라이언트 기준 위치변경
			ReplicatedLocation = NewLocation; // 서버로 값전달
		}
	}
	else
	{
		if (bIsHooked)
		{
			//GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::White,TEXT("매달려있기~"));
		}
	}
	*/


	// 새로운 이동방식 구현해보기

	if (!bIsHooked) return; 
	if (bIsGrappling) // 그래플 이동시작
	{
		if (bCancelGrapple)
		{
			bIsGrappling =  false;
			return;
		}

		GEngine->AddOnScreenDebugMessage(1, 0.2f, FColor::White,TEXT("Moving"));
		
		FVector CurrentLocation = Owner->GetActorLocation();
		FVector Direction = (GrappleTargetLocation - CurrentLocation).GetSafeNormal();
		GrappleDirection = Direction;
		FGrappleMove Move = CreateMove(DeltaTime);

		
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Yellow,TEXT("Send Move"));
			LastMove = Move;
			SimulateMove(Move);
		}
		
		/*
		// 1. 현재 조작중인 캐릭터 (모두)
		if (GetOwnerRole() == ROLE_AutonomousProxy)
		{
			LastMove = Move;
			SimulateMove(Move);
		}
		// 2. 서버가 SimulateProxy를 가진 클라캐릭터( 클라에서 조작중인 다른 유저)
		if (GetOwnerRole( == ROLE_SimulatedProxy && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
		{
			
		}
		if (GetOwnerRole()== ROLE_Authority ||)
		//if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() ==  ROLE_SimulatedProxy)
		{
			GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::White,TEXT("Send Move"));
			//DrawDebugSphere(GetWorld(), Force, 32.0f, 16.0f, FColor::Red, false, 0.2f);
			
			// 1. 틱마다 Createmove로 해당 Move를 바탕으로 SimulateMove해서 로컬에서 움직임 계산함
			LastMove = CreateMove(DeltaTime);
			SimulateMove(LastMove);
		}
		//FVector NewLocation = CurrentLocation + (Direction * GrappleSpeed * DeltaTime);
	}*/
	}
}




void UGrappleMoveComponent::CheckGrappleHitPoint()
{
	if (bIsHooked) return;
	int32 ViewportX =0, ViewportY =0;
	PC->GetViewportSize(ViewportX, ViewportY);
	FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportX /2, ViewportY/ 2,WorldLocation,WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + (WorldDirection * MaxGrappleDistance);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner); // 자기 자신은 제외시키기
		
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.0f, 16, FColor::Green, false, inRate);
			DrawDebugLine(GetWorld(), Start, Hit.ImpactPoint, FColor::Green, false, inRate);
		}
		else
		{
			DrawDebugLine(GetWorld(), Start, End, FColor::White, false, inRate);
		}

	}
}

void UGrappleMoveComponent::ServerFireGrapple_Implementation(const FVector& Start, const FVector& End)
{
	
}
void UGrappleMoveComponent::FireGrapple()
{
	if (!Owner || !PC) return;
	
	// 단순 bool 검사는 클라검사. 
	if (bIsHooked || (Owner && Owner->HasAuthority() && bIsHooked)) 
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("이미 꽂혀있음 - 서버/ 클라차단"));
		return;
	}


	//if (!Owner || !Owner->IsLocallyControlled()) return;
	//FVector Start = Owner->GetActorLocation() + Owner->GetActorForwardVector() * 100.f;
	//FVector End = Start+Owner->GetActorForwardVector() * 3000.0f;

	//ServerFireGrapple(Start, End);
	
	// Deproject 화면 중앙 -> 월드방향
	int32 ViewportX =0, ViewportY =0;
	PC->GetViewportSize(ViewportX, ViewportY);
	FVector WorldLocation, WorldDirection;
	if (PC->DeprojectScreenPositionToWorld(ViewportX /2, ViewportY/ 2,WorldLocation,WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + (WorldDirection * MaxGrappleDistance);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Owner); // 자기 자신은 제외시키기
		
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit)
		{
			//if (Owner == nullptr) return;
			if (!Owner || !Owner->IsLocallyControlled()) return;
			//FVector NormalDirection = ( Owner->GetActorLocation() -  Hit.ImpactPoint).GetSafeNormal();
			GrappleTargetLocation =  Hit.Location + (Hit.ImpactNormal * 30);
			
			GrappleCreatePoint = Hit.Location + (Hit.ImpactNormal * 10);	 
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Grapple Hit"));
			DrawDebugSphere(GetWorld(), GrappleTargetLocation, 64.0f, 16, FColor::Red, false, 2.0f);
			DrawDebugSphere(GetWorld(), GrappleCreatePoint, 32.0f, 16, FColor::Blue, false, 3.0f);
			SpawnGrappleActor(GrappleCreatePoint);
			// 그래플 발사는 함. 맞은곳에서 동작
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Grapple NoHit"));
			//DrawDebugLine(GetWorld(), Start, End, FColor::Red, false, 2.0f);
			// 그래플 발사 일단 방향대로 쏨
		}

	}
}

void UGrappleMoveComponent::StartPreview()
{
	if (bIsHooked) return;
	GetWorld()->GetTimerManager().SetTimer(
	GrapplePreviewHandle,
	this,
	&UGrappleMoveComponent::CheckGrappleHitPoint,
	inRate,
	true
	);
	
	//Owner->ChangeView();
	
	//CallSpringArmStateChange(EViewType::FirstSight);
	//Owner->ChangeViewType_First();
	//Owner->changeview
	//if (Owner!= nullptr) Owner->ChangeViewType(EViewType::FirstSight);
}

void UGrappleMoveComponent::StopPreview()
{
	GetWorld()->GetTimerManager().ClearTimer(GrapplePreviewHandle);
}


void UGrappleMoveComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UGrappleMoveComponent, bIsGrappling); // 해당 변수를 Replcation 대상으로 지정
	DOREPLIFETIME(UGrappleMoveComponent, GrappleTargetLocation); // 언리얼이 내부적으로 서버에서 클라로 값을 전송함
	DOREPLIFETIME(UGrappleMoveComponent, GrappleCreatePoint);
	DOREPLIFETIME(UGrappleMoveComponent, ReplicatedLocation);
	DOREPLIFETIME(UGrappleMoveComponent, GrappleActor);
	DOREPLIFETIME(UGrappleMoveComponent, bIsHooked);
}



void UGrappleMoveComponent::SpawnGrappleActor_Implementation(const FVector& _location)
{
	if (GrapplePointObject ==nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("GrapplePointActor is Null"));
		return;
	}

	// SpawnActor 시 서버에서만 실행되도록 보장.
	// 복제되는 액터는 서버에서만 생성되도록. 
	if (!Owner->HasAuthority()) return;

	bIsHooked = true;
	FRotator CreateRot = FRotationMatrix::MakeFromX((GrappleCreatePoint -Owner->GetActorLocation()).GetSafeNormal()).Rotator();
	GrappleActor = GetWorld()->SpawnActor<AActor>(GrapplePointObject, _location, CreateRot); // FRotator::ZeroRotator
	AAnby* anby = Cast<AAnby>(Owner);
	anby->CableAttachToMovePoint(GrappleActor);
}

void UGrappleMoveComponent::DestroyGrappleActor_Implementation()
{
	if (GrappleActor == nullptr	)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("GrappleActor = null"));
		return;
	}

	
	Owner->GrappleCable->SetVisibility(false);
	SetGravity(true, 2.0f);
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, TEXT("Destroy Actor"));
	bIsHooked= false;
	
	GrappleActor->Destroy();
	GrappleActor = nullptr;
	GrappleTargetLocation = FVector::ZeroVector;
	
	if (bIsGrappling)
	{
		 // Shift가 눌리고 있는지 검사해서 함
		FVector outFec;
		FRotator outRot;
		Owner->GetController()->GetPlayerViewPoint(outFec, outRot);
		FVector dir = outRot.Vector();
		//GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, FString::Printf(TEXT("Camera Direction: %s"), *dir.ToString()));
		//DrawDebugLine(GetWorld(), Owner->GetActorLocation(), Owner-> GetActorLocation()+ dir *  100.0f, FColor::Blue,false, 2.0f);
		//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Yellow, TEXT("취소 발사 슝"));
		Owner->GrappleCancelLaunch(dir, 2000.0f);
	}
	//Owner->ChangeView();
}

void UGrappleMoveComponent::StartGrappleTo_Implementation()
{
	//GrappleTargetLocation = Target;
	bIsGrappling = true;
	bCancelGrapple = false;
	SetGravity(false);

}
void UGrappleMoveComponent::CancelGrapple_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("CancelGrapple"));
	ElapsedTime=  0.0f;
	bCancelGrapple=  true;
	SetGravity(true, 2.0f);
}

void UGrappleMoveComponent::StartGrapple()
{
	//GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Green, TEXT("StartGrapple"));
	StartGrappleTo();
}

// 이 함수에서 위치 조정시킴
void UGrappleMoveComponent::OnRep_GrappleLocation()
{
	if (Owner->HasAuthority()) return;
	
	// 새로 생긴부분
	//서버의 위치를 따라가는식으로 함
	FVector CurrentLocation = Owner->GetActorLocation();
	FVector Interpolated = FMath::VInterpTo(CurrentLocation, ReplicatedLocation, GetWorld()->GetDeltaSeconds(), 12.f);
	Owner->SetActorLocation(Interpolated);
	//
	
	//Owner->SetActorLocation(ReplicatedLocation); 
}

void UGrappleMoveComponent::SetGravity(bool _enable, float _amount)
{
	if (_enable)
	{
		// 중력 활성화
		Owner->GetCharacterMovement()->GravityScale  =_amount;
		Owner->GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		// 중력 비활성화
		Owner->GetCharacterMovement()->GravityScale  = 0.0f;
		Owner->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
	}
}

AActor* UGrappleMoveComponent::GetGrappleActor()
{
	if (GrappleActor == nullptr) return nullptr;
	return GrappleActor;
}

void UGrappleMoveComponent::SimulateMove(const FGrappleMove& _Move)
{
	/*
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throtlle;

	Force += GetAirResistance(); 
	Force += GetRollingResistance();

	FVector Acceleration = Force/Mass;
	Velocity = Velocity + Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime,Move.SteeringThrow);
	UpdateLocationFromVelocity(Move.DeltaTime);*/

	FVector Force = _Move.Direction * GrappleSpeed;

	
	FVector Acceleration = Force/Mass;
	Velocity = Velocity + Acceleration * _Move.DeltaTime;
	//DrawDebugSphere(GetWorld(), Velocity, 32.0f, 16.0f, FColor::Blue, false, 0.2f);
	//ApplyRotation(_Move.DeltaTime, )
	UpdateLocationFromVelocity(_Move.DeltaTime);
}


FGrappleMove UGrappleMoveComponent::CreateMove(float DeltaTime)
{
	FGrappleMove Move;
	Move.DeltaTime = DeltaTime;
	//Move.SteeringThrow = SteeringThrow;
	//Move.Throtlle = Throttle;
	Move.Time = GetWorld()->TimeSeconds;
	Move.Direction = GrappleDirection;
	Move.TargetLocation = GrappleTargetLocation;
	
	return Move;
	
}

FVector UGrappleMoveComponent::GetAirResistance()
{
	return FVector::ZeroVector;
}

FVector UGrappleMoveComponent::GetRollingResistance()
{
	return FVector::ZeroVector;
}

void UGrappleMoveComponent::ApplyRotation(float DeltaTime, float SteeringThrow)
{
	//float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float DeltaLocation = FVector::DotProduct(LastMove.Direction, Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * SteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	Velocity = RotationDelta.RotateVector(Velocity); // 방향을 현재 Velocity에 할당

	GetOwner()->AddActorWorldRotation(RotationDelta);
}

void UGrappleMoveComponent::UpdateLocationFromVelocity(float DeltaTime)
{
	FVector Translation = Velocity * 100 * DeltaTime;

	GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::White,TEXT("UpateLocationFromVelocity"));
	/*
	FHitResult Hit;
	
	GetOwner()->AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit())
	{
		Velocity = FVector::Zero();
	}
*/
	// Direction으로 알아봅시다
	if (FVector::Dist(Velocity, GrappleTargetLocation) < 100.f)
	{
		GEngine->AddOnScreenDebugMessage(1, 0.5f, FColor::White,TEXT("도착"));
		Velocity = FVector::ZeroVector;
	}
}

