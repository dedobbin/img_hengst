#ifndef __OVC_HPP__
#define __OVC_HPP__

#include <iostream>
#include <opencv/cv.h>
#include <opencv2/highgui.hpp>

typedef cv::Mat (*Conv_callback)(cv::Mat img);

namespace ovc
{
	void display(const cv::Mat img);

	void color_mod(cv::Mat img, const std::vector<cv::Vec3b> mods);
	void wonk_insert(cv::Mat dst, cv::Mat const src, const cv::Rect pos, int wonk_spacing = 0);
	void stuff_one(cv::Mat img, int max = 100, int min = 90, std::vector<cv::Vec3b> color_mod_param = {{100, 0, 70}});	//Works best? with solid color input
	void stutter(cv::Mat img, cv::Rect src_rect = cv::Rect(340, 250, 1, 20), int x_times = 99999);
	void insert(cv::Mat dst, cv::Mat const src, const cv::Rect pos, int wonk_spacing=0);
	void stuff_5(cv::Mat img);
	
	void blend_insert(cv::Mat dst, cv::Mat src, int insert_x, int insert_y);
	void convolusion(cv::Mat img, Conv_callback callback, int w, int h);
	
	//conv callbacks
	cv::Mat get_avg(cv::Mat img);	//downsampling
	cv::Mat some_pixelate_callback(cv::Mat img);	//weird downsampling
	cv::Mat vortex_callback(cv::Mat img);

	//generators
	cv::Mat stuff_generator(int proc_map_index = 3, int w = 900, int max_h = 600, int offset = 0);
}

#endif 