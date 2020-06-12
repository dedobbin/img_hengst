#include <stdio.h>
#include <opencv/cv.h>
#include <opencv2/highgui.hpp>
#include <unistd.h> 

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
	ovc::stuff_1(to_mutate);

	ovc::display(to_mutate);
	cv::imwrite("/home/dozer/Pictures/output5.png", to_mutate);
}

void test_two()
{
	cv::Mat dst = cv::imread("/home/dozer/Pictures/beatles1.jpg");
	cv::Mat src = cv::imread("/home/dozer/Pictures/beatles2.jpg");
	//cv::Mat src(600, 600, CV_8UC3, {255, 0, 255});
	
	//ovc::stuff_1(src);
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
	std::vector<mem_hengst::Proc_map_entry> proc_map = mem_hengst::get_proc_map_entries(pid);
	
	mem_hengst::print_pme(proc_map[proc_map_index]);

	uint64_t start = proc_map[proc_map_index].address_start;
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

int main(int argc, char *argv[])
{
	// test_one(false);
	// test_two();
	//test_three();
	test_mem_hengst();
	// cv::Mat img(500, 500, CV_8UC3, {255, 0, 255});
	// ovc::pixel_walk(img);
	// ovc::display(img);

	return 0;
}