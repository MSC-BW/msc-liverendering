#pragma once



void write_jpeg_to_file( const char* filename, int width, int height, const unsigned char* rgb_data );
void write_jpeg_to_memory( int width, int height, const unsigned char* rgb_data, std::string& output );