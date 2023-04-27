#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IImageWrapper.h"

#include "Brain.generated.h"


UCLASS()
class ABrain : public AGameModeBase
{
	GENERATED_BODY()
public:
	ABrain();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly)
		int32 VideoWidth = 512;
	UPROPERTY(EditDefaultsOnly)
		int32 VideoHeight = 128;
	UPROPERTY(EditDefaultsOnly)
		bool bSaveTrainingData = false;

	void AddImageToSave(FString path, TArray<FColor> data);
	void AddDataToSave(TArray<FString> Row);
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	class USplineComponent* GetSplineFromRoad();

	void CreateSplineFromLandscape(class USplineComponent* spline);

	EImageFormat ImageFormat;

	FString CsvFilePath;

	TSharedPtr<IImageWrapper> ImageWrapper = nullptr;

	TQueue<TPair<FString, TArray<FColor>>> photos;

	TQueue<TArray<FString>> CSVdata;

	bool WriteRowToCSV(const FString& FilePath, const TArray<FString>& Row);

	bool SaveCameraViewToDisk(const FString& FilePath, TArray<FColor> Bitmap);

	void SaveTrainingData();

};
