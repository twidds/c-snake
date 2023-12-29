#include <stdio.h>
#include <string.h>
#include <malloc.h>
//#include <stdarg.h>


//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

void setconsole(int w, int h);

void clearconsole();

void writeconsole(const char* data, int length);

int main() {

    //test code
    const char* testdata = "Gimme a string\r\n";
    fprintf(stdout,testdata);
    
    char input[16];
    fgets(input,sizeof(input), stdin);

    printf("you wrote: %s\r\n",input);
    printf("strlen: %lli\r\n",strlen(input));
    
    int count = 0;
    for (;;count++) {
        if (getchar() == '\n') {break;}
    }
    printf("eaten chars: %i\r\n",count);
    printf("input size: %lli\r\n",sizeof(input));
    

    //set console size
    //write test data to console
    
    //initialize console
    //start game loop

    return 0;
}