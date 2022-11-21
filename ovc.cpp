#include "ovc.h"
#include "mem_hengst.h"

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

void ovc::stuff_one(cv::Mat img, int max, int min, std::vector<cv::Vec3b> color_mod_param)
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
	pid_t pid = getpid();
	std::vector<mem_hengst::Proc_map_entry> proc_map = mem_hengst::get_proc_map_entries(pid);
	
	//mem_hengst::print_pme(proc_map[proc_map_index]);
	size_t data_len =  proc_map[proc_map_index].address_end - proc_map[proc_map_index].address_start - offset;
	uint8_t* data= (uint8_t*)mem_hengst::read_process_mem(pid, proc_map[proc_map_index].address_start + offset, data_len);
	if (!data){
		return cv::Mat();
	}	

	//some bytes will be lost because rounding
	int n_pixels = data_len / num_channels;
	int h = std::min(n_pixels / w, max_h);
	cv::Mat output = cv::Mat(h, w, CV_8UC(num_channels), data).clone();
	
	free(data);

	if (output.cols == 0 || output.rows == 0){
		std::cerr << "Incorrect image size, probably not enough data for requested width " << w << std::endl; 
	}
	return output;
 
}

cv::Mat ovc::to_jpg(cv::Mat input, int quality, std::string tmp_path)
{
    quality = quality > 100 ? 100 : quality;
    quality = quality < 0 ? 0 : quality;

    std::vector<int> params;
    params.push_back(CV_IMWRITE_JPEG_QUALITY);
    params.push_back(quality);

    bool check = cv::imwrite(tmp_path, input, params);
    if (!check) {
        std::cerr << "to_jpg: Failed to save image\n";
        return input;
    }

    cv::Mat image = cv::imread(tmp_path);
    if (image.empty()) {
        std::cerr << "to_jpg: Could not open or find the tmp image" << std::endl;
    }
    return image;

}


// void ovc::stuff_5(cv::Mat img)
// {
// 	cv::Mat gray;
// 	cv::cvtColor(img,gray,CV_RGB2GRAY);
   	
// 	cv::Mat high_contrast;
//     gray.convertTo(high_contrast, -1, 4, 0);

//   	// cv::Mat low_contrast;
//     // gray.convertTo(low_contrast, -1, 0.5, 0);

// 	cv::Mat mask;
// 	inRange(high_contrast, cv::Scalar(255,255,255), cv::Scalar(255,255,255), mask);
// 	high_contrast.setTo(cv::Scalar(255,0,255), mask);

// 	ovc::display(high_contrast);
// }

/**
 * Detects edges and returns image showing them
 *
 * @param input cv::Mat
 * @param input Sensativity, 0-100
 * @return cv::Mat of same dimensions as input, containing the edges
 *               
 */
cv::Mat ovc::get_edges(cv::Mat input, int low_threshold)
{
    const int ratio = 3;
    const int kernel_size = 3;

    cv::Mat detected_edges;
    cv::Canny( input, detected_edges, low_threshold, low_threshold*ratio, kernel_size );
    return detected_edges;
}

cv::Mat ovc::overlay_edges(cv::Mat input, int low_threshold)
{
    cv::Mat dst = input.clone();
    cv::Mat edges = get_edges(input, low_threshold);
    cv::Mat edges_3;
    cv::cvtColor(edges,edges_3,cv::COLOR_GRAY2BGR);
    dst += edges_3;

    return dst;
}

cv::Mat ovc::basic_linear_transformation(cv::Mat input, double contrast, int brightness)
{
    //double alpha = 2.2; //contrast
    //int beta = 0; //brightness
	
	double alpha = contrast;
	int beta = brightness;

	cv::Mat new_image = cv::Mat::zeros(input.size(), input.type());
    for (int y = 0; y < input.rows; y++) {
        for (int x = 0; x < input.cols; x++) {
            for (int c = 0; c < input.channels(); c++) {
                new_image.at<cv::Vec3b>(y, x)[c] =
                    cv::saturate_cast<uchar>(alpha*input.at<cv::Vec3b>(y, x)[c] + beta);
            }
        }
    }
	return new_image;
}

cv::Mat ovc::saturation(cv::Mat input, double saturation, double scale)
{	
	cv::Mat dst;
	input.convertTo(dst, CV_8UC1, scale, saturation); 
	return dst;
}