#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdint>
#include <cstdio>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv){
	// Load image
	Mat3b img = imread("..\\assets\\Road Sample\\Sample1.png");

	// Setup a rectangle to define your region of interest
	//Rect roi(0, 325, img.cols, img.rows - 455); //5
	Rect roi(0, 400, img.cols, img.rows - 480); //1 , 10

	//Crop the full image to that image contained by the rectangle myROI
	Mat3b img_crop = img(roi);

	// Convert image to gray, blur and sharpen it
	Mat img_gray, mask_hsv_yellow, mask_white, img_mask;
	cvtColor(img_crop, img_gray, CV_BGR2GRAY);

	// Make target image by apply yellow and white mask
	Scalar m = mean(img_gray);
	cvtColor(img, mask_hsv_yellow, CV_BGR2HSV);
	inRange(img_crop, Scalar(20, 85, 85), Scalar(30, 255, 255), mask_hsv_yellow);
	inRange(img_gray, Scalar(m[0] + (255 - m[0]) / 3.5), Scalar(255), mask_white);
	bitwise_or(mask_white, mask_hsv_yellow, img_mask);
	GaussianBlur(img_mask, img_mask, cv::Size(5, 5), 0);

	// Show result
	imshow("Original", img);
	//imshow("Crop", crop);
	imshow("Gray", img_gray);
	imshow("Mask", img_mask);

	waitKey();

	return 0;
}
