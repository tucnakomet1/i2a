#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define ASCII "M@#W$BG5E20Tbca?1!;:+=-,._` "
#define ASCII_full "M$@WB%8&#Z0OQLCXYUJhkbdpqoawmzcvunxrjft/\\?*-_+~<>1i!lI|;:,\"^`'. "
#define ASCII_low "M$&#x=+;-'`. "
#define ASCII_real "███▓▓▒░░  "


typedef struct {
  int width;
  int height;
  unsigned int** data;
} Image;

// all functions and classes
Image loadImage(const char *file_path);
int load(const char *file);
int ascii_list_size = sizeof(ASCII_full) - 2;

// main image class → load image and define data
Image loadImage(const char *file_path) {
    // load image
    int width, height, channels;
    unsigned char *data = stbi_load(file_path, &width, &height, &channels, 0);

    // image resolution counter
    Image resolution = {width, height};
    resolution.data = malloc(height * sizeof(int*));

    // adding data to [i]
    for (int i = 0; i < height; i++) {
        resolution.data[i] = malloc((width * 3) * sizeof(int));
    }

    // adding data to [i][j]
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width * 3; i++) {
            resolution.data[j][i] = data[i + (j * width * 3)];
        }
    }

    free(data);
    return resolution;
}

int count_average(int r, int g, int b) {
    double simple_avg = (r + g + b)/3;
    double rgb_piece = 255.0 / ascii_list_size;
    double devided = simple_avg / rgb_piece;
    int avg = round(devided);

    //printf("\nsimple_avg: %f, rgb_piece: %f, devided: %f, avg: %d\n", simple_avg, rgb_piece, devided, avg);

    return avg;
}


int load(const char *file) {
    Image img = loadImage(file);

    int w, h;
    w = img.width;
    h = img.height;

    printf("\nW:%d, H:%d\n", w, h);

    if (w == 0) {
        return -1;
    }


    for (int i = 0; i < h; i++) {
        char str[302] = "";

        for (int j = 0; j < w; j++) {
            //printf("\nI:%d, J:%d", i, j);
            
            int r = img.data[i][j];
            int g = img.data[i][j+1];
            int b = img.data[i][j+2];
            
            //printf("\ntry: R: %d, G: %d, B: %d\n", r, g, b);

            int avg = count_average(r, g, b);
            //int ascii_char = ASCII[avg];
            int ascii_char = ASCII_full[avg];

            strncat(str, &ascii_char, 1);
        }
        printf("%s\n", str);
    }

    return 0;

}

int main() {

    // check for path
    const char *file;
    struct stat sb;

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    printf ("lines %d\n", w.ws_row);
    printf ("columns %d\n", w.ws_col);

    //file = "/home/tucna/Plocha/i2a/red.jpg";
    //file = "/home/tucna/Plocha/i2a/black.png";
    //file = "/home/tucna/Plocha/i2a/white.png";
    //file = "/home/tucna/Plocha/square.jpg";
    file = "/home/tucna/Plocha/i2a/inputs/circle.jpg";
    //file = "/home/tucna/Plocha/i2a/inputs/man.jpg";




    //if path → run process; else → input file again
    if (stat(file, &sb) == 0 && S_ISREG(sb.st_mode)) {
        load(file);
    } else {
        printf("\nWrong file path! Try it again...\n");
        printf("Path: %s - does not exist", file);
	main();
    }
    
    return 0;
}


/* TODO:
 * - resize image by window size
 * - show full image, not just half
 * - add argparse
 * - options of ascii list
 * - save to file
 * - make video
 * - add about → version, url
 */
