
#include <cstdio>
#include <string>
#include <jpeglib.h>



// expecting 8bit per channel
void write_jpeg_to_file( const char* filename, int width, int height, const unsigned char* rgb_data )
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	FILE* outfile = fopen(filename, "wb");

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = width;
	cinfo.image_height     = height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	// setup ---
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality (&cinfo, 75, true); // quality between 0 and 100

	// start compressing ---
	jpeg_start_compress(&cinfo, true);

	JSAMPROW scanline_pointer;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		scanline_pointer = (JSAMPROW) &rgb_data[cinfo.next_scanline*3*width];
		jpeg_write_scanlines(&cinfo, &scanline_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fclose(outfile);
}



// expecting 8bit per channel
// we write to std::string because this is what websocketpp uses for binary blobs...
void write_jpeg_to_memory( int width, int height, const unsigned char* rgb_data, std::string& output )
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;


	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);

	//FILE* outfile = fopen("c:/projects/msc/test.jpeg", "wb");
	//jpeg_stdio_dest(&cinfo, outfile);

	unsigned char *mem = NULL;
	unsigned long mem_size = 0;
	jpeg_mem_dest(&cinfo, &mem, &mem_size);

	cinfo.image_width      = width;
	cinfo.image_height     = height;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	// setup ---
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality (&cinfo, 75, true); // quality between 0 and 100

	// start compressing ---
	jpeg_start_compress(&cinfo, true);

	JSAMPROW scanline_pointer;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		scanline_pointer = (JSAMPROW) &rgb_data[cinfo.next_scanline*3*width];
		jpeg_write_scanlines(&cinfo, &scanline_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);

	output = std::string( mem, mem+mem_size );

	free(mem);
}