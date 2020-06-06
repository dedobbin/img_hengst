#include <stdio.h>
#include <opencv/cv.h>
#include <opencv2/highgui.hpp>

#include "ovc.hpp"
#include "jpeg.hpp"

void test_one(bool solid_color = true)
{
	cv::Mat to_mutate;
	if (!solid_color){
		std::string path = "/home/dozer/Pictures/beatles1.jpg";
		cv::Mat src;
		src = cv::imread(path, CV_LOAD_IMAGE_COLOR);
		if(! src.data ) {
			std::cerr <<  "Could not open image" << std::endl ;
			return;
		}
		to_mutate = cv::Mat(src);
	} else {
		to_mutate = cv::Mat (600, 600, CV_8UC3, {255, 0, 255});
	}

	ovc::stuff_1(to_mutate);
	ovc::convolusion(to_mutate, ovc::get_avg, 10, 10);
	ovc::convolusion(to_mutate, ovc::some_pixelate_callback, 10, 10);
	ovc::convolusion(to_mutate, ovc::vortex_callback, 10, 10);
	//ovc::stutter(to_mutate, cv::Rect(340, 250, 1, 20), 200);

	ovc::display(to_mutate);
	cv::imwrite("/home/dozer/Pictures/output5.png", to_mutate);
}


int main(int argc, char *argv[])
{
	test_one(false);
	return 0;
}