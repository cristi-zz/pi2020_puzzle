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
void matchImages(std::vector<Mat_<Vec3b>> cutImages)
{
	for (Mat_<Vec3b> img1 : cutImages)
	{
		for (Mat_<Vec3b> img2 : cutImages)
		{
			bool diff = false;
			for (int i = 0; i < img1.rows; i++) {
				for (int j = 0; j < img1.cols; j++) {
					if (img1(i, j) != img2(i, j))
					{
						diff = true;
					}
				}
			}
			if (diff == true)
			{
				bool noMatchEgde;
				for (int j = 0;j < 3;j++)					//verific primele 3 coloane a imaginii img2
				{
					noMatchEgde = false;
					for (int i = 0;i < img2.rows;i++)
					{
						if (img1(i, img1.cols - 1)[2] != img2(i, j)[2] || img1(i, img1.cols - 1)[1] != img2(i, j)[1] || img1(i, img1.cols - 1)[0] != img2(i, j)[0])
						{
							noMatchEgde = true;
							break;
						}
					}

				}
				if (noMatchEgde == true)
				{
					for (int j = 0;j < 3;j++)				//verific primele 3 linii a imaginii img2
					{
						noMatchEgde = false;
						for (int i = 0;i < img2.cols;i++)
						{
							if (img1(i, img1.cols - 1)[2] != img2(j, img2.cols - 1 - i)[2] || img1(i, img1.cols - 1)[1] != img2(j, img2.cols - 1 - i)[1] || img1(i, img1.cols - 1)[0] != img2(j, img2.cols - 1 - i)[0])
							{
								noMatchEgde = true;
								break;
							}
						}

					}
					if (noMatchEgde == true)			//verific ultimele 3 coloane a imaginii img2
					{
						for (int j = 0;j < 3;j++)
						{
							noMatchEgde = false;
							for (int i = 0;i < img2.cols;i++)
							{
								if (img1(i, img1.cols - 1)[2] != img2(img2.cols - 1 - i, img2.cols - 1 - j)[2] || img1(i, img1.cols - 1)[1] != img2(img2.cols - 1 - i, img2.cols - 1 - j)[1] || img1(i, img1.cols - 1)[0] != img2(img2.cols - 1 - i, img2.cols - 1 - j)[0])
								{
									noMatchEgde = true;
									break;
								}
							}

						}
						if (noMatchEgde == true)		//verific ultimele 3 linii a imaginii img2
						{
							for (int j = 0;j < 3;j++)
							{
								noMatchEgde = false;
								for (int i = 0;i < img2.cols;i++)
								{
									if (img1(i, img1.cols - 1)[2] != img2(img2.rows - 1 - j, img2.cols - 1 - i)[2] || img1(i, img1.cols - 1)[1] != img2(img2.rows - 1 - j, img2.cols - 1 - i)[1] || img1(i, img1.cols - 1)[0] != img2(img2.rows - 1 - j, img2.cols - 1 - i)[0])
									{
										noMatchEgde = true;
										break;
									}
								}

							}
						}

					}
				}




			}

		}
	}

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

Mat rotate(Mat src, double angle)
{
	Mat dst;
	Point2f pt(src.cols / 2., src.rows / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(src, dst, r, Size(src.cols, src.rows));

	return dst;
}

void testRotate(){

	char filename[MAX_PATH];
	openFileDlg(filename);

	Mat src = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
	Mat dst = rotate(src, 90);

	imshow("Source", src);
	imshow("Destination", dst);
	waitKey(0);
}

// Varianta Andrei - ???
double EuclideanDistance(Point p1, Point p2)
{
	double x = p1.x - p2.x; //calculating number to square in next step
	double y = p1.y - p2.y;
	double dist;

	dist = pow(x, 2) + pow(y, 2); //calculating Euclidean distance
	dist = sqrt(dist);

	return dist;
}

void distance(){

	Mat m1 = imread("Images/cameraman.bmp", CV_LOAD_IMAGE_GRAYSCALE);
	Mat m2 = imread("Images/cameraman.bmp", CV_LOAD_IMAGE_GRAYSCALE);

	std::vector<double> distante;

	for (int i = 0; i < m1.rows; i++){
		Point p1 = Point(i, m1.rows - 1);
		Point p2 = Point(i, 0);
		distante.push_back(EuclideanDistance(p1, p2));
	}

}


int getDifference(Vec3b pixelA, Vec3b pixelB) {
	return abs(pixelA[0] - pixelB[0]) + abs(pixelA[1] - pixelB[1]) + abs(pixelA[2] - pixelB[2]);
}

void testDifference() {
	int width = poza.cols();
	int height = poza.rows();
	int *diferentePeColoane = new int[width - 1];

	for (int j = 0; j < width - 1; j++) {
		for (int i = 1; i < height; i++) {
			diferentePeColoane[j] += getDifference(poza.at<Vec3b>(i, j), poza.at<Vec3b>(i, j + 1));
		}
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
		printf(" 5 - Test rotate\n");
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
			case 5:
				testRotate();
				break;
		}
	}
	while (op!=0);
	return 0;
}
