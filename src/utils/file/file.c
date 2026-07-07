#include "file.h"

#include <stdio.h>
#include <stdlib.h>

#include "utils/base-types.h"

char* aqua_file_to_string(const char* file_path) {
    FILE* file = fopen(file_path, "r");
    if (!file) {
        fprintf(stderr, "[ERROR] [Aqua] [File] Failed to open file!\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    if (file_length < 0) {
    	fprintf(stderr, "[ERROR] [Aqua] [File] Failed to read file!\n");
     	fclose(file);
      	return NULL;
    }
    fseek(file, 0, SEEK_SET);

    char* string = (char*) malloc((usize) file_length + 1); // +1 for '\0'
    if (!string) {
        fprintf(stderr, "[ERROR] [Aqua] [File] Failed to allocate memory for string!\n");
        fclose(file);
        return NULL;
    }

    usize bytes_read = fread(string, 1, (usize) file_length, file);
    if (bytes_read != (usize) file_length) {
        fprintf(stderr, "[ERROR] [Aqua] [File] Failed to read entire file!\n");
        free(string);
        fclose(file);
        return NULL;
    }
    string[bytes_read] = '\0';

    fclose(file);

    return string;
}

void aqua_file_free_string(char* string) {
	free(string);
}
