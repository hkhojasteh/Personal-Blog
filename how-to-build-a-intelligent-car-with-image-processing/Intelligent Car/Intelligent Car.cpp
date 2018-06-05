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

Mat3b img;
Mat img_gray, dst, detected_edges;
int lowThreshold;
int const max_lowThreshold = 100;
int ratio = 3;
int kernel_size = 3;
char* window_name = "Edge Map";
// CannyThreshold: Trackbar callback - Canny thresholds input with a ratio 1:3
void CannyThreshold(int, void*){
	/// Reduce noise with a kernel 3x3
	blur(img_gray, detected_edges, Size(3, 3));

	/// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold*ratio, kernel_size);

	/// Using Canny's output as a mask, we display our result
	dst = Scalar::all(0);

	img.copyTo(dst, detected_edges);
	imshow(window_name, dst);
}

int main(int argc, char** argv){
	// Load image
	img = imread("..\\assets\\Road Sample\\Sample1.png");

	// Setup a rectangle to define your region of interest
	//Rect roi(0, 325, img.cols, img.rows - 455); //5
	Rect roi(0, 400, img.cols, img.rows - 480); //1 , 10

	//Crop the full image to that image contained by the rectangle myROI
	Mat3b img_crop = img(roi);

	// Convert image to gray, blur and sharpen it
	Mat mask_hsv_yellow, mask_white, img_mask;
	cvtColor(img_crop, img_gray, CV_BGR2GRAY);

	// Make target image by apply yellow and white mask
	Scalar m = mean(img_gray);
	cvtColor(img, mask_hsv_yellow, CV_BGR2HSV);
	inRange(img_crop, Scalar(20, 85, 85), Scalar(30, 255, 255), mask_hsv_yellow);
	inRange(img_gray, Scalar(m[0] + (255 - m[0]) / 3.5), Scalar(255), mask_white);
	bitwise_or(mask_white, mask_hsv_yellow, img_mask);
	GaussianBlur(img_mask, img_mask, cv::Size(5, 5), 0);

	dst.create(img_gray.size(), img_gray.type());
	// Create a window
	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
	// Create a Trackbar for user to enter threshold
	createTrackbar("Min Threshold:", window_name, &lowThreshold, max_lowThreshold, CannyThreshold);
	// Show the image
	CannyThreshold(0, 0);

	// Show result
	//imshow("Original", img);
	//imshow("Crop", crop);
	//imshow("Gray", img_gray);
	//imshow("Mask", img_mask);

	waitKey(0);

	return 0;
}
