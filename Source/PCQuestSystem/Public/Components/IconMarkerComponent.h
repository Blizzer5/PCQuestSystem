// Copyright © Pedro Costa, 2021. All rights reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UObject/WeakObjectPtrTemplates.h"
#include "UI/IconMarkerUMG.h"
#include "IconMarkerComponent.generated.h"

class UIconMarkerUMG;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PCQUESTSYSTEM_API UIconMarkerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UIconMarkerComponent();
	~UIconMarkerComponent();

    // Called when the game starts
    virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "IconMarker")
	void ActivateMarker();
	UFUNCTION(BlueprintCallable, Category = "IconMarker")
	void DeactivateMarker();
    UFUNCTION(BlueprintPure, Category = "IconMarker")
        bool ShouldShowOnScreen();
    UFUNCTION(BlueprintPure, Category = "IconMarker")
        bool ShouldShowOnCompass();
    UFUNCTION(BlueprintPure, Category = "IconMarker")
		UTexture2D* GetMarkerIcon();

	void SetMarkerUMGToUse(TSubclassOf<UIconMarkerUMG> markerToUse);
public:
	/* Icon to use */
	UPROPERTY(EditAnywhere, Category = "IconMarker")
	UTexture2D* MarkerIcon;
	UPROPERTY(EditAnywhere, Category = "IconMarker")
	TSubclassOf<UIconMarkerUMG> MarkerUMGClass;
	UPROPERTY(EditAnywhere, Category = "IconMarker")
    bool bShowOnScreen;
	UPROPERTY(EditAnywhere, Category = "IconMarker")
    bool bShowOnCompass;
    UPROPERTY(EditAnywhere, Category = "IconMarker")
    FVector ActorOffset;
private:
    TSoftObjectPtr<UIconMarkerUMG> MarkerUMG;
	bool bShowingOnScreen;
	
};
