#include <iostream>
#include <fstream>
#include <sstream>
#include <cassert>
#include <cstdint>
#include <cstdio>
#include <algorithm>
#include "byteswap.h"
#include "./CNN/cnn.h"

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;
using namespace dnn;

float train(vector<layer_t*>& layers, tensor_t<float>& data, tensor_t<float>& expected){
	for (int i = 0; i < layers.size(); i++){
		if (i == 0)
			activate(layers[i], data);
		else
			activate(layers[i], layers[i - 1]->out);
	}

	tensor_t<float> grads = layers.back()->out - expected;

	for (int i = layers.size() - 1; i >= 0; i--){
		if (i == layers.size() - 1)
			calc_grads(layers[i], grads);
		else
			calc_grads(layers[i], layers[i + 1]->grads_in);
	}

	for (int i = 0; i < layers.size(); i++){
		fix_weights(layers[i]);
	}

	float err = 0;
	for (int i = 0; i < grads.size.x * grads.size.y * grads.size.z; i++){
		float f = expected.data[i];
		if (f > 0.5)
			err += abs(grads.data[i]);
	}
	return err * 100;
}

void forward(vector<layer_t*>& layers, tensor_t<float>& data){
	for (int i = 0; i < layers.size(); i++){
		if (i == 0)
			activate(layers[i], data);
		else
			activate(layers[i], layers[i - 1]->out);
	}
}

struct case_t{
	tensor_t<float> data;
	tensor_t<float> out;
};

uint8_t* read_file(const char* szFile){
	ifstream file(szFile, ios::binary | ios::ate);
	streamsize size = file.tellg();
	file.seekg(0, ios::beg);

	if (size == -1)
		return nullptr;

	uint8_t* buffer = new uint8_t[size];
	file.read((char*)buffer, size);
	return buffer;
}

vector<case_t> read_test_cases(){
	vector<case_t> cases;

	uint8_t* train_image = read_file("train-images.idx3-ubyte");
	uint8_t* train_labels = read_file("train-labels.idx1-ubyte");

	uint32_t case_count = byteswap_uint32(*(uint32_t*)(train_image + 4));

	for (int i = 0; i < case_count; i++){
		case_t c{ tensor_t<float>(28, 28, 1), tensor_t<float>(10, 1, 1) };

		uint8_t* img = train_image + 16 + i * (28 * 28);
		uint8_t* label = train_labels + 8 + i;

		for (int x = 0; x < 28; x++)
			for (int y = 0; y < 28; y++)
				c.data(x, y, 0) = img[x + y * 28] / 255.f;

		for (int b = 0; b < 10; b++)
			c.out(b, 0, 0) = *label == b ? 1.0f : 0.0f;
		cases.push_back(c);
	}
	delete[] train_image;
	delete[] train_labels;

	return cases;
}

void learnCNN() {
	vector<case_t> cases = read_test_cases();
	vector<layer_t*> layers;

	conv_layer_t * layer1 = new conv_layer_t(1, 5, 8, cases[0].data.size);		// 28 * 28 * 1 -> 24 * 24 * 8
	relu_layer_t * layer2 = new relu_layer_t(layer1->out.size);
	pool_layer_t * layer3 = new pool_layer_t(2, 2, layer2->out.size);				// 24 * 24 * 8 -> 12 * 12 * 8
	fc_layer_t * layer4 = new fc_layer_t(layer3->out.size, 10);					// 4 * 4 * 16 -> 10

	layers.push_back((layer_t*)layer1);
	layers.push_back((layer_t*)layer2);
	layers.push_back((layer_t*)layer3);
	layers.push_back((layer_t*)layer4);

	float amse = 0;
	int ic = 0;

	for (long ep = 0; ep < 100000; ){
		for (case_t& t : cases){
			float xerr = train(layers, t.data, t.out);
			amse += xerr;

			ep++;
			ic++;

			if (ep % 1000 == 0)
				cout << "case " << ep << " err=" << amse / ic << endl;
		}
	}

	while (true){
		uint8_t * data = read_file("test.ppm");
		if (data){
			uint8_t * usable = data;

			while (*(uint32_t*)usable != 0x0A353532)
				usable++;

#pragma pack(push, 1)
			struct RGB{
				uint8_t r, g, b;
			};
#pragma pack(pop)

			RGB * rgb = (RGB*)usable;

			tensor_t<float> image(28, 28, 1);
			for (int i = 0; i < 28; i++){
				for (int j = 0; j < 28; j++){
					RGB rgb_ij = rgb[i * 28 + j];
					image(j, i, 0) = (((float)rgb_ij.r
						+ rgb_ij.g
						+ rgb_ij.b)
						/ (3.0f*255.f));
				}
			}

			forward(layers, image);
			tensor_t<float>& out = layers.back()->out;
			for (int i = 0; i < 10; i++){
				printf("[%i] %f\n", i, out(i, 0, 0)*100.0f);
			}

			delete[] data;
		}
	}
}

int main(int argc, char** argv){
	// Load image
	Mat3b img = imread(".\\assets\\CAPTCHA Sample\\21.gif");

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

	learnCNN();

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
