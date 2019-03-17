#ifndef _HELPER_H
#define _HELPER_H

#define SIZE 44

struct rec {
    int freq;
    char word[SIZE];
};

int get_file_size(char *filename);
int compare_freq(const void *rec1, const void *rec2);
int divide_even(int * child_sizes, int N, int file_size);
void merge(int pipe_fd[][2],int N, FILE *outfp);

#endif /* _HELPER_H */
