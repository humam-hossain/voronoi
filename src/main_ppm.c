#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <limits.h>

#define WIDTH 800
#define HEIGHT 600
#define SEEDS_COUNT 20
#define SEED_MARKER_RADIUS 5

#define COLOR_WHITE 0xFFFFFFFF
#define COLOR_BLACK 0xFF000000
#define COLOR_RED   0xFF0000FF
#define COLOR_GREEN 0xFF00FF00
#define COLOR_BLUE  0xFFFF0000
#define BACKGROUND_COLOR 0xFF181818
#define SEED_MARKER_COLOR COLOR_BLACK

#define GRUVBOX_BRIGHT_RED     0xFF3449FB  
#define GRUVBOX_BRIGHT_GREEN   0xFF26BBB8  
#define GRUVBOX_BRIGHT_YELLOW  0xFF2FBDFA  
#define GRUVBOX_BRIGHT_BLUE    0xFF98A583  
#define GRUVBOX_BRIGHT_PURPLE  0xFF9B86D3  
#define GRUVBOX_BRIGHT_AQUA    0xFF7CC08E  
#define GRUVBOX_BRIGHT_ORANGE  0xFF1980FE

#define OUTPUT_FILE_PATH "output.ppm"

typedef uint32_t Color32;
typedef struct {
    int x, y;
} Point;

static Color32 image[HEIGHT][WIDTH];
static int depth[HEIGHT][WIDTH];
static Point seeds[SEEDS_COUNT];
static Color32 palette[] = {
    GRUVBOX_BRIGHT_RED,
    GRUVBOX_BRIGHT_GREEN,
    GRUVBOX_BRIGHT_AQUA,
    GRUVBOX_BRIGHT_YELLOW,
    GRUVBOX_BRIGHT_BLUE,
    GRUVBOX_BRIGHT_PURPLE,
    GRUVBOX_BRIGHT_ORANGE,
};

int palette_count = sizeof(palette)/sizeof(palette[0]);

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
            uint8_t bytes[3] = {
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

void generate_random_seeds()
{
    for(size_t i = 0; i < SEEDS_COUNT; ++i){
        seeds[i].x = rand() % WIDTH;
        seeds[i].y = rand() % HEIGHT;
    }
}

int sqr_dist(int x1, int y1, int x2, int y2){
    int dx = x1 - x2;
    int dy = y1 - y2;

    return dx*dx + dy*dy;
}

void fill_circle(int cx, int cy, int radius, Color32 color){
    int x0 = cx - radius;
    int y0 = cy - radius;
    int x1 = cx + radius;
    int y1 = cy + radius;

    for(int y = y0; y <= y1; ++y){
        if(0 <= y && y < HEIGHT){
            for(int x = x0; x <= x1; ++x){
                if(0 <= x && x < WIDTH){
                    if(sqr_dist(cx, cy, x, y) <= radius*radius){
                        image[y][x] = color;
                    }
                }
            }
        }
    }
}

void render_seed_markers()
{
    for(size_t i = 0; i < SEEDS_COUNT; ++i){
        fill_circle(seeds[i].x, seeds[i].y, SEED_MARKER_RADIUS, SEED_MARKER_COLOR);
    }
}

void render_voronoi_naive()
{
    for(int y = 0; y < HEIGHT; ++y){
        for(int x = 0; x < WIDTH; ++x){
            int j = 0;
            for(int i = 0; i < SEEDS_COUNT; ++i){
                if(sqr_dist(seeds[i].x, seeds[i].y, x, y) < sqr_dist(seeds[j].x, seeds[j].y, x, y)){
                    j = i;
                }    
            }

            image[y][x] = palette[j % palette_count];
        }
    }
}


Color32 point_to_color(Point p)
{
    assert(p.x >= 0 && p.x < UINT16_MAX);
    assert(p.y >= 0 && p.y < UINT16_MAX);

    uint16_t x = p.x;
    uint16_t y = p.y;

    return (y<<16) | x;
}

Point color_to_point(Color32 color)
{
    return (Point) {
        .x = (color & 0x0000FFFF) >> 0,
        .y = (color & 0xFFFF0000) >> 16
    };
}

void render_point_gradient()
{
    for(int y = 0; y < HEIGHT; ++y){
        for(int x = 0; x < WIDTH; ++x){
            Point p = {x, y};
            image[y][x] = point_to_color(p);
        }
    }
}

void apply_next_seed(size_t seed_index)
{
    Point seed = seeds[seed_index];
    Color32 color = palette[seed_index%palette_count];

    for(int y = 0; y < HEIGHT; ++y){
        for(int x = 0; x < WIDTH; ++x){
            int d = sqr_dist(seed.x, seed.y, x, y);
            if(d < depth[y][x]){
                depth[y][x] = d;
                image[y][x] = color;
            }
        }
    }
}

void render_voronoi_interesting()
{
    for(int y = 0; y < HEIGHT; ++y){
        for(int x = 0; x < WIDTH; ++x){
            depth[y][x] = INT_MAX;
        }
    }

    for(int i = 0; i < SEEDS_COUNT; ++i){
        apply_next_seed(i);
    }
}

int main()
{
    srand(time(0));

    fill_image(BACKGROUND_COLOR);
    generate_random_seeds();
    render_voronoi_interesting();
    render_seed_markers();
    save_image_as_ppm(OUTPUT_FILE_PATH);

    return 0;
}