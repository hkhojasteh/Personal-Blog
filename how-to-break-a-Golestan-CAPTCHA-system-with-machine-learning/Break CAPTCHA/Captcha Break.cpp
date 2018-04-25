#include <iostream>
#include <fstream>
#include <sstream>

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
using namespace dnn;

int main(int argc, char** argv){
	// Load image
	Mat3b img = imread("E:\\MyWorks\\personal WebPage Design\\blog\\assets\\CAPTCHA Sample\\21.gif");

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

	int txtMargin = 20;
	Mat histImage(hist_h + txtMargin, hist_w + txtMargin, CV_8UC3, Scalar(255, 255, 255));
	//Normalize the result to [ 0, histImage.rows ]
	normalize(histdata, histdata, 0, histImage.rows, NORM_MINMAX, -1, Mat());

	//Draw histogram
	for (int i = 0; i < hist_h; i++) {
		if (i % 20 == 0) {
			ostringstream txt;
			txt << i;
			putText(histImage, txt.str(), Point(0, hist_h - i),
				1, 0.75, Scalar::all(0), 1, 0);
		}
	}
	for (int i = 1; i < histSize; i++){
		line(histImage, Point(bin_w*(i - 1) + txtMargin, hist_h - cvRound(histdata.at<int>(i - 1))),
			Point(bin_w*(i) + txtMargin, hist_h - cvRound(histdata.at<int>(i))),
			Scalar(255, 100, 0), 2, 8, 0);
		if (i % 10 == 0 || i == 1) {
			ostringstream txt;
			txt << i;
			putText(histImage, txt.str(), Point(bin_w*(i), hist_h + txtMargin),
				1, 0.75, Scalar::all(0), 1, 0);
		}
	}
	
	Mat img_zeroone;
	threshold(img_sharp_not, img_zeroone, 20, 255, THRESH_BINARY);

	//Make good representation for clustering
	Mat points = Mat::zeros(sum(img_zeroone)[0], 2, CV_32F);
	for (int i = 0, k = 0; i < img_zeroone.rows; i++) {
		for (int j = 0; j < img_zeroone.cols; j++) {
			if ((int)img_zeroone.at<char>(i, j) == 255) {
				points.at<float>(k, 0) = i;
				points.at<float>(k, 1) = j;
				k++;
			}
		}
	}
	Mat img_segmentation;
	img_sharp_not.copyTo(img_segmentation);
	//Clustering
	Mat kCenters, kLabels;
	int clusterCount = 5, attempts = 10, iterationNumber = 1e40;
	kmeans(points, clusterCount, kLabels, TermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, iterationNumber, 1e-4), attempts, KMEANS_PP_CENTERS, kCenters);
	for (int i = 0; i < kCenters.rows; i++) {
		float x = kCenters.at<float>(i, 1), y = kCenters.at<float>(i, 0);
		circle(img_segmentation, Point(x, y), 2, (0, 0, 255), -1);
		rectangle(img_segmentation, Rect(x - 13, y - 13, 26, 26), Scalar(255, 255, 255));
	}

	// Show result
	imshow("Original", img);
	imshow("Crop", crop);
	imshow("Gray", img_gray);
	imshow("Sharpen", img_sharp);
	imshow("Sharpen not", img_sharp_not);
	imshow("Segmentation", img_segmentation);
	imshow("Zero One", img_zeroone);
	imshow("Histogram", histImage);

	waitKey();

	return 0;
}