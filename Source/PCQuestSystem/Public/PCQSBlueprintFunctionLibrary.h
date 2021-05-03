// Copyright © Pedro Costa, 2021. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include <Components/IconMarkerComponent.h>
#include "Actors/QuestManager.h"
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
    static void GetActorInformationToPlayerController(APlayerController* PlayerController, AActor* ActorToCheck, bool bUseCameraLocation, FVector ActorToCheckOffSet, FVector2D& OutScreenPosition, float& OutRotationAngleDegrees, float& DistanceToActor, bool& bIsOnScreen, float PercentageEdge = 1.0f);
    UFUNCTION(BlueprintCallable, Category = "PCQS Blueprint Function Library")
    static TArray<UIconMarkerComponent*> GetAllIconComponents();
    UFUNCTION(BlueprintCallable, Category = "PCQS Blueprint Function Library")
    static void GetActorXPositionOnCompass(APlayerController* PlayerController, AActor* ActorToCheck, float margin, float& XPosition);
    UFUNCTION(BlueprintCallable, Category = "PCQS Blueprint Function Library")
    static AQuestManager* GetWorldQuestManager(UObject* WorldContext);

    static void AddIconMarkerComponent(UIconMarkerComponent* Component);
    static void RemoveIconMarkerComponent(UIconMarkerComponent* Component);
private:
    static TArray<UIconMarkerComponent*> AllMarkerComponents;
};
