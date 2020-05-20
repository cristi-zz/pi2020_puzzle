// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"



std::vector<Mat_<Vec3b>> images;
void cutToGrid(Mat_<Vec3b> image) {
	images.clear();
	// grid de 3 x 3
	int incrementVertical = (image.rows - 1) / 3; // inaltimea unei subimagini
	int incrementHorizontal = (image.cols - 1) / 3; // latimea unei subimagini
	Mat_<Vec3b> smallImage = Mat::zeros(incrementVertical, incrementHorizontal, CV_8UC3);

	int idx = 0;
	smallImage = Mat(image, Rect(0, 0, incrementHorizontal, incrementVertical));
	for (int i = 0; i < image.rows - incrementVertical; i += incrementVertical) {
		for (int j = 0; j < image.cols - incrementHorizontal; j += incrementHorizontal) {
			smallImage = Mat(image, Rect(i, j, incrementHorizontal, incrementVertical));
			images.push_back(smallImage);
			std::string filename = "../Images/cameraman" + std::to_string(idx) + ".jpg";
			imwrite(filename, smallImage);
			idx++;
		}
	}
}

void stitch() {
	Mat_<Vec3b> image;
	char filename[MAX_PATH];
	while (openFileDlg(filename)) {
		image = imread(filename, CV_LOAD_IMAGE_COLOR);
		cutToGrid(image);
	}
}

/*void matchImages(std::vector<Mat_<Vec3b>> cutImages) {
	for (Mat_<Vec3b> img1 : cutImages) {
		for (Mat_<Vec3b> img2 : cutImages) {
			bool diff = false;
			for (int i = 0; i < img1.rows; i++) {
				for (int j = 0; j < img1.cols; j++) {
					if (img1(i, j) != img2(i, j)) {
						diff = true;
					}
				}
			}
			if (diff == true) {
				bool noMatchEgde;
				for (int j = 0; j < 3; j++) {		    //verific primele 3 coloane a imaginii img2
					noMatchEgde = false;
					for (int i = 0; i < img2.rows; i++) {
						if (img1(i, img1.cols - 1)[2] != img2(i, j)[2] || img1(i, img1.cols - 1)[1] != img2(i, j)[1] || img1(i, img1.cols - 1)[0] != img2(i, j)[0]) {
							noMatchEgde = true;
							break;
						}
					}
				}
				if (noMatchEgde == true) {
					for (int j = 0; j < 3; j++) {		//verific primele 3 linii a imaginii img2
						noMatchEgde = false;
						for (int i = 0; i < img2.cols; i++) {
							if (img1(i, img1.cols - 1)[2] != img2(j, img2.cols - 1 - i)[2] || img1(i, img1.cols - 1)[1] != img2(j, img2.cols - 1 - i)[1] || img1(i, img1.cols - 1)[0] != img2(j, img2.cols - 1 - i)[0]) {
								noMatchEgde = true;
								break;
							}
						}
					}
					if (noMatchEgde == true) {		//verific ultimele 3 coloane a imaginii img2
						for (int j = 0; j < 3; j++) {
							noMatchEgde = false;
							for (int i = 0; i < img2.cols; i++) {
								if (img1(i, img1.cols - 1)[2] != img2(img2.cols - 1 - i, img2.cols - 1 - j)[2] || img1(i, img1.cols - 1)[1] != img2(img2.cols - 1 - i, img2.cols - 1 - j)[1] || img1(i, img1.cols - 1)[0] != img2(img2.cols - 1 - i, img2.cols - 1 - j)[0]) {
									noMatchEgde = true;
									break;
								}
							}
						}
						if (noMatchEgde == true) {	//verific ultimele 3 linii a imaginii img2
							for (int j = 0; j < 3; j++) {
								noMatchEgde = false;
								for (int i = 0; i < img2.cols; i++) {
									if (img1(i, img1.cols - 1)[2] != img2(img2.rows - 1 - j, img2.cols - 1 - i)[2] || img1(i, img1.cols - 1)[1] != img2(img2.rows - 1 - j, img2.cols - 1 - i)[1] || img1(i, img1.cols - 1)[0] != img2(img2.rows - 1 - j, img2.cols - 1 - i)[0]) {
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
}*/

Mat_<Vec3b> rotate(Mat_<Vec3b> src, double angle) {
	Mat_<Vec3b> dst;
	Point2f pt(src.cols / 2., src.rows / 2.);
	Mat r = getRotationMatrix2D(pt, angle, 1.0);
	warpAffine(src, dst, r, Size(src.cols, src.rows));
	return dst;
}

double getDifference(Vec3b p1, Vec3b p2) {
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
		diff += getDifference(e1[i], e2[i]);
	}
	diff /= (double)e1.size();
	return diff;
}

std::vector<double> checkImages(Mat_<Vec3b> src, Mat_<Vec3b> check, int test = 0) {
	if (test == 1) {
		std::vector<double> v;
		v.push_back(checkEdge(src.col(src.cols-1), check.col(0)));
		return v;
	}
	else {
		std::vector<double> v;
		for (int i = 0; i < 4; i++) {
			double d = checkEdge(src.col(src.cols - 1), check.col(0));
			v.push_back(d);
			check = rotate(check, 90);
		}
		check = rotate(check, 90); //revenim la pozitia initiala
		return v;
	}
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
		d= checkEdge(src.row(0), check.row(check.rows-1));			 //translatare check sus
		v.push_back(d);
		d = checkEdge(src.col(0), check.col(check.cols - 1));		 //translatare check in stanga
		v.push_back(d);
		d = checkEdge(src.row(src.rows-1), check.row(0));			 //translatare check jos
		v.push_back(d);
		
		
		return v;
	}
}
Mat_<Vec3b> minMatchRight(Mat_<Vec3b> src, std::vector<Mat_<Vec3b>> vectImg)
{
	
	double min = MAXINT;
	int minIndex = 10;
	for (int i = 0;i < vectImg.size();i++)
	{
		std::vector<double> score;
		score=checkImages_translate(src, vectImg[i], 0);
		if(score[0]<min)   //se verifica minim pt scorul dintre ultima coloana a src si prima coloana a imaginii de test
		{
			min = score[0];
			minIndex = i;
		}
	}
	vectImg.erase(vectImg.begin() + minIndex);

	return vectImg[minIndex];
}
std::vector<Mat_<Vec3b>> createRow(Mat_<Vec3b> src,std::vector<Mat_<Vec3b>> vectImg)
{
	std::vector<Mat_<Vec3b>> row(3);
	row.push_back(src);
	Mat_<Vec3b> imgMid = minMatchRight(src, vectImg);
	row.push_back(imgMid);
	Mat_<Vec3b> imgRight = minMatchRight(imgMid, vectImg);
	row.push_back(imgRight);
	return row;

}
std::vector<Vec3b> extractRow(Mat_<Vec3b> src, int r) {
	std::vector<Vec3b> row = src.row(r);
	return row;
}

std::vector<Vec3b> extractColumn(Mat_<Vec3b> src, int c) {
	std::vector<Vec3b> col = src.col(c);
	return col;
}



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
	Mat dst = rotate(src, 90);

	imshow("Source", src);
	imshow("Destination", dst);
	waitKey(0);
}

// Varianta Andrei - ???
// double EuclideanDistance(Point p1, Point p2)
// {
// 	double x = p1.x - p2.x; //calculating number to square in next step
// 	double y = p1.y - p2.y;
// 	double dist;
// 	dist = pow(x, 2) + pow(y, 2); //calculating Euclidean distance
// 	dist = sqrt(dist);
// 	return dist;
// }
//
// void distance() {
// 	Mat m1 = imread("Images/cameraman.bmp", CV_LOAD_IMAGE_GRAYSCALE);
// 	Mat m2 = imread("Images/cameraman.bmp", CV_LOAD_IMAGE_GRAYSCALE);
// 	std::vector<double> distante;
// 	for (int i = 0; i < m1.rows; i++) {
// 		Point p1 = Point(i, m1.rows - 1);
// 		Point p2 = Point(i, 0);
// 		distante.push_back(EuclideanDistance(p1, p2));
// 	}
// }
//
//void testDifference() {
//	//int width = poza.cols();
//	//int height = poza.rows();
//	int *diferentePeColoane = new int[width - 1];
//
//	for (int j = 0; j < width - 1; j++) {
//		for (int i = 1; i < height; i++) {
//			diferentePeColoane[j] += getDifference(poza.at<Vec3b>(i, j), poza.at<Vec3b>(i, j + 1));
//		}
//	}
//}


int main() {
	int op;
	do {
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
			stitch();
			break;
		case 5:
			testRotate();
			break;
		}
	} while (op != 0);
	return 0;
}
