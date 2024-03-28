#ifndef READ_CSV_H
#define READ_CSV_H

#include <stdio.h>
#include <stdbool.h>

/* error codes */
enum {
    CSVREADER_NO_ERROR,
    CSVREADER_ALLOC_FAILED,
    CSVREADER_READ_FILE_FILED,
};

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
int read_csv(const char *filename, double ***table, int *NColumns, int *NRows, bool readHeader, char ***header, bool isWhitespaceSeparated, char separator);

/**
 * @brief Frees memory allocated for a 2D array.
 * 
 * @param array Pointer to the 2D array.
 * @param len Number of rows in the array.
 */
void free2DArray(void **array, int len);

#endif /* READ_CSV_H */
