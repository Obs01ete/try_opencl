#ifndef TRY_OPENCL_PNG_WRAPPER_H
#define TRY_OPENCL_PNG_WRAPPER_H

#include <vector>

#define PNG_DEBUG 3
#include "png.h"

class Png {

private:

    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;

public:

    void read_png_file(char *file_name);

    void write_png_file(char *file_name);

    std::vector<unsigned char> process_file(int w, int h);
};

#endif //TRY_OPENCL_PNG_WRAPPER_H
