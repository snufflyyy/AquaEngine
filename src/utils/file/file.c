#include "file.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <aqua/types.h>

char* aqua_file_to_string(const char* file_path) {
    // binary mode: in text mode Windows translates line endings, so fread returns fewer bytes than ftell reported
    FILE* file = fopen(file_path, "rb");
    if (!file) {
        fprintf(stderr, "[ERROR] [Aqua] [File] Failed to open '%s': %s\n", file_path, strerror(errno));
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_length = ftell(file);
    if (file_length < 0) {
    	fprintf(stderr, "[ERROR] [Aqua] [File] Failed to get the size of '%s': %s\n", file_path, strerror(errno));
     	fclose(file);
      	return NULL;
    }
    fseek(file, 0, SEEK_SET);

    char* string = (char*) malloc((usize) file_length + 1); // +1 for '\0'
    if (!string) {
        fprintf(stderr, "[ERROR] [Aqua] [File] Failed to allocate memory for '%s'!\n", file_path);
        fclose(file);
        return NULL;
    }

    usize bytes_read = fread(string, 1, (usize) file_length, file);
    if (bytes_read != (usize) file_length) {
        fprintf(stderr, "[ERROR] [Aqua] [File] Failed to read all of '%s'!\n", file_path);
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
