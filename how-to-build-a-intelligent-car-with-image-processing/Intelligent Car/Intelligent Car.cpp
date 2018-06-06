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

Mat3b img, img_crop;
Mat detected_edges, img_mask;

// Standard Hough Line Transform
void HoughTransform(void*) {
	Mat img_hlines = img_crop.clone();
	// will hold the results of the detection
	vector<Vec4i> lines;
	// runs the actual detection
	HoughLinesP(detected_edges, lines, 1, CV_PI / 180, 50, 30, 10);
	// Draw the lines
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4i l = lines[i];
		line(img_hlines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 255), 2, CV_AA);
	}
	imshow("detected lines", img_hlines);
}

int const max_lowThreshold = 255;
int lowThreshold = 240, ratio = 3, kernel_size = 3;
char* window_name = "Edge Map";
// CannyThreshold: Trackbar callback - Canny thresholds input with a ratio 1:3
void CannyThreshold(int, void*){
	// Reduce noise with a kernel 3x3
	blur(img_mask, detected_edges, Size(3, 3));
	
	// Canny detector
	Canny(detected_edges, detected_edges, lowThreshold, lowThreshold * ratio, kernel_size);

	// Using Canny's output as a mask and display our result
	imshow(window_name, detected_edges);

	HoughTransform(0);
}

int main(int argc, char** argv){
	// Load image
	img = imread("..\\assets\\Road Sample\\Sample10.png");

	// Setup a rectangle to define your region of interest
	//Rect roi(0, 325, img.cols, img.rows - 455); //5
	Rect roi(0, 420, img.cols, img.rows - 500); //1 , 10

	//Crop the full image to that image contained by the rectangle myROI
	img_crop = img(roi);

	// Convert image to gray, blur and sharpen it
	Mat img_gray, mask_hsv_yellow, mask_white;
	cvtColor(img_crop, img_gray, CV_BGR2GRAY);

	// Make target image by apply yellow and white mask
	Scalar m = mean(img_gray);
	cvtColor(img, mask_hsv_yellow, CV_BGR2HSV);
	inRange(img_crop, Scalar(20, 85, 85), Scalar(30, 255, 255), mask_hsv_yellow);
	inRange(img_gray, Scalar(m[0] + (255 - m[0]) / 3.5), Scalar(255), mask_white);
	bitwise_or(mask_white, mask_hsv_yellow, img_mask);
	GaussianBlur(img_mask, img_mask, cv::Size(5, 5), 0);

	detected_edges.create(img_mask.size(), img_mask.type());
	// Create a window
	namedWindow(window_name);
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
