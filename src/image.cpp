#include "image.hpp"

ARGBImage::~ARGBImage() {
    delete data;
}

ARGBImage::ARGBImage( XImage* image, glm::ivec2 iloc, glm::ivec4 selectionrect ) {
    glm::ivec2 spos = glm::ivec2( selectionrect.x, selectionrect.y );
    glm::ivec2 ssize = glm::ivec2( selectionrect.z, selectionrect.w );
    glm::ivec2 offset = spos-iloc;
    long long int alpha_mask = ~(image->red_mask|image->green_mask|image->blue_mask);
    long long int roffset = get_shift(image->red_mask);
    long long int goffset = get_shift(image->green_mask);
    long long int boffset = get_shift(image->blue_mask);
    long long int aoffset = get_shift(alpha_mask);

    // Zero our data.
    data = new unsigned char[ssize.x*ssize.y*4];
    for ( unsigned int i = 0; i < ssize.x*ssize.y*4; i++ ) {
        data[i] = 0;
    }

    // Find the intersection of the rectangles.
    int maxx = glm::max( offset.x, 0 );
    int maxy = glm::max( offset.y, 0 );
    int minw = glm::min( offset.x+ssize.x, image->width );
    int minh = glm::min( offset.y+ssize.y, image->height );

    width = ssize.x;
    height = ssize.y;
    if ( aoffset >= image->depth ) {
        // Loop only through the intersecting parts, copying everything.
        for ( unsigned int y = maxy; y < minh; y++ ) {
            for ( unsigned int x = maxx; x < minw; x++ ) {
                unsigned int real = XGetPixel(image, x, y);
                int curpixel = ((y-offset.y)*ssize.x+((x-offset.x)))*4;
                data[curpixel] = (unsigned char)((real & image->red_mask) >> roffset);
                data[curpixel+1] = (unsigned char)((real & image->green_mask) >> goffset);
                data[curpixel+2] = (unsigned char)((real & image->blue_mask) >> boffset);
                data[curpixel+3] = 255;
            }
        }
    } else {
        // Loop only through the intersecting parts, copying everything.
        for ( unsigned int y = maxy; y < minh; y++ ) {
            for ( unsigned int x = maxx; x < minw; x++ ) {
                unsigned int real = XGetPixel(image, x, y);
                int curpixel = ((y-offset.y)*ssize.x+(x-offset.x))*4;
                data[curpixel] = (unsigned char)((real & image->red_mask) >> roffset);
                data[curpixel+1] = (unsigned char)((real & image->green_mask) >> goffset);
                data[curpixel+2] = (unsigned char)((real & image->blue_mask) >> boffset);
                data[curpixel+3] = (unsigned char)((real & alpha_mask) >> aoffset);
            }
        }
    }
}

void png_write_ostream(png_structp png_ptr, png_bytep data, png_size_t length)
{
    std::ostream *stream = (std::ostream*)png_get_io_ptr(png_ptr); //Get pointer to ostream
    stream->write((char*)data,length); //Write requested amount of data
}

void png_flush_ostream(png_structp png_ptr)
{
    std::ostream *stream = (std::ostream*)png_get_io_ptr(png_ptr); //Get pointer to ostream
    stream->flush();
}

void user_error_fn(png_structp png_ptr, png_const_charp error_msg)
{
    throw new std::runtime_error(error_msg);
}

void user_warning_fn(png_structp png_ptr, png_const_charp warning_msg)
{
    std::cerr << warning_msg << "\n";
}

void ARGBImage::writePNG( std::ostream& streamout ) {
    png_structp png = NULL;
	png_infop info = NULL;
	png_bytep *rows = new png_bytep[height];

	png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) throw new std::runtime_error( "Failed to write png image" );
	info = png_create_info_struct(png);
	if(!info) throw new std::runtime_error( "Failed to write png image" );
    png_set_error_fn(png, png_get_error_ptr(png), user_error_fn, user_warning_fn);
    png_set_write_fn(png, &streamout, png_write_ostream, png_flush_ostream);
	png_set_compression_level(png, 9);
	png_set_IHDR(png, info, width, height,
       8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
       PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
    for( int i=0;i<height;i++ ) {
        rows[i] = (png_bytep)(data+(i*width*4));
    }
    png_write_info(png, info);
    png_write_image(png, rows);
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);
}