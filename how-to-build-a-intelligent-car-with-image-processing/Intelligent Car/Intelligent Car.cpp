#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include "byteswap.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
using namespace dnn;

int main(int argc, char** argv){
	// Load image
	Mat3b img = imread("..\\assets\\Road Sample\\Sample1.png");

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

	int histSize = 140;
	Mat histdata, img_sharp_not;
	bitwise_not(img_sharp, img_sharp_not);
	reduce(img_sharp_not, histdata, 0, CV_REDUCE_SUM, CV_32S);

	//Set the ranges ( for B,G,R) )
	//float range[] = { 0, 256 };
	//const float* histRange = { range };
	//bool uniform = true; bool accumulate = false;
	//calcHist(&img_sharp, 1, 0, Mat(), histdata, 1, &histSize, &histRange, uniform, accumulate);

	//Calculate the histograms for input image
	int hist_w = img.cols * 4; int hist_h = img.rows * 4;
	int bin_w = cvRound((double)hist_w / histSize);

	// Show result
	imshow("Original", img);
	imshow("Crop", crop);
	imshow("Gray", img_gray);

	waitKey();

	return 0;
}
