/*
 * DisplayImage.cpp
 *
 *  Created on: Apr 12, 2019
 *      Author: mikez
 */


#include <opencv2/opencv.hpp>

#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"

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
	cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
	cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);

	Poco::Timestamp timeStamp;
	namedWindow( "Display Image", WINDOW_AUTOSIZE );

	cv::Mat frame;
	for(;;)
	{
		//read frame
		cap >> frame;
		imshow( "Display Image", frame );

		//output time information
		timeStamp.update();
		std::string rc = Poco::DateTimeFormatter::format(timeStamp, Poco::DateTimeFormat::ISO8601_FRAC_FORMAT);
		printf("%s\r\n", rc.c_str());

		//save to file
		rc = rc + ".jpg";
		imwrite(rc.c_str(), frame);

		if(waitKey(1) > 0) {
			break;
		}
	}

	cap.release();

	return 0;
}

int main( int argc, char** argv )
{
	//showImage(argc, argv);

	captureImage();

	return 0;
}

