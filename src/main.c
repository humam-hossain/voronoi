#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

#define WIDTH 800
#define HEIGHT 600
#define COLOR_RED 0xFF0000FF
#define OUTPUT_FILE_PATH "output.ppm"

typedef uint32_t Color32;

static Color32 image[HEIGHT][WIDTH];

void fill_image(Color32 color)
{
    for(size_t y = 0; y < HEIGHT; ++y){
        for(size_t x = 0; x < WIDTH; ++x){
            image[y][x] = color;
        }
    }
}

void save_image_as_ppm(const char *file_path)
{
    FILE *f = fopen(file_path, "wb");

    if(f == NULL){
        fprintf(stderr, "ERROR: could not write into file %s: %s\n", file_path, strerror(errno));
        exit(1);
    }

    fprintf(f, "P6\n%d %d 255\n", WIDTH, HEIGHT);
    for(size_t y = 0; y < HEIGHT; ++y){
        for(size_t x = 0; x < WIDTH; ++x){
            // 0xAABBGGRR
            uint32_t pixel = image[y][x];
            uint32_t bytes[3] = {
                (pixel&0x0000FF)>>8*0,
                (pixel&0x00FF00)>>8*1,
                (pixel&0xFF0000)>>8*2,
            };
            fwrite(bytes, sizeof(bytes), 1, f);
            assert(!ferror(f));
        }
    }

    int ret = fclose(f);
    assert(ret == 0);
}

int main()
{
    fill_image(COLOR_RED);
    save_image_as_ppm(OUTPUT_FILE_PATH);

    return 0;
}