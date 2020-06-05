#ifndef __OVC_HPP__
#define __OVC_HPP__

#include <iostream>
#include <opencv/cv.h>
#include <opencv2/highgui.hpp>

namespace ocv
{
	void display(const cv::Mat img);

	void color_mod(cv::Mat img, const std::vector<cv::Vec3b> mods);
	void wonk_insert(cv::Mat dst, cv::Mat const src, const cv::Rect pos, int wonk_spacing = 0);
	void stuff_1(cv::Mat img, int max = 100, int min = 90, std::vector<cv::Vec3b> color_mod_param = {{100, 0, 70}});
	void stutter(cv::Mat img, cv::Rect src_rect = cv::Rect(340, 250, 1, 20), int x_times = 99999);
	void downsample(cv::Mat img, int w = 5, int h = 5);
	void insert(cv::Mat dst, cv::Mat const src, const cv::Rect pos, int wonk_spacing=0);

}
#endif 