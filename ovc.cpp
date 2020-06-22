#include "ovc.hpp"
#include "mem_hengst.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <cmath>
#include <exception>
#include <unistd.h> 
#include <fstream>
#include <regex>

enum direction{
	UP, 
	RIGHT,
	DOWN,
	LEFT,
};

int rand_range(int min, int max)
{
	//todo: seed?
	return min + ( std::rand() % ( max - min + 1 ) );
}

std::vector<cv::Vec3b> flatten (cv::Mat img)
{
	std::vector<cv::Vec3b> output;
	for (int i = 0; i < img.rows; i ++){
		for (int j = 0; j < img.cols; j++){
			output.push_back(img.at<cv::Vec3b>(i, j));
		}
	}
	return output;
}

void ovc::convolusion(cv::Mat img, Conv_callback callback, int w, int h)
{
	int x = 0, y = 0;
	cv::Mat res(img.rows, img.cols, CV_8UC3, {255, 0, 255});

	for(;;){
		for(;;){
			callback(img({x, y, w, h})).copyTo(res({x, y, w, h}));

			x+= w;
			if (x + w > img.cols){
				int leftover_w = w - (x + w - img.cols);
				if (leftover_w){
					cv::Mat sub = callback(img({x, y, leftover_w, h}));
					sub.copyTo(res({x, y, leftover_w, h}));
				}
				break;
			}	
		}
		y+= h;
		x = 0;
		if (y + h > img.rows){
			int leftover = h - (y + h - img.rows);
			if (leftover){
				h = leftover;
			} else {	
				break;
			}
		}
	}
	res.copyTo(img);
}


cv::Mat ovc::get_avg(cv::Mat img)
{
	unsigned int sums[3] = {0, 0, 0};
	for (int i = 0; i < img.rows; i++){
		for (int j = 0; j < img.cols; j++){
			for (int k = 0; k <3; k++){
				sums[k] += img.at<cv::Vec3b>(i,j)[k];
			}
		}
	}

	int divider = img.cols * img.rows;
	cv::Scalar bgr = cv::Scalar({
		sums[0]?  sums[0]/ divider : 0,
		sums[1] ? sums[1]/ divider : 0,
		sums[2] ? sums[2]/ divider : 0,
	});
	return cv::Mat(img.rows, img.cols, CV_8UC3, {bgr[0], bgr[1],bgr[2]});
}

cv::Mat ovc::some_pixelate_callback(cv::Mat img)
{
	cv::Mat res(img);

	for (int i = 0; i < img.rows; i ++){
		for (int j = 0; j < img.cols; j++){
			res.at<cv::Vec3b>(i, j) = img.at<cv::Vec3b>(0,0);

		}
	}
	return res;
}

cv::Mat ovc::vortex_callback(cv::Mat img)
{
	cv::Mat res(img);

	for (int i = 0; i < img.rows; i ++){
		for (int j = 0; j < img.cols; j++){
			int p = j > img.rows ? img.rows : j;
			int q = i > img.cols ? img.cols : i;
			res.at<cv::Vec3b>(i, j) = img.at<cv::Vec3b>(p,q);

		}
	}
	return res;
}

void ovc::display(const cv::Mat img)
{
	cv::namedWindow("display", cv::WINDOW_AUTOSIZE);
	cv::imshow("display", img);
	cv::waitKey(0);
}

void ovc::color_mod(cv::Mat img, const std::vector<cv::Vec3b> mods)
{
	int mods_index = 0;
	for (int i = 0; i < img.rows; i++){
		for (int j = 0; j < img.cols; j++){
			cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
			for (int k = 0; k <img.channels(); k++){
				pixel[k] += mods[mods_index][k];
			}
			img.at<cv::Vec3b>(i, j) = pixel;
			mods_index + 1 >= mods.size() ? 0: mods_index += 1;
		}
	}
}

void ovc::insert(cv::Mat dst, cv::Mat const src, const cv::Rect pos, int wonk_spacing)
{
	bool do_draw = true;
	for (int i = pos.y; i < pos.y + pos.height; i++){
		for (int j = pos.x; j < pos.x + pos.width; j++){
			if (wonk_spacing && (i + j) % wonk_spacing == 0){
				do_draw = !do_draw;
			}
			
			if (do_draw){
				dst.at<cv::Vec3b>(i, j) = src.at<cv::Vec3b>(i, j);
			}
		}
	}
}

void ovc::stuff_1(cv::Mat img, int max, int min, std::vector<cv::Vec3b> color_mod_param)
{
	for (int i = max; i >= min; i-- ){
		cv::Mat altered = img.clone();
		color_mod(altered, color_mod_param);
		insert(img, altered, {0, 0 ,img.cols, img.rows}, i);
	}
}

void ovc::stutter(cv::Mat img, cv::Rect src_rect, int x_times)
{
	cv::Rect dst_rect = src_rect;
	cv::Mat sub = img(src_rect);

	for (int i = 0; i < x_times; i++){
		sub.copyTo(img(dst_rect));
		dst_rect.x += dst_rect.width;
		
		if (dst_rect.x + dst_rect.width > img.cols-1){
			int left_over = dst_rect.x + dst_rect.width - img.cols;
			cv::Rect tmp_src = src_rect;
			tmp_src.width = left_over;
			cv::Mat tmp_sub = img(tmp_src);
			
			cv::Rect tmp_dst = dst_rect;
			tmp_dst.width = left_over;
			tmp_sub.copyTo(img(tmp_dst));

			break;
		}
	}
}

bool check_collision(cv::Rect a, cv::Rect b)
{
	int x1 = a.x;
	int x2 = a.x  + a.width;
	int x3 = b.x;
	int x4 = b.x + b.width;
	
	int y1 = a.y;
	int y2 = a.y + a.height;
	int y3 = b.y;
	int y4 = b.y + b.height;

	return (! (x3 > x2 || y3 > y2 || x1 > x4 || y1 > y4));
}

//WIP
void ovc::pixel_walk(cv::Mat img)
{
	std::cerr << "wip" << std::endl;
	exit(1);

	cv::Mat src_img = cv::imread("/home/dozer/Pictures/beatles1.jpg");
	ovc::convolusion(src_img, ovc::some_pixelate_callback, 1, 10);
	std::vector<cv::Vec3b> flat = flatten(src_img);

	int max_inserts = 5000;

	direction dir = DOWN;

	std::vector<cv::Rect> path;

	int x = img.cols/2, y = img.rows/2, w = 7, h = 7;
	for (;;){
		path.push_back({x, y, w, h});

		//random change dir sometimes
		if (max_inserts % 5 == 0){
			dir = (direction)rand_range(0,3);
		}

		switch (dir){
			case (UP):
				y -= h;
			break;
			case (RIGHT):
				x += w;
			break;
			case (DOWN):
				y += h;
			break;
			case (LEFT):
				x -= w;
			break;
		}

		if (!max_inserts--){
			break;
		}
	}

	int i = 0;
	for (auto pos : path){
		if (pos.x + w > img.cols || pos.y + h > img.rows || pos.x < 0 || pos.y < 0){
			std::cout << "Cannot insert" << std::endl;
			//TODO: partial insert?
		} else {
			src_img({pos.x, pos.y, pos.width, pos.height}).copyTo(img({pos.x, pos.y, pos.width, pos.height}));
			//img.at<cv::Vec3b>(pos.y, pos.x) = src_img.at(i);
		}
		i++;
	}
}

void ovc::blend_insert(cv::Mat dst, cv::Mat src, int insert_x, int insert_y)
{
	for (int i = 0; i < src.rows; i++){
		for (int j = 0; j < src.cols; j++){
			//dst.at<cv::Vec3b>(insert_y + i , insert_x + j) = src.at<cv::Vec3b>(i, j);
			cv::Vec3b old_pixel = src.at<cv::Vec3b>(i, j);
			cv::Vec3b new_pixel = dst.at<cv::Vec3b>(i + insert_y, j + insert_x);
			for (int k = 0; k < 3; k++){
				dst.at<cv::Vec3b>(i + insert_y, j +insert_x)[k] = (new_pixel[k] + old_pixel[k]) / 2;
			}
			
		}
	}
}

cv::Mat ovc::stuff_generator(int proc_map_index, int w, int max_h, int offset)
{
	int num_channels = 3;
 
}

void ovc::stuff_5(cv::Mat img)
{
	cv::Mat gray;
	cv::cvtColor(img,gray,CV_RGB2GRAY);
   	
	cv::Mat high_contrast;
    gray.convertTo(high_contrast, -1, 4, 0);

  	// cv::Mat low_contrast;
    // gray.convertTo(low_contrast, -1, 0.5, 0);

	cv::Mat mask;
	inRange(high_contrast, cv::Scalar(255,255,255), cv::Scalar(255,255,255), mask);
	high_contrast.setTo(cv::Scalar(255,0,255), mask);

	ovc::display(high_contrast);
}