// Fill out your copyright notice in the Description page of Project Settings.


#include "NNI_CNN.h"
#include "Components/ActorComponent.h"
#include "PreOpenCVHeaders.h"
#include "OpenCVHelper.h"

#include <ThirdParty/OpenCV/include/opencv2/imgproc.hpp>
#include <ThirdParty/OpenCV/include/opencv2/highgui/highgui.hpp>

#include <ThirdParty/OpenCV/include/opencv2/core.hpp>
#include <ThirdParty/OpenCV/include/opencv2/dnn.hpp>
#include "PostOpenCVHeaders.h"

#include "NeuralNetwork.h"


UNNI_CNN::UNNI_CNN()
{
	const FString& ONNXModelFilePath = FPaths::ProjectDir() / TEXT("The_Model3.onnx");
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

float UNNI_CNN::RunModel(TArray<FColor> image, int16 width, int16 height)
{
	if (Network == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("UNeuralNetwork could not loaded"));
		return 0;
	}
	float result = 0.f;

	//create a mat with the data from pixels 
	cv::Mat colorData = cv::Mat(cv::Size(width, height), CV_8UC4, image.GetData());
	// Fill input neural tensor
	InArray = PreProcessImage(colorData);

	Network->SetInputFromArrayCopy(InArray);

	

	// Run UNeuralNetwork
	Network->Run();

		//Read and print OutputTensor
		//const FNeuralTensor& OutputTensor = Network->GetOutputTensor();
		TArray<float> OutputTensor = Network->GetOutputTensor().GetArrayCopy<float>();
	result = OutputTensor[0];
	UE_LOG(LogTemp, Log, TEXT("Neural Network Inference complete. Output: %f"), result)

	return result;
}
TArray<float> UNNI_CNN::PreProcessImage(cv::Mat image)
{
	if (image.empty()) {
		return {};
	}

	// Crop image to remove unnecessary features
	image = image(cv::Range(60, image.rows), cv::Range::all());
	cv::cvtColor(image, image, cv::COLOR_BGRA2RGB);
	cv::GaussianBlur(image, image, cv::Size(3, 3), 0);
	cv::resize(image, image, cv::Size(100, 100));


	//spaghetificare

	TArray<float> ueArray;
	ueArray.SetNumUninitialized(image.total() * image.channels());

	for (int32 row = 0; row < image.rows; ++row) {
		for (int32 col = 0; col < image.cols; ++col) {
			cv::Vec3b pixel = image.at<cv::Vec3b>(row, col);

			for (int32 channel = 0; channel < image.channels(); ++channel) {
				int32 index = (row * image.cols * image.channels()) + (col * image.channels()) + channel;
				ueArray[index] = pixel[channel] / 255.0f;
			}
		}
	}

	return ueArray;
}