// Fill out your copyright notice in the Description page of Project Settings.


#include "Anby/PingActor.h"
// Sets default values
APingActor::APingActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false); // 이미 각 클리언트가 생성

	PingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PingMesh"));
	RootComponent = PingMesh;
	//PingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WidgetInstance = CreateDefaultSubobject<UWidgetComponent>(TEXT("PingWidget"));
	///WidgetInstanc->RegisterComponent(); // 반드시 등록해야 월드에 나타남
	WidgetInstance->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepRelativeTransform); //FAttachmentTransformRules::KeepRelativeTransform);
	
	// 위젯 클래스 설정
	if (WidgetInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("Create Ping Widget"));
		//WidgetInstance->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		WidgetInstance->SetWidgetClass(PingWidgetClass);
		WidgetInstance->SetDrawSize(FVector2D(300.f, 300.f)); // 위젯 크기 설정
		WidgetInstance->SetWidgetSpace(EWidgetSpace::World); // 월드 공간에 표시
		
		//WidgetInstance->SetRelativeLocation(FVector::UpVector * 100);

		//WidgetInstance->SetVisibility(false); // 초기엔 숨김
	}
	
}
void APingActor::PreviewStart()
{
	if (WidgetInstance)
	{
		//PingMesh->SetVisibility(true);
		WidgetInstance->SetVisibility(true);
		bIsPingActive = true;
	}
}

void APingActor::PreviewEnd()
{
	if (WidgetInstance)
	{
		//PingMesh->SetVisibility(false);
		WidgetInstance->SetVisibility(false);
		bIsPingActive =false;
	}
}



// Called when the game starts or when spawned
void APingActor::BeginPlay()
{
	Super::BeginPlay();

	PingMesh->SetVisibility(false);
	WidgetInstance->SetVisibility(false);
}

void APingActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsPingActive)
	{
		GEngine->AddOnScreenDebugMessage(-1, 0.2f, FColor::White, TEXT("PingWidget SetLocation"));
		WidgetInstance->SetWorldLocation(GetActorLocation());
	}
	
}

void APingActor::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
}
void APingActor::Ping_Implementation()
{
	
}

