// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IconMarkerUMG.h"
#include "UObject/WeakObjectPtrTemplates.h"
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
private:
    TSoftObjectPtr<UIconMarkerUMG> MarkerUMG;
	bool bShowingOnScreen;
	
};
