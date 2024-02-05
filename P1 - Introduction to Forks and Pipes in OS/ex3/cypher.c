#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>


#define READ_END 0
#define WRITE_END 1
#define LINE_SIZE 8192
#define MAX_WORD_SIZE 128

char **words1;
char **words2;

int dictionarySize = 0;

char *replace(char *string, char *substr, char *replace) {
    int i = 0, j = 0, flag = 0, start = 0;
    char *output = (char *) malloc(sizeof(char) * 100);

    while (string[i] != '\0') {
        if (string[i] == substr[j]) {
            if (!flag)
                start = i;
            j++;
            if (substr[j] == '\0')
                break;
            flag = 1;
        } else {
            flag = start = j = 0;
        }
        i++;
    }
    if (substr[j] == '\0' && flag) {
        for (i = 0; i < start; i++)
            output[i] = string[i];

        for (j = 0; j < strlen(replace); j++) {
            output[i] = replace[j];
            i++;
        }
        for (j = start + strlen(substr); j < strlen(string) + strlen(replace) - strlen(substr); j++) {
            output[i] = string[j];
            i++;
        }
        output[i] = '\0';
        return output;
    } else {
    }
    return " ";
}

void createDictionary() {
    FILE *fp;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int line_counter = 0;

    fp = fopen("cypher.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    while ((read = getline(&line, &len, fp)) != -1) {
        line_counter++;
    }
    fclose(fp);

    words1 = (char **) malloc(line_counter * sizeof(char *));
    words2 = (char **) malloc(line_counter * sizeof(char *));

    char *token = (char *) malloc(LINE_SIZE * sizeof(char));
    fp = fopen("cypher.txt", "r");
    if (fp == NULL)
        exit(EXIT_FAILURE);
    int line_index = 0;

    for (int j = 0; j < line_counter; ++j) {
        words1[j] = (char *) malloc(sizeof(char));
        words2[j] = (char *) malloc(sizeof(char));
    }
    while ((read = getline(&line, &len, fp)) != -1) {
        token = strtok(line, " \n\r\t\0");
        words1[line_index] = realloc(words1[line_index], sizeof(char) * (strlen(token)));
        strcpy(words1[line_index], token);
        token = strtok(NULL, " \n\r\t\0");
        words2[line_index] = realloc(words2[line_index], sizeof(char) * (strlen(token)));
        strcpy(words2[line_index], token);

        dictionarySize++;
        line_index++;

    }
    fclose(fp);
//    if (line) free(line);
//    if (token) free(token);
}

int main(int argc, char *argv[]) {

    if (argc != 1) {
        printf("Usage: cypher < \"input filename\" [> \"output filename\"] \n");
        return EXIT_FAILURE;
    }

    int nbytes, pipePC[2];
    int pipeCP[2];
    pid_t pid;
    if (pipe(pipePC) < 0) { //parent -> child
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    if (pipe(pipeCP) < 0) { // child -> parent
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    if ((pid = fork()) < 0) {
        perror("fork error");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
/* parent */

        /* parent writing to child*/
        close(pipePC[READ_END]);

        char *lineBuffer = (char*) malloc(sizeof(char) * LINE_SIZE);
        size_t size = LINE_SIZE;
        long characters;
        long totalCharacters = 0;
        while ((characters = getline(&lineBuffer, &size, stdin)) != -1) {
            totalCharacters += characters;
            if ((nbytes = write(pipePC[WRITE_END], lineBuffer, characters)) < 0) {
                fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
            }

        }
        if ((nbytes = write(pipePC[WRITE_END], "\0", 1)) < 0) {
            fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
        }

        close(pipePC[WRITE_END]);
        if (waitpid(pid, NULL, 0) < 0) {
            fprintf(stderr, "Cannot wait for child: %s\n", strerror(errno));
        }

        /* parent reading from child */
        close(pipeCP[WRITE_END]);

        char *concatParent;
        char *parentBuffer = (char *) malloc(sizeof(char) * LINE_SIZE);
        memset(parentBuffer, 0, LINE_SIZE);

        int receivingBytes = 1;
        int totalReceivingBytes = 0;
        int counter = 1;
        while (receivingBytes > 0) {
            receivingBytes = read(pipeCP[READ_END], parentBuffer, LINE_SIZE);
            totalReceivingBytes += receivingBytes;
            if (receivingBytes < 0) {
                fprintf(stderr, "Unable to read from pipe: %s\n", strerror(errno));
            } else if (receivingBytes > 0) {
                if (counter++ == 1) {
                    concatParent = (char *) malloc(receivingBytes);
                    memcpy(concatParent, parentBuffer, receivingBytes);
                } else {
                    concatParent = realloc(concatParent, totalReceivingBytes);
                    memcpy(concatParent + totalReceivingBytes - receivingBytes, parentBuffer, receivingBytes);
                }
            }
            else {
                concatParent = realloc(concatParent, totalReceivingBytes + 1);
                concatParent[totalReceivingBytes] = '\0';
            }
        }
        write(STDOUT_FILENO, concatParent, totalReceivingBytes);
        close(pipeCP[READ_END]);
        exit(EXIT_SUCCESS);
    } else {
/* child */
        /* child reading from parent */
        char *concat;
        char *childBuffer = (char *) malloc(sizeof(char) * LINE_SIZE);
        memset(childBuffer, 0, LINE_SIZE);
        close(pipePC[WRITE_END]);

        nbytes = 1;
        int totalBytes = 0;

        int counter = 1;
        while (nbytes > 0) {
            nbytes = read(pipePC[READ_END], childBuffer, LINE_SIZE);
            totalBytes += nbytes;
            if ((nbytes) < 0) {
                fprintf(stderr, "Unable to read from pipe: %s\n", strerror(errno));
            } else if (nbytes > 0) {
                if (counter++ == 1) {
                    concat = (char *) malloc(nbytes);
                    memcpy(concat, childBuffer, nbytes);
                } else {
                    concat = realloc(concat, totalBytes);
                    memcpy(concat + totalBytes - nbytes, childBuffer, nbytes);
                }
            } else {
                concat = realloc(concat, totalBytes + 1); //? concat = ?
                concat[totalBytes] = '\0';
            }
        }

//        if(childBuffer) free(childBuffer);
        createDictionary();
        char *res;
        unsigned long resBytes = 0;
        char *replaced = (char *) malloc(sizeof(char) * MAX_WORD_SIZE);
        char *token = (char *) malloc(sizeof(char) * MAX_WORD_SIZE);
        token = strtok(concat, " ");
        for (int i = 0; i < dictionarySize; i++) {
            if (i > 0) token = replaced;
            if (strstr(token, words1[i]) != NULL) {
                replaced = replace(token, words1[i], words2[i]);
            } else if (strstr(token, words2[i]) != NULL) {
                replaced = replace(token, words2[i], words1[i]);
            } else {
                replaced = token;
            }
        }
        resBytes += strlen(replaced);

        res = (char *) malloc(resBytes);
        memcpy(res, replaced, strlen(replaced));

        while ((token = strtok(NULL, " ")) != 0) {
            res = realloc(res, resBytes + 1);
            res[resBytes++] = ' ';
            for (int i = 0; i < dictionarySize; i++) {
                if (i > 0) token = replaced;
                if (strstr(token, words1[i]) != NULL) {
                    replaced = replace(token, words1[i], words2[i]);
                } else if (strstr(token, words2[i]) != NULL) {
                    replaced = replace(token, words2[i], words1[i]);
                } else {
                    replaced = token;
                }
            }
            resBytes += strlen(replaced);

            res = realloc(res, resBytes);
            memcpy(res + resBytes - strlen(replaced), replaced, strlen(replaced));
        }
        res = realloc(res, resBytes + 1);
        res[resBytes] = '\0';


//        if(concat)free(concat);
//        if(res)free(res);
//        if(words1) free(words1);
//        if(words2) free(words2);
        close(pipePC[READ_END]);

        /*child writing to parent */
        close(pipeCP[READ_END]);

        int sendingBytes = 1;
        int totalSendingBytes = 0;
        sendingBytes = write(pipeCP[WRITE_END], res, strlen(res));
        totalSendingBytes += sendingBytes;
        if ((sendingBytes) < 0) {
            fprintf(stderr, "Unable to write to pipe: %s\n", strerror(errno));
        }

        close(pipeCP[WRITE_END]);


        exit(EXIT_SUCCESS);
    }
}


