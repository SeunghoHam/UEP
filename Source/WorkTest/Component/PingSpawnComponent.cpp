// Fill out your copyright notice in the Description page of Project Settings.


#include "Component/PingSpawnComponent.h"
#include "PingActor.h"
#include "PingWidget.h"
#include "Anby.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UPingSpawnComponent::UPingSpawnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	//PingWidgetInstance = CreateDefaultSubobject<UPingWidget>(TEXT("PingWidget"));
	// ...
}


// Called when the game starts
void UPingSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	anby = Cast<AAnby>(GetOwner());
	Controller = anby->GetLocalViewingPlayerController();

	if (Controller == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("PingComponent Controller = null"));
		return;
	}

	if (anby == nullptr) return;
	if (PingActorClass ==nullptr) return;
	SpawnPing();
}


void UPingSpawnComponent::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPingSpawnComponent, PingLocation);
}

// Called every frame
void UPingSpawnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

void UPingSpawnComponent::PreviewStart()
{
	GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("PreviewStart"));
	bIsShowPreview = true;
	if (anby) anby->SetPingPreviewActive(true);
	if (PingActorInstance == nullptr) return;
	
	PingActorInstance->PreviewStart();// 이거 아직 하는거없음
	
	GetWorld()->GetTimerManager().SetTimer(
		PreviewHandle,
		this,
		&UPingSpawnComponent::PreviewShow,
		inRate,
		true
	);
}

void UPingSpawnComponent::PreviewEnd()
{
	PrevDisable();
}

void UPingSpawnComponent::PreviewShow()
{
	int32 ViewportX = 0, ViewportY = 0;
	Controller->GetViewportSize(ViewportX, ViewportY);
	FVector WorldLocation, WorldDirection;
	if (Controller->DeprojectScreenPositionToWorld(ViewportX / 2, ViewportY / 2, WorldLocation, WorldDirection))
	{
		FVector Start = WorldLocation;
		FVector End = Start + (WorldDirection * 1000.0f);

		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(anby); // 자기 자신은 제외시키기
		Params.AddIgnoredActor(PingActorInstance);


		DrawDebugLine(GetWorld(), Start, End, FColor::White);
		bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECollisionChannel::ECC_Visibility, Params);
		if (bHit)
		{
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Magenta, FString::Printf(TEXT("Hit Actor : %s"),
				*Hit.GetActor()->GetName()));
			SpawnLocation = Hit.Location + (Hit.ImpactNormal * 100.0f);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(1, 1.0f, FColor::Red, TEXT("No hit"));
			SpawnLocation = End;
		}

		FRotator Rot = UKismetMathLibrary::FindLookAtRotation(SpawnLocation, Start);
		//Rot.Roll += 45;
		//PingActorInstance->SetActorLocationAndRotation(SpawnLocation,Rot);
		if (bIsShowPreview)
		{
			PingLocation = SpawnLocation;
			float Distance = FVector::Dist(Start,SpawnLocation);
			
			PingActorInstance->SetActorLocation(PingLocation);
			if (PingWidget) PingWidget->GetDistaneOwnerToWidget(Distance);	
			//PingActorInstance->GetDistaneOwnerToWidget(Distance);
		}
		
		PingActorInstance->SetActorRotation(Rot);
	}
}

void UPingSpawnComponent::PingChooseStart()
{
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("핑을 골라보아요"));
	if (Controller == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("컨트롤러 없음"));
		return;
	}
}

void UPingSpawnComponent::PingChooseEnd()
{
	PrevDisable();
}

void UPingSpawnComponent::PrevDisable()
{
	if (PingWidget)
	{
		PingWidget->ServerPing();
		// 서버 동기화 동작
		ServerSendPing(PingLocation);
	}
	// 미리 변경할거
	if (anby) anby->SetPingPreviewActive(false);
	bIsShowPreview = false;
	// 일정시간 뒤에 적용할거
	GetWorld()->GetTimerManager().SetTimer(
	MouseDirectionTimerHandle,
	this,               
	&UPingSpawnComponent::DisablePing,     
	1.0f,
	false);
}

void UPingSpawnComponent::DisablePing()
{
	if (PingActorInstance == nullptr) return;
	
	if (GetWorld()->GetTimerManager().IsTimerActive(MouseDirectionTimerHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(MouseDirectionTimerHandle);
	}
	if (GetWorld()->GetTimerManager().IsTimerActive(PreviewHandle))
	{
		GetWorld()->GetTimerManager().ClearTimer(PreviewHandle);
	}
	
	bIsShowPreview = false;
	PingActorInstance->PreviewEnd();
	if (PingWidget) PingWidget->ImageInitailzie();
	YawValue = 0;
	PitchValue = 0;
}

void UPingSpawnComponent::GetDirection(float _yaw, float _pitch)
{
	float MaxValue = 10.0f;
	YawValue = FMath::Clamp(YawValue += _yaw, -MaxValue, MaxValue);
	PitchValue = FMath::Clamp(PitchValue += _pitch, -MaxValue, MaxValue);

	// 1차 절대값비교
	if (FMath::Abs(YawValue) > FMath::Abs(PitchValue) && FMath::Abs(YawValue) > 3.0f)
	{
		if (YawValue > 0)
		{
			// 오른쪽
			PingWidget->SetPingType(EPingTypes::Go);
		}  
		else
		{
			// 왼쪽
			PingWidget->SetPingType(EPingTypes::Missing);
		}
	}
	else if ( FMath::Abs(PitchValue) > 3.0f)
	{
		if (PitchValue > 0)
		{
			// 아래
			PingWidget->SetPingType(EPingTypes::Help);
		}
		else
		{
			// 위
			PingWidget->SetPingType(EPingTypes::Danger);
		}
	}
	else
	{
		PingWidget->SetPingType(EPingTypes::Basic);
	}
}


void UPingSpawnComponent::SpawnPing()
{
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, TEXT("SpawnPing"));
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	PingActorInstance = GetWorld()->SpawnActor<APingActor>(PingActorClass,
														   anby->GetActorLocation() + (anby->GetCameraForwardVector() * 200),
														   FRotator::ZeroRotator, SpawnParams);
	if (PingWidget == nullptr)
	{
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::White, TEXT("PingWidget 겟또다제"));
		
		PingWidget = Cast<UPingWidget>(PingActorInstance->WidgetInstance->GetUserWidgetObject());
	}
}

void UPingSpawnComponent::ServerSendPing_Implementation(const FVector& Location)
{
	MulticastSpawnPing(Location);
}

void UPingSpawnComponent::MulticastSpawnPing_Implementation(const FVector& Location)
{
	PingActorInstance->SetActorLocation(Location);
	//PingActorInstance = GetWorld()->SpawnActor<APingActor>(PingActorClass, Location, FRotator::ZeroRotator);
}
