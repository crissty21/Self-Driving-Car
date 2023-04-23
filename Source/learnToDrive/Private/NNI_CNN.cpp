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
		Network->SetDeviceType(ENeuralDeviceType::CPU);
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
	
	// Run UNeuralNetwork
	Network->Run();

	UE_LOG(LogTemp, Log, TEXT("Neural Network Inference complete."))
		//Read and print OutputTensor
		//const FNeuralTensor& OutputTensor = Network->GetOutputTensor();
	TArray<float> OutputTensor = Network->GetOutputTensor().GetArrayCopy<float>();
	result = OutputTensor[0];
	
	return result;
}

TArray<float> UNNI_CNN::PreProcessImage(cv::Mat image)
{
	if (image.empty()) {
		return {};
	}
	
	// Crop image to remove unnecessary features
	//image = image(cv::Rect(0, 60, image.cols, 140));
	cv::cvtColor(image, image, cv::COLOR_BGRA2RGB);
	cv::GaussianBlur(image, image, cv::Size(3, 3), 0);
	cv::resize(image, image, cv::Size(100, 100));

	//spaghetificare
	// reshape to 1D
	image = image.reshape(1, 1);

	
	// uint_8, [0, 255] -> float, [0, 1]
	TArray<float> output;

	output.Reserve(image.rows * image.cols);
	float val;
	float coef = 1. / 255.f;
	for (size_t ch = 0; ch < 3; ++ch) {
		for (int j = ch; j < image.cols; j+=3) {
			val = static_cast<float>(image.at<uint8>(0, j)) * coef;
			output.Add(val);
		}
	}

	//cv::OutputArray vec;
	//image.convertTo(vec, CV_32FC1, 1. / 255);
	/*
	// HWC -> CHW
	TArray<float> output;
	for (size_t ch = 0; ch < 3; ++ch) {
		for (size_t i = ch; i < vec.Num(); i += 3) {
			output.Emplace(vec[i]);
		}
	}
	*/
	return output;
}

