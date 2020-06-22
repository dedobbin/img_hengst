#include <stdio.h>
#include <opencv/cv.h>
#include <opencv2/highgui.hpp>
#include<opencv2/opencv.hpp>
#include <unistd.h> 
#include <sstream>

#include "ovc.hpp"
#include "jpeg.hpp"
#include "mem_hengst.hpp"

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

	ovc::convolusion(to_mutate, ovc::get_avg, 10, 10);
	ovc::convolusion(to_mutate, ovc::vortex_callback, 20, 20);
	ovc::convolusion(to_mutate, ovc::some_pixelate_callback, 10, 10);
	ovc::stutter(to_mutate, cv::Rect(340, 250, 1, 20), 200);
	ovc::stuff_one(to_mutate);

	ovc::display(to_mutate);
	cv::imwrite("/home/dozer/Pictures/output5.png", to_mutate);
}

void test_two()
{
	cv::Mat dst = cv::imread("/home/dozer/Pictures/beatles1.jpg");
	cv::Mat src = cv::imread("/home/dozer/Pictures/beatles2.jpg");
	//cv::Mat src(600, 600, CV_8UC3, {255, 0, 255});
	
	//ovc::stuff_one(src);
	ovc::blend_insert(dst, src, 0, 0);
	ovc::display(dst);
	cv::imwrite("/home/dozer/Pictures/some_output.png", dst);
}

void test_three() 
{
	cv::Mat img = ovc::stuff_generator();
	ovc::convolusion(img, ovc::some_pixelate_callback, 3, 3);
	ovc::display(img);
	cv::imwrite("/home/dozer/Pictures/some_output.png", img);
}

void test_mem_hengst()
{
	int proc_map_index = 12;

	pid_t pid = getpid();
	std::vector<mem_hengst::Proc_map_entry> mem_map = mem_hengst::get_proc_map_entries(pid);
	
	mem_hengst::print_pme(mem_map[proc_map_index]);

	if (mem_map[proc_map_index].permissions.substr(1,1) != "w"){
		std::cout << "No permission to write to memory" << std::endl;
		return;
	}

	uint64_t start = mem_map[proc_map_index].address_start;
	int buffer_len = 10;
	
	uint8_t* data= (uint8_t*)mem_hengst::read_process_mem(pid, start, buffer_len);
	printf("Read '%s'\n", data);

	uint8_t new_data[10] = "test";
	printf("Will write '%s'\n", new_data);
	mem_hengst::write_process_mem(pid, start, new_data, buffer_len);	

	free(data);
	data= (uint8_t*)mem_hengst::read_process_mem(pid, start, buffer_len);
	printf("Read '%s'\n", data);
	free(data);
}

void test_five()
{
	// cv::Mat img = ovc::stuff_generator(0, 400);
	// cv::imwrite("/home/dozer/Pictures/mem324872389478923.png", img);
	// ovc::display(img);
	
	// int loop_times = 40;
	// int offset = 100;
	// int w = 1200;
	srand(time(NULL));
	int n = 10;
	int w = 900;
	cv::Mat result;
	for (int j = 0; j <  n; j++){
	result = cv::Mat(0, 0, CV_8UC3, {255, 0, 255});
		int loop_times = 20 + rand() % (( 70 + 1 ) - 20);
		int offset = 100 + rand() % (( 200 + 1 ) - 100);

		std::cout << offset << " - " << offset + loop_times << std::endl;
		
		for (int i = offset; i < loop_times + offset; i++){
			cv::Mat data = ovc::stuff_generator(i, w);
			result.push_back(data); 
		}
		std::cout << "writing " << result.cols << "x" << result.rows << std::endl;
		std::stringstream fn1;
		fn1 << "/home/dozer/Pictures/output" << j << ".png";
		cv::imwrite(fn1.str(), result);
		
		ovc::convolusion(result, ovc::some_pixelate_callback, 3, 3);


		std::stringstream fn2;
		fn2 << "/home/dozer/Pictures/output" << j << "_chunky.png";
		cv::imwrite(fn2.str(), result);

	}
	//ovc::display(result);
}

int main(int argc, char *argv[])
{
	// test_one(false);
	// test_two();
	// test_three();
	 test_five();
	return 0;
}