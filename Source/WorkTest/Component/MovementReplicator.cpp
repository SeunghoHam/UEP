// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/MovementReplicator.h"

#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UMovementReplicator::UMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
	// ...
}


// Called when the game starts
void UMovementReplicator::BeginPlay()
{
	Super::BeginPlay();


	GrappleMoveComponent = GetOwner()->FindComponentByClass<UGrappleMoveComponent>();
	// ...
	
}

void UMovementReplicator::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UMovementReplicator, ServerState);
}


// Called every frame
void UMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GrappleMoveComponent == nullptr) return;

	// 클라이언트 A 기준
	// GetOwnerRole -> Role_AutonomouseProxy - 클라(내가 조작)
	// GetRemoteRole -> Role_SimulateProxy - 서버에서는 애를 시뮬레이션할 캐릭터로 봄

	//서버에서 실행
	// GetOwnerRole -> Role_Authority - 내가 캐릭터를 소유(권한)하고 있음
	// GetRemoteRole -> Role_AutonomousProxy || Role_simulatedProxy 직접 조작 or관전

	// PlayerCtonrller->IsLocalController() = 내 클라이언트에서만 처리할 코드 (예 - ping)

	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Blue, TEXT("Replicator : Tick1"));
	FGrappleMove LastMove = GrappleMoveComponent->GetLastMove();
	if (GetOwnerRole() == ROLE_AutonomousProxy) // 본인이 조작하는 캐릭터라면?(클라)
	{
	GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::Blue, TEXT("Replicator : Tick2"));
		// 클라이언트 환경이라면 ( 미적용움직임 배열에 추가하고, 서버로 마지막 움직인 정보를 넘김)
		UnacknowledgeMoves.Add(LastMove);
		Server_SendMove(LastMove);
	}

	// Role = 내가 누구냐
	// RemoteRole = 상대방이 나를 누구로 생각하느냐
	
	if (GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy) // 서버가 실행(이 캐릭터를 클라에서 시뮬레이션 중)
	{
		// 서버에서만 true. 다른 클라이언트에서 시뮬레이션 되고 있다는 의미
		// 서버는 이 상태를 기억해서 복제해야함
		UpdateServerState(LastMove);
	}

	
	if (GetOwnerRole() == ROLE_SimulatedProxy) // 클라 실행(내가 직접 조작하지 않는 캐릭터)
	{
		// 클라에서만 true. 이 캐릭터가 보인 소유가 아닌, 다른 플레이어를 대신 보여주는 캐릭터.
		// 보간 등을 통해서 맞춰야함
		ClientTick(DeltaTime); 
	}
	
	// ...
}

void UMovementReplicator::ClearAcknowledgeMoves(FGrappleMove& LastMove)
{
	TArray<FGrappleMove> NewMoves;
	for (const FGrappleMove& Move : UnacknowledgeMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}
}

void UMovementReplicator::UpdateServerState(const FGrappleMove& Move)
{
	ServerState.LastMove=  Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = GrappleMoveComponent->GetVelocity();
}

void UMovementReplicator::ClientTick(float DeltaTime)
{
	ClientTimesinceUpdate += DeltaTime;
	if (ClientTimeBetweenLastUpdates < KINDA_SMALL_NUMBER) return; // 너무 작은값이 되지 않도록 방어하기
	// 너무 작은값이 들어오면 비교가 안됨
	if (GrappleMoveComponent == nullptr) return;

	float LerpRatio = ClientTimesinceUpdate / ClientTimeBetweenLastUpdates;
	FHermiteCubicSpline Spline = CreateSpline();

	InterpolateLocation(Spline, LerpRatio);

	InterpolateVelocity(Spline, LerpRatio);
	
	InterpolateRotation(LerpRatio);
}

FHermiteCubicSpline UMovementReplicator::CreateSpline()
{
	FHermiteCubicSpline Spline;
	Spline.TargetLocation = ServerState.Transform.GetLocation();
	Spline.StartLocation = ClientStartTransform.GetLocation();
	Spline.StartDerivative = ClientStartVelocity * VelocityToDerivative();
	Spline.TargetDerivative = ServerState.Velocity * VelocityToDerivative();
	return Spline;
}

void UMovementReplicator::InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio)
{
	FVector NewLocation = Spline.InterpolateLocation(LerpRatio);
	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldLocation(NewLocation);
	}
}

void UMovementReplicator::InterpolateRotation(float LerpRatio)
{
	// 서버가 가지고 있는 회전정보랑, 현재 클라이언트가 가지고 있는 회전정보를 비교해서 보간
	FQuat TargetRotation = ServerState.Transform.GetRotation();
	FQuat StartRotation = ClientStartTransform.GetRotation();

	FQuat NewRotaion  = FQuat::Slerp(StartRotation, TargetRotation, LerpRatio);

	if (MeshOffsetRoot != nullptr)
	{
		MeshOffsetRoot->SetWorldRotation(NewRotaion);
	}
	
}

void UMovementReplicator::InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio)
{
	FVector NewDerivative = Spline.InterpolateLocation(LerpRatio);
	FVector NewVelocity = NewDerivative / VelocityToDerivative();
	GrappleMoveComponent->SetVelocity(NewVelocity);
}

float UMovementReplicator::VelocityToDerivative()
{
		return ClientTimeBetweenLastUpdates * 100;
}

void UMovementReplicator::OnRep_ServerState()
{
	switch (GetOwnerRole())
	{
	case ENetRole::ROLE_AutonomousProxy:
		// 서버 위치로 보정 + 서버가 인정한 move까지 지우고, 나머지 다시 시뮬레이션
		AutonomousProxy_OnRep_ServerState();
		break;
		
		case ENetRole::ROLE_SimulatedProxy:
		// 위치/속도 보간용 저장
		SimulatedProxy_OnRep_ServerState();
		break;
		
		default:
		break;
	}
}

void UMovementReplicator::AutonomousProxy_OnRep_ServerState()
{

	if (GrappleMoveComponent == nullptr) return;
	GetOwner()->SetActorTransform(ServerState.Transform);
	GrappleMoveComponent->SetVelocity(ServerState.Velocity);

	
	ClearAcknowledgeMoves(ServerState.LastMove);
	// 남아있는 Move들 적용하기
	for (const FGrappleMove& Move : UnacknowledgeMoves)
	{
		GrappleMoveComponent->SimulateMove(Move);
	}
}

void UMovementReplicator::SimulatedProxy_OnRep_ServerState()
{
	if (GrappleMoveComponent == nullptr) return;
	ClientTimeBetweenLastUpdates = ClientTimesinceUpdate;
	ClientTimesinceUpdate = 0;

	if (MeshOffsetRoot != nullptr)
	{
		ClientStartTransform.SetLocation(MeshOffsetRoot->GetComponentLocation());
		ClientStartTransform.SetRotation(MeshOffsetRoot->GetComponentQuat());
	}
	ClientStartVelocity = GrappleMoveComponent->GetVelocity();

	GetOwner()->SetActorTransform(ServerState.Transform);
}

void UMovementReplicator::Server_SendMove_Implementation(FGrappleMove Move)
{
	if (GrappleMoveComponent == nullptr) return;

	// SimulateMove로 서버에서 다시 계산함.
	// ServerState를 그걸 기반으로 갱신
	ClientSimulatedTime += Move.DeltaTime;
	GrappleMoveComponent->SimulateMove(Move);

	UpdateServerState(Move);
	
}

