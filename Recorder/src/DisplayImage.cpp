/*
 * DisplayImage.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: mikez
 */


#include <opencv2/opencv.hpp>

using namespace cv;

static int showImage(int argc, char** argv)
{
	Mat image;
	image = imread( argv[1], 1 );
	if( argc != 2 || !image.data )
	{
		printf( "No image data \n" );
		return -1;
	}
	namedWindow( "Display Image", WINDOW_AUTOSIZE );
	imshow( "Display Image", image );
	waitKey(0);

	return 0;
}

static int captureImage()
{
	cv::VideoCapture cap(0);
	if(!cap.isOpened()) {
		printf("Video device cannot be opened\r\n");
		return -1;
	}

	namedWindow( "Display Image", WINDOW_AUTOSIZE );

	cv::Mat frame;
	for(;;)
	{
		cap >> frame;
		imshow( "Display Image", frame );
		if(waitKey(1) > 0) {
			break;
		}
	}

	return 0;
}

int main( int argc, char** argv )
{
	//showImage(argc, argv);

	captureImage();

	return 0;
}

