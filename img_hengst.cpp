#include "img_hengst.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <cmath>
#include <exception>

int rand_range(int min, int max)
{
	return min + ( std::rand() % ( max - min + 1 ) );
}

cv::Mat get_avg(cv::Mat img)
{
	unsigned int sums[3] = {0, 0, 0};
	for (int i = 0; i < img.rows; i++){
		for (int j = 0; j < img.cols; j++){
			for (int k = 0; k <3; k++){
				sums[k] += img.at<cv::Vec3b>(i,j)[k];
			}
		}
	}

	//TODO: get rid of warning
	int divider = img.cols * img.rows;
	cv::Scalar bgr = cv::Scalar({
		sums[0]?  sums[0]/ divider : 0,
		sums[1] ? sums[1]/ divider : 0,
		sums[2] ? sums[2]/ divider : 0,
	});
	return cv::Mat(img.rows, img.cols, CV_8UC3, {bgr[0], bgr[1],bgr[2]});
}

//-------------------------------------------------------------------
void display(const cv::Mat img)
{
	cv::namedWindow("display", cv::WINDOW_AUTOSIZE);
	cv::imshow("display", img);
	cv::waitKey(0);
}

void color_mod(cv::Mat img, const std::vector<cv::Vec3b> mods)
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

void insert(cv::Mat dst, cv::Mat const src, const cv::Rect pos, int wonk_spacing)
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

//Works good with solid color input
void stuff_1(cv::Mat img, int max, int min, std::vector<cv::Vec3b> color_mod_param)
{
	for (int i = max; i >= min; i-- ){
		cv::Mat altered = img.clone();
		color_mod(altered, color_mod_param);
		insert(img, altered, {0, 0 ,img.cols, img.rows}, i);
	}
}

void stutter(cv::Mat img, cv::Rect src_rect, int x_times)
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

void downsample(cv::Mat img, int w, int h)
{	
	//TODO: make params
	int x = 0, y = 0;
	cv::Mat res(img.rows, img.cols, CV_8UC3, {255, 0, 255});

	for(;;){
		for(;;){
			get_avg(img({x, y, w, h})).copyTo(res({x, y, w, h}));

			x+= w;
			if (x + w > img.cols){
				int leftover_w = w - (x + w - img.cols);
				cv::Mat sub = get_avg(img({x, y, leftover_w, h}));
				sub.copyTo(res({x, y, leftover_w, h}));
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