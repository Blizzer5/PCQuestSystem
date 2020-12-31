// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "IconMarkerComponent.h"
#include "PCQSBlueprintFunctionLibrary.generated.h"

class UIconMarkerComponent;

/**
 * 
 */
UCLASS()
class PCQUESTSYSTEM_API UPCQSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "PCQS Blueprint Function Library")
    static void GetActorInformationToPlayerController(APlayerController* PlayerController, AActor* ActorToCheck, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, float& DistanceToActor, bool& bIsOnScreen, float PercentageEdge = 1.0f);
    UFUNCTION(BlueprintCallable, Category = "PCQS Blueprint Function Library")
    static TArray<UIconMarkerComponent*> GetAllIconComponents();
    UFUNCTION(BlueprintCallable, Category = "PCQS Blueprint Function Library")
    static void GetActorXPositionOnCompass(APlayerController* PlayerController, AActor* ActorToCheck, float margin, float& XPosition);

    static void AddIconMarkerComponent(UIconMarkerComponent* Component);
    static void RemoveIconMarkerComponent(UIconMarkerComponent* Component);
private:
    static TArray<UIconMarkerComponent*> AllMarkerComponents;
};
