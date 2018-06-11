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
Mat detected_edges, img_mask, img_line;

bool less_left(const Vec4i& lhs, const Vec4i& rhs){
	return lhs[0] < rhs[0];
}
bool less_right(const Vec4i& lhs, const Vec4i& rhs) {
	return lhs[0] < rhs[0];
}

// Standard Hough Line Transform
void HoughTransform(void*) {
	Mat img_hlines = img_crop.clone();
	// will hold the results of the detection
	vector<Vec4f> lines, rightls, leftls;
	// runs the actual detection
	HoughLinesP(detected_edges, lines, 1, CV_PI / 180, 50, 30, 10);
	// Draw the lines
	for (size_t i = 0; i < lines.size(); i++) {
		Vec4f l = lines[i];
		line(img_hlines, Point(l[0], l[1]), Point(l[2], l[3]), Scalar(0, 255, 255), 2, CV_AA);
		
		//Calculating the slope and group lines
		float slope = (float)(l[3] - l[1]) / (l[2] - l[0]);
		if (slope > 0.40) {
			//printf("right %f\n", slope);
			rightls.push_back(l);
		}else if (slope < -0.40) {
			//printf("left %f\n", slope);
			leftls.push_back(l);
		}
	}
	// Find regions
	Point left_b, left_t, right_b, right_t;
	if (leftls.size() > 0) {
		auto lmmx = minmax_element(leftls.begin(), leftls.end(), less_left);
		left_b = Point(get<0>(lmmx)[0][0], get<0>(lmmx)[0][1]);
		left_t = Point(get<0>(lmmx)[0][2], get<0>(lmmx)[0][3]);
	}else{
		// Set default values
		left_b = Point(img_hlines.cols / 2, img_hlines.rows - 20);
		left_t = Point(img_hlines.cols / 2, 75);
	}
	if (rightls.size() > 0) {
		auto rmmx = minmax_element(rightls.begin(), rightls.end(), less_right);
		right_t = Point(get<0>(rmmx)[0][0], get<0>(rmmx)[0][1]);
		right_b = Point(get<0>(rmmx)[0][2], get<0>(rmmx)[0][3]);
	}else{
		// Set default values
		right_b = Point(img_hlines.cols / 2, img_hlines.rows - 20);
		right_t = Point(img_hlines.cols / 2, 75);
	}

	/*vector<Point> fitPoints;
	fitPoints.push_back(right_b);
	fitPoints.push_back(right_t);
	Vec4f l(0.0 ,0.0 ,0.0 ,0.0);
	fitLine(fitPoints, l, CV_DIST_L2, 0, 0.01, 0.01);
	line(img_hlines, Point(l[2] - m*l[0], l[2] - m*l[1]), Point(l[2] + m*l[0], l[2] + m*l[1]), (0, 255, 0));*/

	Mat poly = img_hlines.clone();
	vector<Point> vertices{ left_b, left_t, right_t, right_b };
	vector<vector<Point>> pts{ vertices };
	fillPoly(poly, pts, Scalar(58, 190, 37, 0));
	addWeighted(poly, 0.50, img_hlines, 0.50, 0, img_hlines);

	// Find reference line points by average top and bottom middle
	float rx = (((right_b.x - left_b.x) / 2 + left_b.x) + ((right_t.x - left_t.x) / 2 + left_t.x)) / 2;
	float ry = img_hlines.rows;
	// Draw dashed reference line
	Point p1(rx, 0);
	Point p2(rx, ry);
	LineIterator itl1(img_hlines, p1, p2, 8);	// get a line iterator
	LineIterator itl2(img_hlines, Point(p1.x + 1, p1.y), Point(p2.x + 1, p2.y), 8);	// get a line iterator
	for (int i = 0; i < itl1.count; i++, itl1++, itl2++) {
		if (i % 5 != 0) {
			// every 5'th pixel gets dropped, blue stipple line
			(*itl1)[1] = (*itl2)[1] = 80;
			(*itl1)[2] = (*itl2)[2] = 75;
		}
	}
	vector<Point> trivertices{ Point(rx - 7, ry - 1), Point(rx + 7, ry - 1), Point(rx, ry - 8) };
	vector<vector<Point>> tripts{ trivertices };
	fillPoly(img_hlines, tripts, Scalar(103, 80, 75, 0));

	img.copyTo(img_line);
	img_hlines.copyTo(img_line(Rect(0, 325, img_hlines.cols, img_hlines.rows)));

	imshow("detected lines", img_line);
}

int const max_lowThreshold = 255;
int lowThreshold = 210, ratio = 3, kernel_size = 3;
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
	//img = imread("..\\assets\\Road Sample\\Sample10.png");

	//VideoCapture cap(0);     // open the camera

	VideoCapture cap("..\\assets\\2.mp4");
	// Check if camera opened successfully
	if (!cap.isOpened()) {
		cout << "Error opening video stream or file" << endl;
		return -1;
	}

	while (1) {
		// Capture frame-by-frame
		cap >> img;
		// If the frame is empty, break immediately
		if (img.empty()) {
			double count = cap.get(CV_CAP_PROP_FRAME_COUNT); //get the frame count
			cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to last frame
			continue;
		}
		// Display the resulting frame
		//imshow("Frame", img);
		// Press  ESC on keyboard to exit
		char c = (char)waitKey(25);
		if (c == 27)
			break;

		// Setup a rectangle to define your region of interest
		Rect roi(0, 325, img.cols, img.rows - 455); //5
		//Rect roi(0, 420, img.cols, img.rows - 500); //1 , 10
		//Rect roi(0, 0, img.cols, img.rows);

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
		// bitwise_or(img_gray, img_mask, img_mask);
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
	}
	// When everything done, release the video capture object
	cap.release();

	waitKey(0);

	return 0;
}
