#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"

#include "PreOpenCVHeaders.h"
#include "OpenCVHelper.h"

#include <ThirdParty/OpenCV/include/opencv2/imgproc.hpp>
#include <ThirdParty/OpenCV/include/opencv2/highgui/highgui.hpp>

#include <ThirdParty/OpenCV/include/opencv2/core.hpp>
#include "PostOpenCVHeaders.h"

#include "Interpreter.generated.h"


UCLASS()
class AInterpreter : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInterpreter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(BlueprintReadOnly)
		UTexture2D* Texture1 = nullptr;

	UPROPERTY(BlueprintReadOnly)
		UTexture2D* Texture2 = nullptr;

	UPROPERTY(BlueprintReadOnly)
		UTexture2D* Texture3 = nullptr;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = Data)
		TArray<FColor> ColorData;

	UFUNCTION(BlueprintCallable, Category = Thresholds)
		void GetThresholds(int32 index, FVector2D& threshold, bool& useThreshold);
	UFUNCTION(BlueprintCallable, Category = Thresholds)
		void SetThresholds(int32 index, FVector2D threshold, bool useThreshold);

	void Save();

	void Load();

protected:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		UTextureRenderTarget2D* TextureRenderRef = nullptr;

	// The rate at which the color data array and video texture is updated (in frames per second)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera, meta = (ClampMin = 0, UIMin = 0))
		float RefreshRate;

	// The refresh timer
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Camera)
		float RefreshTimer;

	UPROPERTY(EditDefaultsOnly, Category = "Create Binary Image")
		bool DrawEdgesInRed = true;

	UPROPERTY(EditDefaultsOnly, Category = "Create Binary Image")
		bool ShowMaxLane = false;

	UPROPERTY(EditDefaultsOnly, Category = "Perspective Wrap")
		uint32 Offset = 50;
	
	UPROPERTY(EditDefaultsOnly, Category = "Perspective Wrap")
		TArray<FVector2D> SourcePoints = { FVector2D(205,55),	//top left
										   FVector2D(0,128),	//bottom left
										   FVector2D(512,128),	//bottom right
										   FVector2D(307,55)	//top right
		};

	UPROPERTY(EditDefaultsOnly, Category = "Perspective Wrap")
		bool DrawPerspectiveLines = true;

	UPROPERTY(EditDefaultsOnly, Category = "Perspective Wrap")
		bool PerspectiveWarpRaw = true;

	UPROPERTY(EditDefaultsOnly, Category = "Perspective Wrap")
		bool PerspectiveWarpBinary = true;

	UPROPERTY(EditDefaultsOnly, Category = "Histogram")
		bool ShowHistogram = true;

	UPROPERTY(EditAnywhere)
		class UImageProcessor* ImageProcesingUnit = nullptr;

private:
	FVector2D VideoSize;

	cv::Point2f srcPts[4];
	cv::Point2f destPts[4];

	void ConstructTextures();
	

	void BindInput();
	
	void ReadFrame();
	
	void CreateTextures(cv::Mat& finalImage, cv::Mat& colorData);
	
	cv::Mat DrawHistogram(cv::Mat& hist);

	void GetHistogramPeaks(cv::Mat& hist, cv::Point2i& leftMax, cv::Point2i& rightMax);
	
	void CreateAndDrawPerspectiveLines(cv::Mat& colorData);
};
