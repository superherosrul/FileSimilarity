#include "compare.h"// Define the suffix for text files
#include <ctype.h>

#define _POSIX_C_SOURCE 200809L
#define SUFFIX ".txt"// Function to create a new WordNode

// Function to check if a file has the specified suffix// Function to create a new WordNode
WordNode* createWordNode(char *word) {
    WordNode *node = (WordNode *)malloc(sizeof(WordNode));
    if(!node) return NULL;
    node->word = strdup(word);
    node->count = 1;
    node->frequency = 0.0;
    node->next = NULL;
    return node;
}
// Function to add a word to the linked list or update its count if it already exists
void addWord(WordNode **head, char *word) {
    WordNode *current = *head;
    while (current != NULL) {
        if (strcmp(current->word, word) == 0) {
            current->count++;
            return;
        }
        current = current->next;
    }
    WordNode *newNode = createWordNode(word);
    if (newNode) {
        newNode->next = *head;
        *head = newNode;
    }
}
// Function to check if a character is part of a word (alphanumeric or hyphen)
int checkwordchar(char c) {
    return (isalnum(c) || c == '-');
}

// Function to process a file and populate its word list and total word count
void processFile(FileData *fileData) {
    int fd = open(fileData->path, O_RDONLY);
    if (fd < 0) {
        perror(fileData->path);
        return;
    }
    char word[1024];
    char buffer[256];
    int index = 0;

    ssize_t bytesRead;
    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) {// Read the file in chunks and process each character
        for (int i = 0; i < bytesRead; i++) {// Read the file in chunks and process each character
            char c = tolower((unsigned char)buffer[i]);
            if (checkwordchar(c)) {// If the character is part of a word, add it to the current word buffer
                if (index < 1023) word[index++] = c;
            } else {// If the character is not part of a word and we have a word in the buffer, add it to the word list
                if (index > 0) {// If the character is not part of a word and we have a word in the buffer, add it to the word list
                    word[index] = '\0';
                    addWord(&fileData->wordList, word);
                    fileData->totalWords++;
                    index = 0;
                }
            }
        }
    }
    if (index > 0) {// If we have a word in the buffer after reading the file, add it to the word list
        word[index] = '\0';
        addWord(&fileData->wordList, word);
        fileData->totalWords++;
    }
    close(fd);

    WordNode *current = fileData->wordList;
    while (current != NULL) {// Calculate the frequency of each word in the file
        current->frequency = (double)current->count / fileData->totalWords;
        current = current->next;
    }
}

//Function to get the frequency of a word in a file's word list
double getfrequency(WordNode *list, char *word) {
    while (list) {// Traverse the linked list to find the word and return its frequency
        if (strcmp(list->word, word) == 0) return list->frequency;// Traverse the linked list to find the word and return its frequency
        list = list->next;
    }
    return 0.0;// If the word is not found in the list, return 0.0
}

// Function to compute the Jensen-Shannon Divergence between two files
double compute_jsd(FileData *file1, FileData *file2) {
    WordNode *w = file1->wordList;
    double kld1 = 0.0, kld2 = 0.0;

    while (w) {// Traverse the word list of the first file and calculate the Kullback-Leibler divergence for both files
        double p = w->frequency;// Get the frequency of the word in the second file
        double q = getfrequency(file2->wordList, w->word);
        double m = (p + q) / 2.0;// Calculate the average frequency of the word in both files
        kld1 += p * log2(p / m);// Calculate the contribution of the word to the Kullback-Leibler divergence for the first file
        w = w->next;
    }

    w = file2->wordList;
    while (w) {// Traverse the word list of the second file to account for words that are only in the second file
        double q = w->frequency;// Get the frequency of the word in the first file
        double p = getfrequency(file1->wordList, w->word);
        if(p == 0) {
            double m = q / 2.0;// If the word is only in the second file, calculate its contribution to the Kullback-Leibler divergence for the second file
            kld2 += q * log2(q / m);
        }else {
            double m = (p + q) / 2.0;// If the word is in both files, calculate its contribution to the Kullback-Leibler divergence for the second file
            kld2 += q * log2(q / m);
        }
        w = w->next;
    }
    return sqrt(0.5 * kld1 + 0.5 * kld2);

}
// Function to check if a file has the specified suffix
int has_suffix(char *name) {// Check if the filename ends with the specified suffix
    int len_filename = strlen(name);
    int len_suffix = strlen(SUFFIX);// Calculate the length of the suffix to compare with the end of the filename
    // Check if the filename ends with the specified suffix
    return len_filename >= len_suffix && strcmp(name + len_filename - len_suffix, SUFFIX) == 0;
}

void collect_files(char *path, FileData **files, int *file_count) {
    struct stat st;
    if (stat(path, &st) < 0) return;

    if (S_ISREG(st.st_mode)) {
        // Process the regular file
        if(has_suffix(path)) {
            files[*file_count] = (FileData *)malloc(sizeof(FileData));
            files[*file_count]->path = (char *)strdup(path);
            files[*file_count]->wordList = NULL;
            files[*file_count]->totalWords = 0;
            (*file_count)++;
        }
    }
    else if (S_ISDIR(st.st_mode)) {
        // Process the directory
        DIR *dir = opendir(path);
        if (dir == NULL) return;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue; // Skip current and parent directories

            char fullpath[1024];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);
            collect_files(fullpath, files, file_count);
        }
        closedir(dir);
    }

}

int compare_cmp(const void *a, const void *b) {
    CompareResult *resA = (CompareResult *)a;
    CompareResult *resB = (CompareResult *)b;
    if (resA->similarity > resB->similarity) return -1; // Sort in descending order of similarity
    if (resA->similarity < resB->similarity) return 1;
    return 0; // Sort in descending order of total words
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <file_or_directory> [<file_or_directory> ...]\n", argv[0]);
        return 1;
    }

    FileData *files[1024];
    int file_count = 0;

    for (int i = 1; i < argc; i++) {
        collect_files(argv[i], files, &file_count);
    }

    if (file_count < 2) {
        fprintf(stderr, "At least two files are required for comparison.\n");
        return 1;
    }

    for (int i = 0; i < file_count; i++) {
        processFile(files[i]);
    }

    int totalpairs = file_count * (file_count - 1) / 2;
    CompareResult *results = malloc(totalpairs * sizeof(CompareResult));

    int index = 0;
    for (int i = 0; i < file_count; i++) {
        for (int j = i + 1; j < file_count; j++) {
            results[index].file1 = files[i]->path;
            results[index].file2 = files[j]->path;
            results[index].totalWordsFile = files[i]->totalWords + files[j]->totalWords;
            results[index].similarity = compute_jsd(files[i], files[j]);
            index++;
        }
    }

    // Sort results in descending order of similarity
    qsort(results, totalpairs, sizeof(CompareResult), compare_cmp);

    // Print results
    for (int i = 0; i < totalpairs; i++) {
        printf("Similarity between %s and %s: %.4f\n", results[i].file1, results[i].file2, results[i].similarity);
    }

    // Free allocated memory
    for (int i = 0; i < file_count; i++) {
        WordNode *current = files[i]->wordList;
        while (current) {
            WordNode *temp = current;
            current = current->next;
            free(temp->word);
            free(temp);
        }
        free(files[i]->path);
        free(files[i]);
    }
    free(results);

    return 0;
}