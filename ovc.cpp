#include "ovc.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include <vector>
#include <cmath>
#include <exception>
#include <unistd.h> 
#include <fstream>
#include <regex>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <sys/uio.h>

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


struct Proc_map_entry
{
	uint64_t address_start;
	uint64_t address_end;
	std::string permissions;
	std::string offset;
	std::string device;
	std::string inode;
	std::string pathname;
};

void print_pme(Proc_map_entry pme)
{
	std::cout << pme.address_start << "-" << pme.address_start << std::endl;
	std::cout << "Permissions: \t " << pme.permissions << std::endl;
	std::cout << "Offset: \t " << pme.offset << std::endl;
	std::cout << "Device: \t " << pme.device << std::endl;
	std::cout << "inode: \t\t " << pme.inode << std::endl;
	std::cout << "pathname: \t " << pme.pathname << std::endl;
}

std::vector<Proc_map_entry> get_proc_map_entries(pid_t pid)
{
	std::vector<Proc_map_entry> res;
	std::ifstream file("/proc/" + std::to_string(pid) + "/maps");
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			//printf("%s\n", line.c_str());
			std::regex regex{R"([ ]+)"};
			std::sregex_token_iterator it{line.begin(), line.end(), regex, -1};
			std::vector<std::string> splits{it, {}};

			std::regex regex2{R"([-]+)"};
			std::sregex_token_iterator it2{splits[0].begin(), splits[0].end(), regex2, -1};
			std::vector<std::string> address_range{it2, {}};

			Proc_map_entry pme{
				strtol( address_range[0].c_str(), NULL, 16 ),
				strtol( address_range[1].c_str(), NULL, 16 ),
				splits[1],
				splits[2],
				splits[3],
				splits[4],
				splits.size() == 6 ? splits[5] : "",
			};
			res.push_back(pme);
		}
		file.close();
	}
	return res;
}

void* get_process_mem(pid_t pid, uint64_t address_start, size_t bufferLength)
{
    void *remotePtr = (void *)address_start;

    struct iovec local[1];
    local[0].iov_base = calloc(bufferLength, sizeof(char));
    local[0].iov_len = bufferLength;

    struct iovec remote[1];
    remote[0].iov_base = remotePtr;
    remote[0].iov_len = bufferLength;

    ssize_t nread = process_vm_readv(pid, local, 2, remote, 1, 0);
    if (nread < 0) {
        switch (errno) {
            case EINVAL:
              std::cerr << "ERROR: INVALID ARGUMENTS" << std::endl;
              break;
            case EFAULT:
              std::cerr << "ERROR: UNABLE TO ACCESS TARGET MEMORY ADDRESS" << std::endl;
              break;
            case ENOMEM:
              std::cerr << "ERROR: UNABLE TO ALLOCATE MEMORY" << std::endl;
              break;
            case EPERM:
              std::cerr << "ERROR: INSUFFICIENT PRIVILEGES TO TARGET PROCESS" << std::endl;
              break;
            case ESRCH:
              std::cerr << "ERROR: PROCESS DOES NOT EXIST" <<std::endl;
              break;
            default:
              std::cerr << "ERROR: AN UNKNOWN ERROR HAS OCCURRED" << std::endl;
        }

        return NULL;
    }
    //printf("%s\n", local[0].iov_base);

    return local[0].iov_base;
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

cv::Mat ovc::stuff_generator()
{
	int num_channels = 3;
	int proc_i = 3;
	int w = 900;
	int max_h = 600;
	int offset = 2000000;	//data offset to read from

	pid_t pid = getpid();
	std::vector<Proc_map_entry> proc_map = get_proc_map_entries(pid);
	
	print_pme(proc_map[proc_i]);
	size_t data_len =  proc_map[proc_i].address_end - proc_map[proc_i].address_start - offset;
	uint8_t* data= (uint8_t*)get_process_mem(pid, proc_map[proc_i].address_start + offset, data_len);
	
	//printf("%s\n",data);	

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