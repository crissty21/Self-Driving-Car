#include "NNI_CNN.h"


UNNI_CNN::UNNI_CNN()
{
	PrimaryComponentTick.bCanEverTick = false;

	const FString& ONNXModelFilePath = TEXT("C:/Users/crist/Desktop/The_Model.onnx");
	// Create Network object if null
	if (Network == nullptr) {
		Network = NewObject<UNeuralNetwork>((UObject*)GetTransientPackage(), UNeuralNetwork::StaticClass());

		// Load model from file.
		// Set Device
		Network->SetDeviceType(ENeuralDeviceType::GPU);
		// Check that the network was successfully loaded
		if (Network->Load(ONNXModelFilePath))
		{
			UE_LOG(LogTemp, Warning, TEXT("Neural Network loaded successfully."))
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("UNeuralNetwork could not loaded from %s."), *ONNXModelFilePath);
			Network = nullptr;
		}
	}

}

void UNNI_CNN::BeginPlay()
{
	Super::BeginPlay();
	
}

float UNNI_CNN::RunModel(cv::Mat image)
{
	if (Network == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UNeuralNetwork could not loaded"));
		return 0;
	}
	float result = 0.f;

	// Fill input neural tensor
	const TArray<float> InArray = PreProcessImage(image); 
	Network->SetInputFromArrayCopy(InArray); 

	UE_LOG(LogTemp, Display, TEXT("Input tensor: %s."), *Network->GetInputTensor().ToString());


	// Run UNeuralNetwork
	Network->Run();

	UE_LOG(LogTemp, Log, TEXT("Neural Network Inference complete."))
		//Read and print OutputTensor
		//const FNeuralTensor& OutputTensor = Network->GetOutputTensor();
	TArray<float> OutputTensor = Network->GetOutputTensor().GetArrayCopy<float>();
	result = OutputTensor[0];
	UE_LOG(LogTemp, Warning, TEXT("Output is: %f"), result);

	
	return result;
}

TArray<float> UNNI_CNN::PreProcessImage(cv::Mat image)
{
	if (image.empty()) {
		return {};
	}
	
	// Crop image to remove unnecessary features
	cv::Mat CroppedImg = image(cv::Rect(0, 60, image.cols, 140));

	// Convert image to YUV color space
	cv::Mat YuvImg;
	cv::cvtColor(image, YuvImg, cv::COLOR_RGB2YUV);

	// Apply Gaussian blur
	cv::Mat BlurredImg;
	cv::GaussianBlur(YuvImg, BlurredImg, cv::Size(3, 3), 0);

	// Resize image for easier processing
	cv::Mat ResizedImg;
	cv::resize(BlurredImg, ResizedImg, cv::Size(100, 100));

	// reshape to 1D
	ResizedImg = ResizedImg.reshape(1, 1);
	
	// uint_8, [0, 255] -> float, [0, 1]
	float x = 1. / 255.f;
	TArray<float> vec;
	vec.Reserve(ResizedImg.rows * ResizedImg.cols);
	for (int i = 0; i < ResizedImg.rows; i++) {
		for (int j = 0; j < ResizedImg.cols; j++) {
			vec.Add(static_cast<float>(ResizedImg.at<uchar>(i, j)) * x);
		}
	}

	//cv::OutputArray vec;
	//image.convertTo(vec, CV_32FC1, 1. / 255);

	// HWC -> CHW
	TArray<float> output;
	for (size_t ch = 0; ch < 3; ++ch) {
		for (size_t i = ch; i < vec.Num(); i += 3) {
			output.Emplace(vec[i]);
		}
	}
	return output;
}

