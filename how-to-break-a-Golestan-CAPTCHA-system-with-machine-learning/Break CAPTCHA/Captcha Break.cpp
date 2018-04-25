#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

const char* keys =
"{ help  h     | | Print help message. }"
"{ input i     | | Path to input image or video file. Skip this argument to capture frames from a camera.}"
"{ model m     | | Path to a binary file of model contains trained weights. "
"It could be a file with extensions .caffemodel (Caffe), "
".pb (TensorFlow), .t7 or .net (Torch), .weights (Darknet) }"
"{ config c    | | Path to a text file of model contains network configuration. "
"It could be a file with extensions .prototxt (Caffe), .pbtxt (TensorFlow), .cfg (Darknet) }"
"{ framework f | | Optional name of an origin framework of the model. Detect it automatically if it does not set. }"
"{ classes     | | Optional path to a text file with names of classes. }"
"{ mean        | | Preprocess input image by subtracting mean values. Mean values should be in BGR order and delimited by spaces. }"
"{ scale       | 1 | Preprocess input image by multiplying on a scale factor. }"
"{ width       |   | Preprocess input image by resizing to a specific width. }"
"{ height      |   | Preprocess input image by resizing to a specific height. }"
"{ rgb         |   | Indicate that model works with RGB input images instead BGR ones. }"
"{ backend     | 0 | Choose one of computation backends: "
"0: default C++ backend, "
"1: Halide language (http://halide-lang.org/), "
"2: Intel's Deep Learning Inference Engine (https://software.seek.intel.com/deep-learning-deployment)}"
"{ target      | 0 | Choose one of target computation devices: "
"0: CPU target (by default),"
"1: OpenCL }";

using namespace cv;
using namespace std;
using namespace dnn;

std::vector<std::string> classes;

int main(int argc, char** argv){
	// Load image
	Mat3b img = imread("E:\\MyWorks\\personal WebPage Design\\blog\\assets\\CAPTCHA Sample\\_1.gif");

	// Setup a rectangle to define your region of interest
	Rect roi(0, 0, img.cols, img.rows / 1.11);

	//Crop the full image to that image contained by the rectangle myROI
	Mat3b crop = img(roi);

	// Convert image to gray, blur and sharpen it
	Mat img_gray, img_sharp;
	cvtColor(crop, img_gray, CV_BGR2GRAY);
	blur(img_gray, img_gray, Size(4, 4));

	GaussianBlur(img_gray, img_sharp, cv::Size(0, 0), 6);
	addWeighted(img_gray, 1.80, img_sharp, -0.60, 0, img_sharp);

	//Establish the number of bins
	int histSize = 140;
	//Set the ranges ( for B,G,R) )
	float range[] = { 0, 256 };
	const float* histRange = { range };

	bool uniform = true; bool accumulate = false;

	Mat histdata, img_sharp_not;
	bitwise_not(img_sharp, img_sharp_not);
	reduce(img_sharp_not, histdata, 0, CV_REDUCE_SUM, CV_32S);
	//calcHist(&img_sharp, 1, 0, Mat(), histdata, 1, &histSize, &histRange, uniform, accumulate);

	// Draw the histograms for B, G and R
	int hist_w = 560; int hist_h = 200;
	int bin_w = cvRound((double)hist_w / histSize);

	Mat histImage(hist_h, hist_w, CV_8UC3, Scalar(255, 255, 255));
	/// Normalize the result to [ 0, histImage.rows ]
	normalize(histdata, histdata, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	/// Draw for each channel
	for (int i = 1; i < histSize; i++){
		line(histImage, Point(bin_w*(i - 1), hist_h - cvRound(histdata.at<int>(i - 1))),
			Point(bin_w*(i), hist_h - cvRound(histdata.at<int>(i))),
			Scalar(255, 0, 0), 2, 8, 0);
	}

	// Show result
	imshow("Original", img);
	imshow("Crop", crop);
	imshow("Gray", img_gray);
	imshow("Sharpen", img_sharp);
	imshow("Histogram", histImage);

	waitKey();

	return 0;
}