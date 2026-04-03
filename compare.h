#ifndef COMPARE_H
#define COMPARE_H
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct WordNode {
    char *word;
    int count;
    double frequency;
    struct WordNode *next;
} WordNode;

typedef struct FileData{
    char *path;
    WordNode *wordList;
    int totalWords;
} FileData;

typedef struct CompareResult {
    char *file1;
    char *file2;
    double similarity;
    int totalWordsFile;
} CompareResult;

#endif