#include "NNI_CNN.h"


UNNI_CNN::UNNI_CNN()
{
	PrimaryComponentTick.bCanEverTick = false;

	const FString& ONNXModelFilePath = TEXT("C:/Users/crist/Desktop/The_Model1.onnx");
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
	cv::Mat float_image;
	image.convertTo(float_image, CV_32FC3);

	cv::Mat transposed_image = float_image.reshape(1,1);
	//cv::transpose(float_image.reshape(1, 1), transposed_image);

	TArray<float> ImageData;
	ImageData.SetNumUninitialized(transposed_image.total());
	UE_LOG(LogTemp, Warning, TEXT("%i"), ImageData.Num());

	for (size_t i = 0; i < ImageData.Num(); ++i) {
		ImageData[i] = transposed_image.at<float>(0, i) / 255.0f;
		UE_LOG(LogTemp, Warning, TEXT("%f"), ImageData[i]);
	}
	return ImageData;
}

