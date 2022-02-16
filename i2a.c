#include <stdio.h>
#include <math.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <regex.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

#define ASCII_less "M$&#x=+;-'`. "
#define ASCII "M@#W$BG5E20Tbca?1!;:+=-,._` "
#define ASCII_more "M$@WB%8&#Z0OQLCXYUJhkbdpqoawmzcvunxrjft/\\?*-_+~<>1i!lI|;:,\"^`'. "


// argparse info
//
int ascii_value;
int file_option = 0;
const char *argparse_version = "Version 0.0.2 (16.02.2022)";
static char doc[] = "Image to Ascii convertor made in C.\n    ---> https://github.com/tucnakomet1/i2a\n    ---> tucnakomet@gmail.com";


// structure 
//
typedef struct {
  int width;
  int height;
  int channels;
  unsigned int** data;
} Image;

Image loadImage(const char *file_path);
int load(const char *file);
int count_average(int r, int g, int b);



// main Image class → load image and define data
//
Image loadImage(const char *file_path) {
    int width, height, channels;
    unsigned char *data = stbi_load(file_path, &width, &height, &channels, 0);

    // get terminal window size
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w); 
    
    int window_width = w.ws_col;

    // resizing the image
    if (window_width < width) {
        int window_height = round(height/(width / (float)window_width));
        //printf ("Resoluition %dx%d 'll be resized to %dx%d ", width, height, window_width, window_height); //

        Image resolution = {width, height};
        resolution.data = malloc(height * sizeof(int*));

        unsigned char * resized_data = malloc(3 * sizeof(unsigned char) * window_width * window_height);

        int r = stbir_resize_uint8(data, width, height, 0, resized_data, window_width, window_height, 0, 3);

        if (r == 0) { perror("Resizing Error:"); }

        data = resized_data;
        width = window_width;
        height = window_height;
    }

    Image resolution = {width, height};
    resolution.data = malloc(height * sizeof(int*));

    // allocating data [i]
    for (int i = 0; i < height; i++) {
        resolution.data[i] = malloc((width * 3) * sizeof(int));
    }

    // allocating data [i][j]
    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width * 3; i++) {
            resolution.data[j][i] = data[i + (j * width * 3)];
        }
    }

    free(data);
    return resolution;
}


// counting average of RGB
//
int count_average(int r, int g, int b) {
    double simple_avg = (r + g + b)/3;
    int ascii_list_size;

    if (ascii_value == 0) {
        ascii_list_size = sizeof(ASCII) - 2;
    } else if (ascii_value == 1) {
        ascii_list_size = sizeof(ASCII_less) - 2;
    } else {
        ascii_list_size = sizeof(ASCII_more) - 2;
    }
    double rgb_piece = 255.0 / ascii_list_size;
    int avg = round(simple_avg / (float)rgb_piece);

    return avg;
}


// load function → call Image and print result
//
int load(const char *file) {
    Image img = loadImage(file);

    int w, h;
    w = img.width;
    h = img.height;

    //printf("\nW:%d, H:%d\n", w, h); //

    if (w == 0) { return -1; }

    FILE *fp;
    if (file_option == 1) { fp = fopen("i2a.txt", "w+"); }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w*3; j+=3){
            
            int r = img.data[i][j];
            int g = img.data[i][j+1];
            int b = img.data[i][j+2];
            
            //printf("\ntry: R: %d, G: %d, B: %d\n", r, g, b); //

            int avg = count_average(r, g, b);
            int ascii_char;

            if (ascii_value == 0) {
                ascii_char = ASCII[avg];
            } else if (ascii_value == 1) {
                ascii_char = ASCII_less[avg];
            } else {
                ascii_char = ASCII_more[avg];
            }

            printf("%c", ascii_char);
            if (file_option == 1) { fprintf(fp, "%c", ascii_char); }

        }
        printf("\r\n");
        if (file_option == 1) { fprintf(fp, "\r\n"); }
    }
    if (file_option == 1) { fclose(fp); }

    return 0;
}


// main function → check for input arguments and call load function
//

int main(int argc, const char **argv) {
    
    // argparse
    regex_t regex;
    const char *file;
    struct stat sb;
    int reg_val;

    if (argc < 2) {
        printf("Please enter file path...\n [-h] for help\n");
        return -1;
    } else {
        for (int i = 1; i < argc; i++) {

            reg_val = regcomp( &regex, "[.(jpg|jpeg|png|gif|JPG|JPEG|PNG|GIF)]", 0);
            reg_val = regexec( &regex, argv[1], 0, NULL, 0);

            if ((strcmp(argv[i], "-v") == 0) || (strcmp(argv[i], "--version") == 0)&& (i == 1)) {
                printf("%s\n", argparse_version);
                return -1;
            }
            if ((strcmp(argv[i], "-a") == 0) || (strcmp(argv[i], "--about") == 0) && (i == 1)) {
                printf("\n%s\n\n%s\n", argparse_version, doc);
                return -1;
            }
            if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0) && (i == 1)) {
                printf("\nusage: i2a [file] [-h] [-v] [-a] [-f] [-l] [-m]\n");
                printf("\nrequired arguments:\n [file]  file path of your image\n");
                printf("\noptional arguments:\n  -h, --help     show this help message and exit\n  -v, --version  show current version\n  -a, --about    show about page\n  -f, --file     Save result into the file.\n  -l, --less     Use ASCII list with less characters -> higher contrast.\n  -m , --more    Use ASCII list with more characters -> better quality, worse contrast.\n");
                printf("\n<https://github.com/tucnakomet1/i2a>\n");
                return -1;
            }
            if ((i == 1) && (reg_val == 0)) {
                ascii_value = 0;
                file = argv[i];
            }
            if ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--low") == 0) && (i > 1)) {
                ascii_value = 1;
            }
            if ((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--more") == 0) && (i > 1)) {
                ascii_value = 2;
            }
            if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--file") == 0) && (i > 1)) {
                file_option = 1;
            }
        }
    }
      

    // check for path
    struct stat sb;

    if (stat(file, &sb) == 0 && S_ISREG(sb.st_mode)) {
        load(file);
    } else {
        printf("Path: %s - does not exist!\n\n", file);
        return -1;
    }
    
    return 0;
}


/* TODO:
 * - png support
 * - bmp support
 * - gif support
 * - video support
 */
