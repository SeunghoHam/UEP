// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GrappleMoveComponent.h"
#include "MovementReplicator.generated.h"


USTRUCT()
struct FGrappleMoveState
{
	// 움직임 및 방향을 가지고 있는 구조체
	GENERATED_BODY()
	
	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FGrappleMove LastMove; // 마지막 움직임 가져와서 뭐함?:
};

struct FHermiteCubicSpline
{
	FVector StartLocation, StartDerivative, TargetLocation, TargetDerivative;

	FVector InterpolateLocation(float LerpRatio) const
	{
		return FMath::CubicInterp(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
	FVector InterpolateDerivative(float LerpRatio) const
	{
		return FMath::CubicInterpDerivative(StartLocation, StartDerivative, TargetLocation, TargetDerivative, LerpRatio);
	}
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WORKTEST_API UMovementReplicator : public UActorComponent
{
	GENERATED_BODY()



public:	
	// Sets default values for this component's properties
	UMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	void ClearAcknowledgeMoves(FGrappleMove& LastMove);

	void UpdateServerState(const FGrappleMove& Move);

	void ClientTick(float DeltaTime);
	FHermiteCubicSpline CreateSpline();
	void InterpolateLocation(const FHermiteCubicSpline& Spline, float LerpRatio);
	void InterpolateRotation(float LerpRatio);
	void InterpolateVelocity(const FHermiteCubicSpline& Spline, float LerpRatio);
	float VelocityToDerivative(); //도함수 받아냄 ( 남은시간 * 100)

	UFUNCTION(Server, Reliable)
	void Server_SendMove(FGrappleMove Move);

	UPROPERTY(ReplicatedUsing=OnRep_ServerState)
	FGrappleMoveState ServerState;

	UFUNCTION()
	void OnRep_ServerState();

	void AutonomousProxy_OnRep_ServerState();
	void SimulatedProxy_OnRep_ServerState();

	TArray<FGrappleMove> UnacknowledgeMoves;

	float ClientTimesinceUpdate;
	float ClientTimeBetweenLastUpdates;
	FTransform ClientStartTransform;
	FVector ClientStartVelocity;

	float ClientSimulatedTime; // 치트검사용

	UPROPERTY()
	UGrappleMoveComponent* GrappleMoveComponent;

	UPROPERTY()
	USceneComponent* MeshOffsetRoot;
	UFUNCTION()
	void SetMeshOffsetRoot(USceneComponent* Root) { MeshOffsetRoot = Root;}
	
	
};
