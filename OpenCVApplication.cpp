// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include "opencv2/stitching.hpp" 

std::vector<Mat_<Vec3b>> images;
Mat_<Vec3b> img;

void testOpenImage()
{
	char fname[MAX_PATH];
	while (openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		imshow("image", src);
		waitKey();
	}
}

void testOpenImagesFld()
{
	char folderName[MAX_PATH];
	if (openFolderDlg(folderName) == 0)
		return;
	char fname[MAX_PATH];
	FileGetter fg(folderName, "bmp");
	while (fg.getNextAbsFile(fname))
	{
		Mat src;
		src = imread(fname);
		imshow(fg.getFoundFileName(), src);
		if (waitKey() == 27) //ESC pressed
			break;
	}
}

void testImageOpenAndSave()
{
	Mat src, dst;

	src = imread("Images/Lena_24bits.bmp", CV_LOAD_IMAGE_COLOR);	// Read the image

	if (!src.data)	// Check for invalid input
	{
		printf("Could not open or find the image\n");
		return;
	}

	// Get the image resolution
	Size src_size = Size(src.cols, src.rows);

	// Display window
	const char* WIN_SRC = "Src"; //window for the source image
	namedWindow(WIN_SRC, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(WIN_SRC, 0, 0);

	const char* WIN_DST = "Dst"; //window for the destination (processed) image
	namedWindow(WIN_DST, CV_WINDOW_AUTOSIZE);
	cvMoveWindow(WIN_DST, src_size.width + 10, 0);

	cvtColor(src, dst, CV_BGR2GRAY); //converts the source image to a grayscale one

	imwrite("Images/Lena_24bits_gray.bmp", dst); //writes the destination to file

	imshow(WIN_SRC, src);
	imshow(WIN_DST, dst);

	printf("Press any key to continue ...\n");
	waitKey(0);
}

void testNegativeImage()
{
	char fname[MAX_PATH];
	int MAX_GREY_VALUE = 256;
	while (openFileDlg(fname))
	{
		Mat_<uchar> src = imread(fname, CV_LOAD_IMAGE_GRAYSCALE);
		int height = src.rows;
		int width = src.cols;
		Mat_<uchar> dst = Mat_<uchar>(height, width);
		// Asa se acceseaaza pixelii individuali pt. o imagine cu 8 biti/pixel
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				uchar val = src(i, j);
				uchar neg = MAX_GREY_VALUE - val;
				dst(i, j) = neg;
			}
		}
		imshow("Input image", src);
		imshow("Negative image", dst);
		waitKey();
	}
}

void testRotate() {

	char filename[MAX_PATH];
	openFileDlg(filename);

	Mat src = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	Mat dst;
	cv::rotate(src, dst, 90);

	imshow("Source", src);
	imshow("Destination", dst);
	waitKey(0);
}




// PROIECT DECUPARE IMAGINI

void formate_grid_helper(Mat_<Vec3b> image) {
	images.clear();
	// grid de 3 x 3
	if (image.rows != image.cols) {
		printf("Imaginea nu este patrata\n");
		exit(1);
	}
	int v = (image.rows - 1) / 3; // inaltimea unei subimagini
	int h = (image.cols - 1) / 3; // latimea unei subimagini
	int idx = 0;

	Mat_<Vec3b> mini_images = Mat::zeros(v, h, CV_8UC3);
	mini_images = Mat(image, Rect(0, 0, v, h));

	for (int i = 0; i < image.rows - v; i += v) {
		for (int j = 0; j < image.cols - h; j += h) {
			mini_images = Mat(image, Rect(i, j, h, v));
			images.push_back(mini_images);
			std::string filename = "../Images/GRID_IMAGE" + std::to_string(idx) + ".jpg";
			imwrite(filename, mini_images);
			idx++;
		}
	}
}

void formate_grid() {
	Mat_<Vec3b> source;
	char filename[MAX_PATH];
	while (openFileDlg(filename)) {
		source = imread(filename, CV_LOAD_IMAGE_COLOR);
		formate_grid_helper(source);
		imshow("Imagine sursa", source);
		break;
	}
}

double euclidian_difference(Vec3b p1, Vec3b p2) {
	int r = (p2[2] - p1[2]) * (p1[2] - p1[2]);
	int g = (p2[1] - p1[1]) * (p2[1] - p1[1]);
	int b = (p2[0] - p1[0]) * (p2[0] - p1[0]);
	double dif = sqrt(r + b + g);
	return dif;
}

double euclidianDiffEdge(std::vector<Vec3b> e1, std::vector<Vec3b> e2) {
	//presupunem ca au aceeasi lungime
	double diff = 0.0f;
	for (int i = 0; i < e1.size(); i++) {
		diff += euclidian_difference(e1[i], e2[i]);
	}
	diff /= (double)e1.size();
	return diff;
}

std::vector<double> compareImages(Mat_<Vec3b> src, Mat_<Vec3b> check, int test = 0) {
	if (test == 1) {
		std::vector<double> v;
		v.push_back(euclidianDiffEdge(src.col(src.cols - 1), check.col(0)));
		return v;
	}
	else {
		std::vector<double> v;
		double d = euclidianDiffEdge(src.col(src.cols - 1), check.col(0));  //translatare check in dreapta
		v.push_back(d);
		d = euclidianDiffEdge(src.row(0), check.row(check.rows - 1));			 //translatare check sus
		v.push_back(d);
		d = euclidianDiffEdge(src.col(0), check.col(check.cols - 1));		 //translatare check in stanga
		v.push_back(d);
		d = euclidianDiffEdge(src.row(src.rows - 1), check.row(0));			 //translatare check jos
		v.push_back(d);


		return v;
	}
}

Mat_<Vec3b> minMatchRight(Mat_<Vec3b> src, std::vector<Mat_<Vec3b>> vectImg) {
	double min = MAXINT;
	int minIndex = 10;
	//printf("%d ", vectImg.size());
	for (int i = 0; i < vectImg.size(); i++) {
		std::vector<double> score;
		score = compareImages(src, vectImg.at(i));//
		if (score[0] < min) {   //se verifica minim pt scorul dintre ultima coloana a src si prima coloana a imaginii de test
			min = score[0];
			minIndex = i;
		}
	}
	//vectImg.erase(vectImg.begin() + minIndex);

	return vectImg[minIndex];
}

std::vector<Mat_<Vec3b>> createRow(Mat_<Vec3b> src, std::vector<Mat_<Vec3b>> vectImg) {
	std::vector<Mat_<Vec3b>> row;//
	row.push_back(src);
	Mat_<Vec3b> imgMid = minMatchRight(src, vectImg);
	row.push_back(imgMid);
	Mat_<Vec3b> imgRight = minMatchRight(imgMid, vectImg);
	row.push_back(imgRight);
	return row;

}

Mat_<Vec3b> rotate(Mat_<Vec3b> src, double angle) {
	Mat_<Vec3b> dst;
	Point2f pt(src.cols / 2., src.rows / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(src, dst, r, Size(src.cols, src.rows));
	return dst;
}

void detectieContur() {
	char fname[MAX_PATH];
	openFileDlg(fname);
	cv::Mat image = cv::imread(fname);

	//Prepare the image for findContours
	cv::cvtColor(image, image, CV_BGR2GRAY);
	cv::threshold(image, image, 128, 255, CV_THRESH_BINARY);

	//Find the contours. Use the contourOutput Mat so the original image doesn't get overwritten
	std::vector<std::vector<cv::Point> > contours;
	cv::Mat contourOutput = image.clone();
	cv::findContours(contourOutput, contours, CV_RETR_LIST, CV_CHAIN_APPROX_NONE);

	//Draw the contours
	cv::Mat contourImage(image.size(), CV_8UC3, cv::Scalar(0, 0, 0));
	cv::Scalar colors[3];
	colors[0] = cv::Scalar(255, 0, 0);
	colors[1] = cv::Scalar(0, 255, 0);
	colors[2] = cv::Scalar(0, 0, 255);
	for (size_t idx = 0; idx < contours.size(); idx++) {
		cv::drawContours(contourImage, contours, idx, colors[idx % 3]);
	}

	cv::imshow("Input Image", image);
	cvMoveWindow("Input Image", 0, 0);
	cv::imshow("Contours", contourImage);
	cvMoveWindow("Contours", 200, 0);
	cv::waitKey(0);
}

std::vector<double> checkImages(Mat_<Vec3b> src, Mat_<Vec3b> check, int test = 0) {
	// rotate
	if (test == 1) {
		std::vector<double> v;
		v.push_back(euclidianDiffEdge(src.col(src.cols - 1), check.col(0)));
		return v;
	}
	else {
		std::vector<double> v;
		for (int i = 0; i < 4; i++) {
			double d = euclidianDiffEdge(src.col(src.cols - 1), check.col(0));
			v.push_back(d);
			check = rotate(check, 90);
		}
		check = rotate(check, 90); //revenim la pozitia initiala
		return v;
	}
}

void testCheckImages() {
	char filename[MAX_PATH];
	Mat_<Vec3b> image;
	formate_grid();

	Mat_<Vec3b> src = images.at(0);

	std::random_shuffle(images.begin(), images.end());
	std::vector<double> v = compareImages(src, images.at(1));

	printf("Valori din verificare: ");
	float min = 1505, minIdx = 0;
	for (int i = 0; i < 4; i++) {
		if (v.at(i) < min) {
			minIdx = i;
			min = v.at(i);
		}
		printf("%.3f ", v.at(i));
	}

	Mat_<Vec3b> dest;

	if (minIdx == 0) {
		cv::hconcat(src, images.at(1), dest);
	}
	else if (minIdx == 1) {
		cv::vconcat(images.at(1), src, dest);
	}
	else if (minIdx == 2) {
		cv::hconcat(images.at(1), src, dest);
	}
	else if (minIdx == 3) {
		cv::vconcat(src, images.at(1), dest);
	}

	imshow("Final image", dest);

	imshow("Source", src);
	imshow("Check", images.at(1));
	waitKey(0);

}

void testCreateRow() {
	Mat_<Vec3b> src = imread("Images/GRID_IMAGE0.jpg");
	Mat_<Vec3b> img1 = imread("Images/GRID_IMAGE1.jpg");
	Mat_<Vec3b> img2 = imread("Images/GRID_IMAGE2.jpg");
	Mat_<Vec3b> img3 = imread("Images/GRID_IMAGE3.jpg");
	Mat_<Vec3b> img4 = imread("Images/GRID_IMAGE4.jpg");
	Mat_<Vec3b> img5 = imread("Images/GRID_IMAGE5.jpg");
	Mat_<Vec3b> img6 = imread("Images/GRID_IMAGE6.jpg");
	Mat_<Vec3b> img7 = imread("Images/GRID_IMAGE7.jpg");
	Mat_<Vec3b> img8 = imread("Images/GRID_IMAGE8.jpg");
	std::vector<Mat_<Vec3b>> imgs;
	imgs.push_back(img3);
	imgs.push_back(img8);
	imgs.push_back(img1);
	imgs.push_back(img5);
	imgs.push_back(img6);
	imgs.push_back(img4);
	imgs.push_back(img7);
	imgs.push_back(img2);


	std::random_shuffle(imgs.begin(), imgs.end());
	std::vector<Mat_<Vec3b>> row(3);

	row = createRow(src, imgs);
	for (int i = 0; i < 3; i++) {
		imshow(std::to_string(i), row[i]);
	}

	Mat_<Vec3b> dest(src.rows, (src.cols) * 3);
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			dest(i, j) = row[0](i, j);
		}
	}
	for (int i = 0; i < row[1].rows; i++) {
		for (int j = 0; j < row[1].cols; j++) {
			dest(i, j + src.cols) = row[1](i, j);
		}
	}
	for (int i = 0; i < row[2].rows; i++) {
		for (int j = 0; j < row[2].cols; j++) {
			dest(i, j + row[0].cols + row[1].cols) = row[2](i, j);
		}
	}

	imwrite("result.jpg", dest);
	imshow("Result", dest);
	waitKey(0);
}

int getIndexMin(std::vector<double> v) {
	float min = 1505.23;
	int minIdx = 0;
	for (int i = 0; i < v.size(); i++) {
		if (v.at(i) < min) {
			minIdx = i;
			min = v.at(i);
		}
	}
	return minIdx;
}

double getValMin(std::vector<double> v) {
	double min = 1505.23;
	for (int i = 0; i < v.size(); i++) {
		if (v.at(i) < min) {
			min = v.at(i);
		}
	}
	return min;
}

std::vector<std::pair<int, double>> getMinVectorEdge(Mat_<Vec3b> i1, int orientation = 0) { //daca se pune doar parametriul i1 atunci returneaza un vector cu valorile minime pentru latura din dreapta
	std::vector<std::pair<int, double>> v;

	for (int i = 0; i < images.size(); i++) {
		std::vector<double> aux = compareImages(i1, images.at(i));
		if (getIndexMin(aux) == orientation) {
			v.push_back(std::pair<int, double>(i, getValMin(aux)));
		}
	}
	return v;
}


int main() {
	int op;
	do {
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Open image\n");
		printf(" 2 - Open BMP images from folder\n");
		printf(" 3 - Negative image\n");
		printf(" 4 - Create Row Test\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d", &op);
		switch (op) {
		case 1:
			testOpenImage();
			break;
		case 2:
			testOpenImagesFld();
			break;
		case 3:
			testNegativeImage();
			break;
		case 4:
			testCreateRow();
			break;
		}
	} while (op != 0);
	return 0;
}
