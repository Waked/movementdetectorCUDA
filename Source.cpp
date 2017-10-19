#include "opencv2/opencv.hpp"
#include <windows.h> // WinApi header 

#define BUF_SZ 5

using namespace cv;
using namespace std;

extern "C" bool cuImageProcessing(uchar *dataset, uchar *res, int w, int h);

int main(int, char**)
{
	cout << "Initialising camera... ";
	VideoCapture cap(0); // open the default camera
	if (!cap.isOpened()) {  // check if we succeeded
		cout << "Failure! Quitting the program" << endl;
		return -1;
	}
	const int img_w = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	const int img_h = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	cout << "Success. Resolution: " << img_w << "x" << img_h << endl;

	namedWindow("edges", 1);

	// Wstêpne 10 klatek
	Mat *klatka[BUF_SZ];
	for (int i = 0; i < BUF_SZ-1; i++) {
		Mat frame;
		cap >> frame;
		Mat *newframe = new Mat();
		cvtColor(frame, *newframe, CV_BGR2GRAY);
		klatka[i] = newframe;
	}

	uchar* dataset = new uchar[img_w * img_h * BUF_SZ];

	while (true)
	{
		Mat frame;
		Mat *newframe = new Mat();
		cap >> frame; // get a new frame from camera
		cvtColor(frame, *newframe, CV_BGR2GRAY);
		klatka[BUF_SZ-1] = newframe;

		// Aggregate all data
		for (int y = 0; y < img_h; y++) {
			for (int x = 0; x < img_w; x++) {
				for (int n = 0; n < BUF_SZ; n++) {
					dataset[(y * img_w + x) * BUF_SZ + n] = klatka[n]->at<uchar>(y, x);
				}
			}
		}

		uchar *result = new uchar[img_w * img_h];
		cuImageProcessing(dataset, result, img_w, img_h);
		Mat resultMat(img_h, img_w, CV_8UC1, result);
		imshow("edges", resultMat);

		int sum = 0;
		for (int i = 0; i < img_w * img_h; i++) {
			sum += (int)(result[i] == 255 ? 1 : 0);
		}
		if (sum >= 0.5 * img_w * img_h) {
			cout << "Movement!" << endl;
			PlaySoundA("alert.wav", nullptr, SND_FILENAME | SND_ASYNC);
		} else {
			cout << "Standby..." << endl;
		}

		delete klatka[0];
		for (int i = 1; i < BUF_SZ; i++) {
			klatka[i - 1] = klatka[i];
		}
		klatka[BUF_SZ-1] = nullptr;
		delete result;
		if (waitKey(30) != 255) break;
	}

	delete[] dataset;
	// the camera will be deinitialized automatically in VideoCapture destructor
	return 0;
}