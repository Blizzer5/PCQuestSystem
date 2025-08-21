#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "QuestObject.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UQuestObject : public UInterface
{
	GENERATED_BODY()
};
 
/* Actual Interface declaration. */
class IQuestObject
{
	GENERATED_BODY()
 
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual void ActivateObject() {};
	UFUNCTION(BlueprintImplementableEvent)
	void BP_ActivateObject();
	
	virtual void DeactivateObject(bool bReset) {};
	UFUNCTION(BlueprintImplementableEvent)
	void BP_DeactivateObject(bool bReset);

	virtual void SetTag(FGameplayTag Tag) {};
};
