#include <stdio.h>
#include <opencv/cv.h>
#include <opencv2/highgui.hpp>

#include "ovc.hpp"
#include "jpeg.hpp"

int main(int argc, char *argv[])
{
	std::string path = "/home/dozer/Pictures/beatles1.jpg";
	cv::Mat src;
	src = cv::imread(path, CV_LOAD_IMAGE_COLOR);
  
	if(! src.data ) {
    	std::cerr <<  "Could not open image" << std::endl ;
    	return 1;
	}

	cv::Mat cpy(src);

	//ocv::downsample(cpy);
	ocv::vortex_stuff(cpy, 50, 50);
	// ocv::stuff_1(cpy);
	// ocv::stutter(cpy, cv::Rect(340, 250, 1, 20), 200);

	ocv::display(cpy);
	cv::imwrite("/home/dozer/Pictures/output5.jpg", cpy);
	
	return 0;
}