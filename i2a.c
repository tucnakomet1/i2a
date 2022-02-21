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
#define ASCII_less_reverse " .`'-;+=x#&$M"
#define ASCII "M@#W$BG5E20Tbca?1!;:+=-,._` "
#define ASCII_reverse " `_.,-=+:;!1?acbT02E5GB$W#@M"
#define ASCII_more "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. "
#define ASCII_more_reverse " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$"

#define ANSI_f_start "\x1b[38;2;"
#define ANSI_b_start "\x1b[48;2;"
#define ANSI_end "\x1b[0m"


// argparse info
//
int ascii_value;
int file_option = false;       // TRUE --> save to file
int color_option = 0;          // 0    --> none; 1 --> foreground; 2 --> background; 3 --> both
bool invert_option = false;    // TRUE --> inverse
const char *argparse_version = "Version 0.0.4 (21.02.2022)";
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
    
    int window_width = (w.ws_col)/2;

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
    double simple_avg = sqrt((r*r + g*g + b*b)/3);
    int ascii_list_size;

    if (ascii_value == 0) { ascii_list_size = sizeof(ASCII) - 2; }
    else if (ascii_value == 1) { ascii_list_size = sizeof(ASCII_less) - 2; }
    else { ascii_list_size = sizeof(ASCII_more) - 2; }
    
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

    if (w == 0) { return -1; }

    FILE *fp;
    if (file_option) { fp = fopen("i2a.txt", "w+"); }

    for (int i = 0; i < h; i++) {
        for (int j = 0; j < w*3; j+=3){
            
            int r = img.data[i][j];
            int g = img.data[i][j+1];
            int b = img.data[i][j+2];
            
            int avg = count_average(r, g, b);
            int ascii_char;
            
            // ASCII identification
            if (ascii_value == 0) { if (invert_option){ ascii_char = ASCII_reverse[avg]; } else { ascii_char = ASCII[avg]; }}
            else if (ascii_value == 1) { if (invert_option) { ascii_char = ASCII_less_reverse[avg]; } else { ascii_char = ASCII_less[avg]; }} 
            else { if (invert_option) { ascii_char = ASCII_more_reverse[avg]; } else { ascii_char = ASCII_more[avg]; }}

            // ANSI identification
            if (color_option == 0) { printf("%c%c", ascii_char, ascii_char); }
            else if (color_option == 1) { printf("%s%d;%d;%dm%c%c%s", ANSI_f_start, r, g, b, ascii_char, ascii_char, ANSI_end); }
            else if (color_option == 2) { printf("%s%d;%d;%dm%c%c%s", ANSI_b_start, r, g, b, ascii_char, ascii_char, ANSI_end); }
            else { printf("%s%d;%d;%dm%s%d;%d;%dm%c%c%s", ANSI_b_start, r, g, b, ANSI_f_start, r, g, b, ascii_char, ascii_char, ANSI_end); }
            
            // write to file
            if (file_option) { fprintf(fp, "%c%c", ascii_char, ascii_char); }

        }
        printf("\r\n");
        if (file_option) { fprintf(fp, "\r\n"); }
    }
    if (file_option) { fclose(fp); }

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
            } if ((strcmp(argv[i], "-a") == 0) || (strcmp(argv[i], "--about") == 0) && (i == 1)) {
                printf("\n%s\n\n%s\n", argparse_version, doc);
                return -1;
            } if ((strcmp(argv[i], "-h") == 0) || (strcmp(argv[i], "--help") == 0) && (i == 1)) {
                printf("\nusage: i2a [file] [-h] [-v] [-a] [-f] [-l] [-m] [-c] [-C] [-Cc] [-i]\n");
                printf("\nrequired arguments:\n [file]  file path of your image\n");
                printf("\noptional arguments:\n  -h,  --help\t\tshow this help message and exit\n  -v,  --version\t\tshow current version\n  -a,  --about\t\tshow about page\n  -f,  --file\t\tSave result into the file.\n  -l,  --less\t\tUse ASCII list with less characters -> higher contrast.\n  -m,  --more\t\tUse ASCII list with more characters -> better quality, worse contrast.\n  -c,  --foreground\tadd foreground ANSI color to image\n  -C,  --background\tadd background ANSi color to image\n  -Cc, --color\t\tAdd foreground and background ANSI color to image\n  -i,  --invert\t\tinvert colors\n");
                printf("\n<https://github.com/tucnakomet1/i2a>\n");
                return -1;
            } if ((i == 1) && (reg_val == 0)) {
                ascii_value = 0;
                file = argv[i];
            } if ((strcmp(argv[i], "-l") == 0) || (strcmp(argv[i], "--low") == 0) && (i > 1)) {
                ascii_value = 1;
            } if ((strcmp(argv[i], "-m") == 0) || (strcmp(argv[i], "--more") == 0) && (i > 1)) {
                ascii_value = 2;
            } if ((strcmp(argv[i], "-f") == 0) || (strcmp(argv[i], "--file") == 0) && (i > 1)) {
                file_option = true;
            } if ((strcmp(argv[i], "-c") == 0) || (strcmp(argv[i], "--foreground") == 0) && (i > 1)) {
                color_option = 1;
            } if ((strcmp(argv[i], "-C") == 0) || (strcmp(argv[i], "--background") == 0) && (i > 1)) {
                color_option = 2;
            } if ((strcmp(argv[i], "-Cc") == 0) || (strcmp(argv[i], "--color") == 0) && (i > 1)) {
                color_option = 3;
            } if ((strcmp(argv[i], "-i") == 0) || (strcmp(argv[i], "--invert") == 0) && (i > 1)) {
                invert_option = true;
            }
        }
    }
      

    // check for path

    if (stat(file, &sb) == 0 && S_ISREG(sb.st_mode)) {
        load(file);
    } else {
        printf("Path: %s - does not exist!\n\n", file);
        return -1;
    }
    
    return 0;
}


/* TODO:
 * - png fix
 * - gif support
 * - video support
 */
