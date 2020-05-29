// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include "opencv2/stitching.hpp" 

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


std::vector<Mat_<Vec3b>> images;

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

		printf("Imagine decupata\n");
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

//presupunem ca au aceeasi lungime
double checkEdge(std::vector<Vec3b> e1, std::vector<Vec3b> e2) {
	double diff = 0.0f;
	for (int i = 0; i < e1.size(); i++) {
		diff += euclidian_difference(e1[i], e2[i]);
	}
	diff /= (double)e1.size();
	return diff;
}

std::vector<double> checkImages_translate(Mat_<Vec3b> src, Mat_<Vec3b> check, int test = 0) {
	if (test == 1) {
		std::vector<double> v;
		v.push_back(checkEdge(src.col(src.cols - 1), check.col(0)));
		return v;
	}
	else {
		std::vector<double> v;
		double d = checkEdge(src.col(src.cols - 1), check.col(0));  //translatare check in dreapta
		v.push_back(d);
		d = checkEdge(src.row(0), check.row(check.rows - 1));			 //translatare check sus
		v.push_back(d);
		d = checkEdge(src.col(0), check.col(check.cols - 1));		 //translatare check in stanga
		v.push_back(d);
		d = checkEdge(src.row(src.rows - 1), check.row(0));			 //translatare check jos
		v.push_back(d);


		return v;
	}
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
		printf(" 4 - FINAL \n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d", &op);
		switch (op)
		{
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
			testCheckImages();
			break;
		}
	} while (op != 0);
	return 0;
}
