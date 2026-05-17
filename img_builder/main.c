#include "raylib.h"

#include <stdio.h>

int main(int argc, char** argv) {
    if (argc < 2) {return 1;}
    printf("Image: %s, Output: %s", argv[1], argv[2]);
    Image img = LoadImage(argv[1]);
    ExportImageAsCode(img, argv[2]);
    UnloadImage(img);
}