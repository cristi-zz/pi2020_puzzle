// OpenCVApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "common.h"
#include <cmath>  
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
			imshow(filename, mini_images);	
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


// final

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

void printVector(std::vector<double> v) {
	for (int i = 0; i < v.size(); i++) {
		printf("%.3f ", v.at(i));
	}
}

// 0 = dreapta
// 1 = sus
// 2 = stanga
// 3 = jos
std::vector<std::pair<int, double>> getMinVectorEdge(Mat_<Vec3b> i1, int orientation = 0) { //daca se pune doar parametriul i1 atunci returneaza un vector cu valorile minime pentru latura din dreapta
	std::vector<std::pair<int, double>> v;

	for (int i = 0; i < images.size(); i++) {
		std::vector<double> aux = compareImages(i1, images.at(i));
		printf("\nImagine %d: ", i);
		printVector(aux);
		if (getIndexMin(aux) == orientation) {
			v.push_back(std::pair<int, double>(i, getValMin(aux)));
		}
	}
	return v;
}

std::pair<int, double> getMinPair(std::vector<std::pair<int, double>> v) {
	double min = 1505.23;
	std::pair<int, double> r;
	for (int i = 0; i < v.size(); i++) {
		if (v.at(i).second < min) {
			min = v.at(i).second;
			r = v.at(i);
		}
	}
	return r;
}

Mat_<Vec3b> createRowFromFirst(Mat_<Vec3b> sursa) { //creeaza un rand cu prima imagine 'sursa'
	Mat_<Vec3b> dest, aux;
	sursa.copyTo(aux);
	sursa.copyTo(dest);

	int img = 0;

	printf("\n\nCaut imagine in dreapta:  ");
	while (img != 2) {
		printf("\n\nColoana %d ", img + 1);
		std::pair<int, double> r = getMinPair(getMinVectorEdge(aux, 0));
		printf("\tValoare aleasa: (imagine %d) (distanta %.3f)", r.first, r.second);
		cv::hconcat(dest, images.at(r.first), dest);
		images.at(r.first).copyTo(aux);
		img++;
	}

	return dest;
}

Mat_<Vec3b> firstFromRow(Mat_<Vec3b> sursa) { //caut imaginea care se potriveste jos
	Mat_<Vec3b> first;
	int n = images.size();
	printf("\n\nCaut prima imagine dintr-un rand: ");
	std::pair<int, double> r = getMinPair(getMinVectorEdge(sursa, 3));
	printf("\tValoare aleasa: (imagine %d) (distanta %.3f)", r.first, r.second);
	first = images.at(r.first);
	return first;
}

void finalForm() {
	formate_grid();

	Mat_<Vec3b> first = images.at(0);
	images.erase(images.begin());

	Mat_<Vec3b> dst = createRowFromFirst(first);

	Mat_<Vec3b> second = firstFromRow(first);
	Mat_<Vec3b> dst1 = createRowFromFirst(second);

	Mat_<Vec3b> third = firstFromRow(second);
	Mat_<Vec3b> dst2 = createRowFromFirst(third);

	imshow("Final image1", dst);
	imshow("Final image2", dst1);
	imshow("Final image3", dst2);

	Mat_<Vec3b> finalDest;
	cv::vconcat(dst, dst1, finalDest);
	cv::vconcat(finalDest, dst2, finalDest);

	imshow("DESTINATION", finalDest);
	waitKey(0);
}



// puzzle
//int iii = 0;
std::vector<int> generateChainCode(Mat_<Vec3b> image, int vec = 8) {
	int rows[] = { 0, -1, -1, -1,  0,  1, 1, 1 };
	int cols[] = { 1,  1,  0, -1, -1, -1, 0, 1 };
	std::vector<int> v;

	int height = image.rows;
	int width = image.cols;

	Mat_<Vec3b> imgDest = Mat::zeros(height, width, CV_8UC3);
	Mat_<uchar> binary = Mat::zeros(height, width, CV_8UC1);
	boolean first = true;
	Point p0 = Point(0, 0);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			imgDest(i, j) = Vec3b(255, 255, 255);
			if (image(i, j) == Vec3b(0, 0, 0)) {
				binary(i, j) = 1;
				if (first) {
					p0 = Point(i, j); // Alegem pixelul de start P0
					first = false;
				}
			}
		}
	}

	int dir = 7;
	int index = 0;
	Point current = p0;
	Point p1 = (0, 0);
	first = true;

	while (true) {
		if (current == p0 && !first)
			break;
		first = false;
		imgDest(current.x, current.y) = Vec3b(0, 0, 0);
		int posIndex = dir % 2 == 0 ? (dir + 7) % 8 : (dir + 6) % 8;
		while (binary(current.x, current.y) != binary(current.x + rows[posIndex], current.y + cols[posIndex])) {
			if (posIndex == 7)
				posIndex = 0;
			else
				posIndex++;
		}
		v.push_back(posIndex);
		current.x += rows[posIndex];
		current.y += cols[posIndex];
		if (v.size() == 1)
			p1 = current;
		else if (p1 == current)
			break;
		//actualizam DIR
		dir = posIndex;
	}

	//imshow(std::string("conturrrrr") + std::to_string(iii++), imgDest);
	return v;
}

void construireContur(std::vector<int> dirs, int x, int y) {
	int rows[] = { 0, -1, -1, -1,  0,  1, 1, 1 };
	int cols[] = { 1,  1,  0, -1, -1, -1, 0, 1 };

	img(x, y) = Vec3b(0, 0, 255);
	int index = 0, dir;
	while (index < dirs.size()) {
		dir = dirs.at(index);

		x += rows[dir];
		y += cols[dir];
		img(x, y) = Vec3b(0, 0, 255);
		index++;
	}
}

int checkValues(std::vector<int> v, int i, int nr) {
	int aux = 0;
	for (int j = i; j < v.size(); j++) {
		if (v.at(j) == nr) {
			aux++;
		}
		else {
			if (aux >= 40)
				return j;
			else if (j > i + 20)
				return -1;
		}
	}
	return -1;
}

std::vector<int> getCodeLeftEdge(std::vector<int> v1) {
	int indexS1 = 0, indexF1 = 0;
	std::vector<int> edge1;

	for (int i = 0; i < v1.size(); i++) {
		int j = checkValues(v1, i, 0);
		if (j > 0) {
			int jj = checkValues(v1, j, 2);
			if (jj > 0) {
				indexS1 = j - 15;
			}
		}

		int jj = checkValues(v1, i, 2);
		if (jj > 0) {
			int aux = checkValues(v1, jj, 4);
			if (aux > 0) {
				indexF1 = jj + 15;
				break;
			}
		}
	}
	std::copy(v1.begin() + indexS1, v1.begin() + indexF1, back_inserter(edge1));
	construireContur(edge1, 180, 50);

	return edge1;
}

std::vector<int> getCodeRightEdge(std::vector<int> v2) {
	int indexS2 = 0, indexF2 = 0;
	std::vector<int> edge2;
	for (int i = 0; i < v2.size(); i++) {
		int j = checkValues(v2, i, 4);
		if (j > 0) {
			int jj = checkValues(v2, j, 6);
			if (jj > 0) {
				indexS2 = j - 15;
			}
		}

		int jj = checkValues(v2, i, 6);
		if (jj > 0) {
			int aux = checkValues(v2, jj, 0);
			if (aux > 0) {
				indexF2 = jj + 15;
			}
		}
	}

	std::copy(v2.begin() + indexS2, v2.begin() + indexF2, back_inserter(edge2));
	construireContur(edge2, 30, 150);

	return edge2;
}

bool cross(int x, int y) {
	if (x == abs(y - 4) || y == abs(x - 4))
		return true;
	return false;
}

void checkPuzzleImages(std::vector<int> p1, std::vector<int> p2) {
	std::reverse(p1.begin(), p1.end());
	int n = min(p1.size(), p2.size());
	int err = 0;
	int errM = 0;
	for (int x = 0; x < n; x++) {
		if (!cross(p1.at(x), p2.at(x))) {
			err++;
			
			int a = p1.at(x);
			int b = p2.at(x);

			int dif = std::abs(a - b);
			if (dif == 0) errM += 4;
			else if (dif == 1 || dif == 7) errM += 3;
			else if (dif == 2 || dif == 6) errM += 2;
			else if (dif == 3 || dif == 5) errM += 1;

		}
	}

	float finalErr = (float)errM / err;

	printf("\nEroarea din codul generat: %f\n", finalErr);
	finalErr *= 45;
	printf("Eroarea din codul generat (in grade): %f\n", finalErr);

}

void checkPuzzleImagesOffset(std::vector<int> p1, std::vector<int> p2) {
	std::reverse(p1.begin(), p1.end());
	int n = min(p1.size(), p2.size());
	int size = p1.size() - p2.size();
	int offsetMaxim = p1.size() / 2;
	std::vector<float> errs;
	for (int of = 0; of <= offsetMaxim; of++) {
		int err = 0;
		int errM = 0;
		for (int x = 0; x < n - of; x++) {
			if (!cross(p1.at(x + of), p2.at(x))) {
				err++;

				int a = p1.at(x);
				int b = p2.at(x);
				int dif = std::abs(a - b);

				if (dif == 0) errM += 4;
				else if (dif == 1 || dif == 7) errM += 3;
				else if (dif == 2 || dif == 6) errM += 2;
				else if (dif == 3 || dif == 5) errM += 1;
			}
		}
		float finalErr = (float)errM / err;
		errs.push_back(finalErr);
	}

	std::pair<int, float> min = std::pair<int, float>(0, 999990.0);
	for (int i = 0; i < errs.size(); i++) {
		printf("%.3f ", errs.at(i));
		if (errs.at(i) < min.second) {
			min.first = i;
			min.second = errs.at(i);
		}
	}

	printf("\nOffsetul pentru eroare minima este: %d %f", min.first, min.second);

	std::ofstream file;
	file.open("errors.csv", std::ofstream::out | std::ofstream::trunc);
	for (int i = 0; i < errs.size(); i++) {
		file << errs.at(i) << ", \n";
	}


	/*printf("Erorea din codul generat: %f\n", finalErr);
	finalErr *= 45;
	printf("Erorea din codul generat (in grade): %f\n", finalErr);*/

}


void puzzleImage() {
	Mat_<Vec3b> p1, p2;
	p1 = imread("Images/piece1.jpg", CV_LOAD_IMAGE_COLOR);
	p2 = imread("Images/piece2.jpg", CV_LOAD_IMAGE_COLOR);
	img = Mat(220, 220, CV_8UC3);

	for (int i = 0; i < img.rows; i++) {
		for (int j = 0; j < img.cols; j++) {
			img(i, j) = Vec3b(255, 255, 255);
		}
	}
	std::vector<int> c1 = generateChainCode(p1);
	std::vector<int> c2 = generateChainCode(p2);

	imshow("Piece1", p1);
	imshow("Piece2", p2);
	std::vector<int> left = getCodeLeftEdge(c1);
	std::vector<int> right = getCodeRightEdge(c2);

	checkPuzzleImagesOffset(left, right);

	/*printf("%d %d\n", left.size(), right.size());

	for (int i = 0; i < left.size(); i++) {
		printf("%d ", left.at(i));
	}
	printf("\n\n");
	for (int i = 0; i < right.size(); i++) {
		printf("%d ", right.at(i));
	}*/


	checkPuzzleImages(left, right);
	
	imshow("Contur margini", img);

	waitKey(0);
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
		printf(" 5 - Forma finala\n");
		printf(" 6 - Piese puzzle\n");
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
		case 5:
			finalForm();
			break;
		case 6:
			puzzleImage();
			break;
		}
	} while (op != 0);
	return 0;
}
