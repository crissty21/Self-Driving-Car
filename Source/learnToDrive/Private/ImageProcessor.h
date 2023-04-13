#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "PreOpenCVHeaders.h"
#include "OpenCVHelper.h"

#include <ThirdParty/OpenCV/include/opencv2/imgproc.hpp>
#include <ThirdParty/OpenCV/include/opencv2/highgui/highgui.hpp>

#include <ThirdParty/OpenCV/include/opencv2/core.hpp>
#include "PostOpenCVHeaders.h"

#include "ImageProcessor.generated.h"


USTRUCT(BlueprintType)
struct FChanelThreshold
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
		FVector2D Threshold = FVector2D(0);
	UPROPERTY(EditAnywhere)
		bool UseThreshold = false;
	UPROPERTY()
		uint8 LookupTable[256];
};

enum EThresholds
{
	R_RGB_Binary,
	G_RGB_Binary,
	B_RGB_Binary,
	H_HLS_Binary,
	L_HLS_Binary,
	S_HLS_Binary,
	L_LAB_Binary,
	A_LAB_Binary,
	B_LAB_Binary
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UImageProcessor : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UImageProcessor();

protected:
	virtual void BeginPlay() override;

public:

	void SaveData();
	void LoadData();

	UFUNCTION()
		void GetThresholds(int32 index, FVector2D& threshold, bool& useThreshold);
	UFUNCTION()
		void SetThresholds(int32 index, FVector2D threshold, bool useThreshold);

	cv::Mat PrelucrateImage(cv::Mat);
private:
	
	UPROPERTY(EditDefaultsOnly, Category = "Post Procces Binary")
		uint8 ErosionSize = 1;
	UPROPERTY(EditDefaultsOnly, Category = "Post Procces Binary")
		uint8 DilationSize = 3;
	UPROPERTY(EditDefaultsOnly, Category = "Post Procces Binary")
		bool UseErodeDilate = true;
	UPROPERTY(EditDefaultsOnly, Category = "Post Procces Binary")
		uint8 KernelBlurSize = 11;
	UPROPERTY(EditDefaultsOnly, Category = "Post Procces Binary")
		bool UseBlur = false;


	const cv::Mat elementErode = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(2 * ErosionSize + 1, 2 * ErosionSize + 1),
		cv::Point(ErosionSize, ErosionSize));

	const cv::Mat elementDilate = cv::getStructuringElement(cv::MORPH_RECT,
		cv::Size(2 * DilationSize + 1, 2 * DilationSize + 1),
		cv::Point(DilationSize, DilationSize));
	
	const int8 RGBs[6] = { 0,1,2 };
	const int8 HLSs[6] = { 3,4,5 };
	const int8 LABs[6] = { 6,7,8 };
	
	TArray<FChanelThreshold> refToLookUpTables;

	cv::Mat ConvertImage(cv::Mat inputImage, int code);
	cv::Mat BinaryThreshold(cv::Mat input, const int8* threshold);

	bool checkUsageBinary(const int8* table);

	void CreateLUT(uint8* LUT, FVector2D Threshold);
	void LoadSettingsClear();
	void GenerateLookUpTables();


};
