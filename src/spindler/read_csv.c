#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "read_csv.h"

/**
 * @brief Frees memory allocated for a 2D array.
 * 
 * @param array Pointer to the 2D array.
 * @param len Number of rows in the array.
 */
void free2DArray(void **array, int len){
    if (array == NULL){
        return;
    }
    for (int i=0; i<len; i++){
        free(array[i]);
    }
    free(array);
}

/**
 * @brief Parses a line of text into tokens.
 * 
 * @param line The line of text to parse.
 * @param tokens Pointer to an array to store the parsed tokens. Needs to be freed with free2DArray after use.
 * @param count Pointer to an integer to store the number of tokens.
 * @param isWhitespaceSeparated If true, tokens are separated by whitespace; otherwise, tokens are separated by the specified separator.
 * @param separator The separator character used to separate tokens (ignored if isWhitespaceSeparated is true).
 * @return int Returns 0 on success, non-zero on failure.
 */
int parseLine(const char *line, char ***tokens, int *count, bool isWhitespaceSeparated, char separator) {
    char *token;
    int allocated = 10;
    char *linecopy = strdup(line);
    if (linecopy == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    *count = 0;
    *tokens = malloc(allocated * sizeof(char *));
    if (*tokens == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        free(linecopy);
        return 1;
    }

    if (isWhitespaceSeparated) {
        token = strtok(linecopy, " \t\n");
    } else {
        token = strtok(linecopy, &separator);
    }
    while (token != NULL) {
        if (*count >= allocated) {
            allocated = allocated*2;
            *tokens = realloc(*tokens, allocated * sizeof(char *));
            if (*tokens == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                free(linecopy);
                free2DArray((void **)*tokens, *count);
                return 1;
            }
        }
        (*tokens)[*count] = strdup(token);
        if ((*tokens)[*count] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(linecopy);
            free2DArray((void **)*tokens, *count);
            return 1;
        }
        (*count)++;
        if (isWhitespaceSeparated) {
            token = strtok(NULL, " \t\n");
        } else {
            token = strtok(NULL, &separator);
        }
    }
    free(linecopy);
    return 0;
}

/**
 * @brief Parses the header of a CSV file.
 * 
 * @param file Pointer to the CSV file.
 * @param NColumns Pointer to an integer to store the number of columns in the header.
 * @param header Pointer to an array to store the header data.
 * @param isWhitespaceSeparated If true, data in the CSV is separated by whitespace; otherwise, data is separated by the specified separator.
 * @param separator The separator character used in the CSV file (ignored if isWhitespaceSeparated is true).
 * @return int Returns 0 on success, non-zero on failure.
 */
int parseHeader(FILE *file, int *NColumns, char ***header, bool isWhitespaceSeparated, char separator) {
    rewind(file);
    char* line = NULL;
    size_t size = 0;
    if (getline(&line, &size, file) <= 0) {
            fprintf(stderr, "Error in reading the line\n");
            return 2;
    }
    if (parseLine(line, header, NColumns, isWhitespaceSeparated, separator) != 0) {
        fprintf(stderr, "Error in parsing the line\n");
        free(line);
        return 3;
    }
    free(line);
    return 0;
}

/**
 * @brief Reads data from a CSV file.
 * 
 * @param file Pointer to the CSV file.
 * @param NColumns The number of columns in the CSV.
 * @param NRows Pointer to an integer to store the number of rows read.
 * @param table Pointer to an array to store the data read from the CSV.
 * @param isWhitespaceSeparated If true, data in the CSV is separated by whitespace; otherwise, data is separated by the specified separator.
 * @param separator The separator character used in the CSV file (ignored if isWhitespaceSeparated is true).
 * @return int Returns 0 on success, non-zero on failure.
 */
int readData(FILE *file, int NColumns, int *NRows, double ***table, bool isWhitespaceSeparated, char separator) {
    char* line = NULL;
    int allocatedRows = 10;
    size_t size = 0;
    if (getline(&line, &size, file) <= 0) {
            fprintf(stderr, "Error in reading the line\n");
            return 2;
    }
    *NRows = 0;
    *table = malloc(allocatedRows * sizeof(double *));
    while (getline(&line, &size, file) > 0) {
        // Resize table if needed
        if (*NRows >= allocatedRows) {
            allocatedRows = allocatedRows*2;
            *table = realloc(*table, allocatedRows * sizeof(double *));
            if (*table == NULL) {
                fprintf(stderr, "Memory allocation failed\n");
                free(line);
                return 1;
            }
        }
        (*table)[*NRows] = (double *)malloc(NColumns * sizeof(double));
        if ((*table)[*NRows] == NULL) {
            fprintf(stderr, "Memory allocation failed\n");
            free(line);
            free2DArray((void **)*table, *NRows);
            return 1;
        }
        char **tokens = NULL;
        int count;
        if (parseLine(line, &tokens, &count, isWhitespaceSeparated, separator) != 0) {
            fprintf(stderr, "Error in parsing the line\n");
            free(line);
            free2DArray((void **)*table, *NRows);
            return 3;
        }
        if (count != NColumns) {
            fprintf(stderr, "Row %d does not have the expected number of columns\n", *NRows + 1);
            return 4;
        }
        for (int i = 0; i < NColumns; i++) {
            (*table)[*NRows][i] = atof(tokens[i]);
        }
        free2DArray((void **)tokens, count);
        (*NRows)++;
    }
    free(line);
    return 0;
}


/**
 * @brief Reads a CSV file and stores its content in a 2D array.
 * 
 * @param filename The name of the CSV file to read.
 * @param table Pointer to the 2D array where the CSV data will be stored.
 * @param NColumns Pointer to an integer to store the number of columns in the CSV.
 * @param NRows Pointer to an integer to store the number of rows in the CSV.
 * @param readHeader If true, the first row of the CSV will be considered as a header.
 * @param header Pointer to a pointer to store the header data (if readHeader is true).
 * @param isWhitespaceSeparated If true, data in the CSV is separated by whitespace.
 * @param separator The separator character used in the CSV file (ignored if isWhitespaceSeparated is true).
 * @return int Returns 0 on success, non-zero on failure.
 */
int read_csv(const char *filename, double ***table, int *NColumns, int *NRows, bool readHeader, char ***header, bool isWhitespaceSeparated, char separator) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file %s\n", filename);
        return 1;
    }

    // Read header
    if (readHeader) {
        if (parseHeader(file, NColumns, header, isWhitespaceSeparated, separator) != 0) {
            fprintf(stderr, "Error reading the header.\n");
            fclose(file);
            return 2;
        }
    }

    // Read data
    *table = NULL;
    if (readData(file, *NColumns, NRows, table, isWhitespaceSeparated, separator) != 0) {
        fprintf(stderr, "Error reading the data.\n");
        fclose(file);
        return 3;
    }

    fclose(file);
    return 0;
}