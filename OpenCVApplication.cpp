// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"


void cutImage(Mat_<Vec3b> image) {
	std::vector<Mat_<Vec3b>> cutImages;
	std::vector<Point> positions;
	// grid de 3 x 3
	int incrementVertical = (image.rows-1) / 3; // inaltimea unei subimagini
	int incrementHorizontal = (image.cols-1) / 3; // latimea unei subimagini
	Mat_<Vec3b> smallImage = Mat(incrementVertical, incrementHorizontal, CV_8UC3);

	smallImage = Mat(image, Rect(0, 0, incrementHorizontal, incrementVertical));
	imshow("dadaa", smallImage);
	for (int i = 0; i < image.rows - incrementVertical; i += incrementVertical) {
		for (int j = 0; j < image.cols - incrementHorizontal; j += incrementHorizontal) {
			smallImage = Mat(image, Rect(i, j, incrementHorizontal, incrementVertical));
			positions.push_back(Point(i+550, j+320));
			cutImages.push_back(smallImage);
		}
	}

	imshow("Original Image", image);
	moveWindow("Original Image", 200, 200);
	for (int i = 0; i < cutImages.size(); i++) {
		std::string imgName = "Cut image no " + std::to_string(i);
		imshow(imgName, cutImages.at(i));
		moveWindow(imgName, positions.at(i).x, positions.at(i).y);
	}
	waitKey(0);
}

void stitch() {
	Mat_<Vec3b> image;
	char filename[MAX_PATH];
	while (openFileDlg(filename)) {
		image = imread(filename, CV_LOAD_IMAGE_COLOR);

		cutImage(image);
	}
}

void testOpenImage()
{
	char fname[MAX_PATH];
	while(openFileDlg(fname))
	{
		Mat src;
		src = imread(fname);
		imshow("image",src);
		waitKey();
	}
}

void testOpenImagesFld()
{
	char folderName[MAX_PATH];
	if (openFolderDlg(folderName)==0)
		return;
	char fname[MAX_PATH];
	FileGetter fg(folderName,"bmp");
	while(fg.getNextAbsFile(fname))
	{
		Mat src;
		src = imread(fname);
		imshow(fg.getFoundFileName(),src);
		if (waitKey()==27) //ESC pressed
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
	while(openFileDlg(fname))
	{
		Mat_<uchar> src = imread(fname,CV_LOAD_IMAGE_GRAYSCALE);
		int height = src.rows;
		int width = src.cols;
		Mat_<uchar> dst = Mat_<uchar>(height,width);
		// Asa se acceseaaza pixelii individuali pt. o imagine cu 8 biti/pixel
		for (int i=0; i<height; i++)
		{
			for (int j=0; j<width; j++)
			{
				uchar val = src(i,j);
				uchar neg = MAX_GREY_VALUE - val;
				dst(i,j) = neg;
			}
		}
		imshow("Input image",src);
		imshow("Negative image",dst);
		waitKey();
	}
}

int main()
{
	int op;
	do
	{
		system("cls");
		destroyAllWindows();
		printf("Menu:\n");
		printf(" 1 - Open image\n");
		printf(" 2 - Open BMP images from folder\n");
		printf(" 3 - Negative image\n");
		printf(" 4 - Cut open image\n");
		printf(" 0 - Exit\n\n");
		printf("Option: ");
		scanf("%d",&op);
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
				stitch();
				break;
		}
	}
	while (op!=0);
	return 0;
}
