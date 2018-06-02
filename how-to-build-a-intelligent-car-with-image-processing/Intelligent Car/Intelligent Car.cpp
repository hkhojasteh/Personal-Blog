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
	Rect roi(0, 100, img.cols, (img.rows / 1.18) - 100);

	//Crop the full image to that image contained by the rectangle myROI
	Mat3b crop = img(roi);

	// Convert image to gray, blur and sharpen it
	Mat img_gray, img_sharp;
	cvtColor(crop, img_gray, CV_BGR2GRAY);

	// Show result
	imshow("Original", img);
	imshow("Crop", crop);
	imshow("Gray", img_gray);

	waitKey();

	return 0;
}
