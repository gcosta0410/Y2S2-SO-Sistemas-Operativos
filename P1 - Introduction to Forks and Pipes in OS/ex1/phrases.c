#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{   
    FILE* fp;
    int counter = 0;
    char c;

    /* check for correct input */
    switch (argc){
    case 2:{
        char *fileName = argv[1];

        /* check if the file is valid */
        fp = fopen(fileName, "r");
        if (fp == NULL){
            printf("error: cannot open %s\n", fileName);
            return EXIT_FAILURE;
        }

        /* Check if the file is empty*/
        if(fseek(fp, 0, SEEK_END) == -1){
            printf("%s: Error in fseek(fp, 0, SEEK_END)", __func__);
            return EXIT_FAILURE;
        }   

        size_t size;
        if((size = ftell(fp)) == -1){
            printf("%s: Error in ftell(fp)", __func__);
            return EXIT_FAILURE;
        }

        if(size == 0) {
            printf("%s: file is empty!\n", fileName);
            return EXIT_FAILURE;
        }

        /* Set pointer back to start of file */
        fseek(fp, 0, SEEK_SET);

        /* Extract characters from file and store in char c */
        while(!feof(fp)){
            c = fgetc(fp);
            if (c == '!' || c == '.' || c == '?' || c == EOF)
                counter = counter + 1;
        }
        printf("%d\n", counter);
        break;
    }
    case 3:{
        char *fileName = argv[2];

        if(strcmp(argv[1], "-l") != 0){
            printf("usage: phrases [-l] file\n");
            return EXIT_FAILURE;
        }

        /* check if the file is valid */
        fp = fopen(fileName, "r");
        if (fp == NULL){
            printf("error: cannot open %s\n", fileName);
            return EXIT_FAILURE;
        }

        /* Check if the file is empty*/
        if(fseek(fp, 0, SEEK_END) == -1){
            printf("%s: Error in fseek(fp, 0, SEEK_END)", __func__);
            return EXIT_FAILURE;
        }   

        size_t size;
        if((size = ftell(fp)) == -1){
            printf("%s: Error in ftell(fp)", __func__);
            return EXIT_FAILURE;
        }

        if(size == 0) {
            printf("%s: file is empty!\n", fileName);
            return EXIT_FAILURE;
        }

        /* Set pointer back to the start of the file */
        fseek(fp, 0, SEEK_SET);

        /* Extract characters from file and store in char c */
        printf("[%d] ", counter+1);
        while(!feof(fp)){
            c = fgetc(fp);
            if (c == '!' || c == '.' || c == '?' || c == EOF){
                counter = counter + 1;
                if(c != EOF ) printf("%c\n[%d]", c, counter+1);
            }
            else if(c != '\n' && c != '\t' && c!= '\r'){
                printf("%c",c);
            }
            else{
                printf(" ");
            }
        }
        printf("\n");
        break;
    }
    default:
        printf("usage: phrases [-l] file\n");
        return EXIT_FAILURE;
    }
    
    /* close file */
    fclose(fp);
    return EXIT_SUCCESS;
}