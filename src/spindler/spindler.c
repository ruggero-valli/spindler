#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <rinterpolate.h>
#include "csvreader.h"

#define SPINDLER_BUFSIZE 128

/*error codes*/
enum {
    SPINDLER_NO_ERROR,
    SPINDLER_ALLOC_FAILED,
    SPINDLER_READ_FILE_FAILED,
    SPINDLER_INIT_FAILED,
    SPINDLER_DIR_NOT_FOUND,
};

/**
 * @brief Contains one interpolation table and its metadata.
 * 
 * It is initialized by spindler_init_interpolator.
 * It is freed by spindler_free_interpolator.
 */
struct spindler_interpolator_t {
    rinterpolate_float_t* table;
    char** parameter_names;
    unsigned int number_of_interpolation_parameters;
    unsigned int number_of_interpolation_points;    
    struct rinterpolate_data_t* rinterpolate_data;
};

/**
 * @brief Frees a splindler_interpolator
 * Doesn't free the struct itself, the caller needs to call
 * free(spindler_interpolator).
 * @param spindler_interpolator the struct to free
 */
void spindler_free_interpolator(struct spindler_interpolator_t* interp){
    free(interp->table);
    free2DArray((void **)(interp->parameter_names), interp->number_of_interpolation_parameters);
    rinterpolate_free_data(interp->rinterpolate_data);
    free(interp->rinterpolate_data);
}

/**
 * @brief Contains the a splindler_interpolator for each of the interpolated
 * variable and the model name.
 * 
 * It is initialized by spindler_init.
 * It is freed by spindler_free_data.
 */
struct spindler_data_t {
    char model_name[SPINDLER_BUFSIZE];
    struct spindler_interpolator_t* edot_interp;
    struct spindler_interpolator_t* adota_interp;
    struct spindler_interpolator_t* qdot_interp;
};

/**
 * @brief Frees a splindler_data
 * Doesn't free the struct itself, the caller needs to call
 * free(splindler_data).
 * @param spindler_data the struct to free
 */
void spindler_free_data(struct spindler_data_t* spindler_data){
    if (spindler_data->edot_interp != NULL){
        spindler_free_interpolator(spindler_data->edot_interp);
        free(spindler_data->edot_interp);
    }
    if (spindler_data->adota_interp != NULL){
        spindler_free_interpolator(spindler_data->adota_interp);
        free(spindler_data->adota_interp);
    }
    if (spindler_data->qdot_interp != NULL){
        spindler_free_interpolator(spindler_data->qdot_interp);
        free(spindler_data->qdot_interp);
    }
}

/**
 * @brief Initialize a spindler_interpolator with the content of an interpolation
 * table read from a file.
 * 
 * @param filename the location of the interpolation table
 * @param interp the struct to initialize. It needs to be
 *  allocated by the caller
 * @return error code
 */
int spindler_init_interpolator(char* filename, struct spindler_interpolator_t* interp){
    /* Read csv */
    double **csvTable = NULL;
    int NColumns;
    int NRows;
    bool readHeader = true;
    char **header = NULL;
    bool isWhitespaceSeparated = false;
    char separator = ',';
    if (read_csv(filename, &csvTable, &NColumns, &NRows, readHeader, &header,
        isWhitespaceSeparated, separator) != 0){
        fprintf(stderr, "Problem reading csv\n");
        return SPINDLER_READ_FILE_FAILED;
    }

    /* Allocate rinterpolate_data */
    interp->rinterpolate_data = NULL;
    rinterpolate_alloc_dataspace(&(interp->rinterpolate_data));
    if (interp->rinterpolate_data == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        free2DArray((void **)csvTable, NRows);
        free2DArray((void **)header, NColumns);
        return SPINDLER_ALLOC_FAILED;
    }

    /* Assign the values */
    interp->parameter_names = header;
    interp->number_of_interpolation_parameters = NColumns-1; // The last column is data
    interp->number_of_interpolation_points = NRows;

    /* Allocate the table */
    interp->table = NULL;
    interp->table = malloc(NColumns*NRows*sizeof(rinterpolate_float_t));
    if (interp->table == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        free2DArray((void **)csvTable, NRows);
        free2DArray((void **)header, NColumns);
        rinterpolate_free_data(interp->rinterpolate_data);
        free(interp->rinterpolate_data);
        return SPINDLER_ALLOC_FAILED;
    }

    /* Fill the table */
    for (int i=0; i<NRows; i++){
        for (int j=0; j<NColumns; j++){
            interp->table[i*NColumns+j] = csvTable[i][j];
        }
    }
    free2DArray((void **)csvTable, NRows);
    return SPINDLER_NO_ERROR;
}

/**
 * @brief Initialize a spindler_data struct with the interpolation tables of 
 *  a given model.
 * 
 * @param model_name the name of the model. It has to correspond to the name of 
 *  an existing subdirectory of `tables/`.
 * @param spindler_data the struct to initialize. It has to be allocated by
 *  the caller
 * @return error code
 */
int spindler_init(char* model_name, struct spindler_data_t* spindler_data){
    strcpy(spindler_data->model_name, model_name);

    /* Check existence of the directory */
    struct stat st;
    char dir_path[SPINDLER_BUFSIZE], filename[SPINDLER_BUFSIZE];
    snprintf(dir_path, SPINDLER_BUFSIZE, "tables/%s", model_name);
    if (stat(dir_path, &st) == -1) {
        fprintf(stderr, "Directory does not exist: %s\n", dir_path);
        return SPINDLER_DIR_NOT_FOUND;
    }

    /* edot */
    spindler_data->edot_interp = NULL;
    snprintf(filename, SPINDLER_BUFSIZE, "tables/%s/edot.csv", model_name);
    /* check if file exists */
    if (access(filename, F_OK) == 0) {
        /* Allocate interpolator */
        spindler_data->edot_interp = calloc(1, sizeof(struct spindler_interpolator_t));
        if (spindler_data->edot_interp == NULL){
            fprintf(stderr, "Memory allocation failed\n");
            return SPINDLER_ALLOC_FAILED;
        }
        /* Initialize interpolator */
        if (spindler_init_interpolator(filename, spindler_data->edot_interp) != SPINDLER_NO_ERROR){
            fprintf(stderr, "Failed initializing edot interpolator\n");
            return SPINDLER_INIT_FAILED;
        }
    }

    /* qdot */
    spindler_data->qdot_interp = NULL;
    snprintf(filename, SPINDLER_BUFSIZE, "tables/%s/qdot.csv", model_name);
    /* check if file exists */
    if (access(filename, F_OK) == 0) {
        /* Allocate interpolator */
        spindler_data->qdot_interp = calloc(1, sizeof(struct spindler_interpolator_t));
        if (spindler_data->qdot_interp == NULL){
            fprintf(stderr, "Memory allocation failed\n");
            return SPINDLER_ALLOC_FAILED;
        }
        /* Initialize interpolator */
        if (spindler_init_interpolator(filename, spindler_data->qdot_interp) != SPINDLER_NO_ERROR){
            fprintf(stderr, "Failed initializing qdot interpolator\n");
            return SPINDLER_INIT_FAILED;
        }
    }

    /* adota */
    spindler_data->adota_interp = NULL;
    snprintf(filename, SPINDLER_BUFSIZE, "tables/%s/adota.csv", model_name);
    /* check if file exists */
    if (access(filename, F_OK) == 0) {
        /* Allocate interpolator */
        spindler_data->adota_interp = calloc(1, sizeof(struct spindler_interpolator_t));
        if (spindler_data->adota_interp == NULL){
            fprintf(stderr, "Memory allocation failed\n");
            return SPINDLER_ALLOC_FAILED;
        }
        /* Initialize interpolator */
        if (spindler_init_interpolator(filename, spindler_data->adota_interp) != SPINDLER_NO_ERROR){
            fprintf(stderr, "Failed initializing adota interpolator\n");
            return SPINDLER_INIT_FAILED;
        }
    }
    return SPINDLER_NO_ERROR;
}

/**
 * @brief Compute the interpolation
 * 
 * @param q mass ratio
 * @param e eccentricity
 * @param interp interpolator struct
 * @return result of the interpolation 
 */
double spindler_interpolate(double q, double e, struct spindler_interpolator_t* interp){
    /* The table is missing */
    if (interp == NULL){
        return 0;
    }

    rinterpolate_float_t *table = interp->table;
    rinterpolate_counter_t N = interp->number_of_interpolation_parameters;
    rinterpolate_counter_t D = 1;
    rinterpolate_counter_t L = interp->number_of_interpolation_points;
    rinterpolate_float_t *x = calloc(N, sizeof(rinterpolate_float_t));
    if (x == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        return SPINDLER_ALLOC_FAILED;
    }
    rinterpolate_float_t r[1];
    int usecache = 0;
    
    /* Fill the x array with the coordinates of the point to interpolate */
    for (int i=0; i<N; i++){
        if (strcmp(interp->parameter_names[i], "e") == 0){
            x[i] = e;
        } else if (strcmp(interp->parameter_names[i], "q") == 0){
            x[i] = q;
        } else {
            fprintf(stderr, "Warning: parameter %s not recognized\n", interp->parameter_names[i]);
        }
    }

    /* Interpolate */
    rinterpolate(table,
                interp->rinterpolate_data,
                N,
                D,
                L,
                x,
                r,
                usecache);

    /* free memory and return */
    free(x);
    return r[0];
}

double spindler_get_De(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* interp = (spindler_data->edot_interp);
    double De;
    if (e != 0){
        De = spindler_interpolate(q, e, interp)/e;
    } else {
        De = 0;
    }

    return De;
}

double spindler_get_Dq(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* interp = (spindler_data->qdot_interp);
    double Dq = spindler_interpolate(q, e, interp)/q;
    return Dq;
}

double spindler_get_Da(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* interp = (spindler_data->adota_interp);
    double Da = spindler_interpolate(q, e, interp);
    return Da;
}

double spindler_get_DE(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* adota_interp = (spindler_data->adota_interp);
    struct spindler_interpolator_t* qdot_interp = (spindler_data->qdot_interp);
    double Da = spindler_get_Da(q,e, spindler_data);
    double Dq = spindler_get_Dq(q,e, spindler_data);

    /*
    By differentiating the orbital energy
    E = - (GM \mu)/(2a),
    where \mu=M q/(1+q)^2 is the reduced mass:
    */
    double DE = 2.0 - Da + (1-q)/(q*q + q)*Dq;
    return DE;
}

double spindler_get_DJ(double q, double e, struct spindler_data_t* spindler_data){
    double Da = spindler_get_Da(q,e, spindler_data);
    double Dq = spindler_get_Dq(q,e, spindler_data);
    double De = spindler_get_De(q,e, spindler_data);

    /*
    By differentiating the orbital angular momentum 
    J = \mu \sqrt{GMa(1-e^2)}
    where \mu=M q/(1+q)^2 is the reduced mass:
    */
    double DJ = 3/2 + 1/2*Da + (1-q)/(q*q + q)*Dq - e/(1-e*e)*De;
    return DJ;
}


int main(){
    char* model_name = "Zrake21";
    struct spindler_data_t* spindler_data = calloc(1, sizeof(struct spindler_data_t));
    if (spindler_data == NULL){
        fprintf(stderr, "Memory allocation failed\n");
        return SPINDLER_ALLOC_FAILED;
    }
    spindler_init(model_name, spindler_data);

    double q=0.5, e=0.5;
    double Dq, De, Da;
    Dq = spindler_get_Dq(q, e, spindler_data);
    De = spindler_get_De(q, e, spindler_data);
    Da = spindler_get_Da(q, e, spindler_data);

    printf("Dq: %lf, De: %lf, Da: %lf\n", Dq, De, Da);

    spindler_free_data(spindler_data);
    free(spindler_data);
    return 0;

    // /* Number of parameters */
    // const rinterpolate_counter_t N = 2;

    //     /* Number of data */
    // const rinterpolate_counter_t D = 1;

    // /* length of each line (in doubles) i.e. N+D */
    // const rinterpolate_counter_t ND = N + D;

    // /* make rinterpolate data (for cache etc.) */
    // struct rinterpolate_data_t * rinterpolate_data = NULL;
    // rinterpolate_counter_t status = rinterpolate_alloc_dataspace(&rinterpolate_data);

    // char *filename = "tables/Siwek23/edot.csv";
    // double **csvTable = NULL;
    // int NColumns;
    // int NRows;
    // bool readHeader = true;
    // char **header = NULL;
    // bool isWhitespaceSeparated = false;
    // char separator = ',';
    // if (read_csv(filename, &csvTable, &NColumns, &NRows, readHeader, &header,
    //     isWhitespaceSeparated, separator) != 0){
    //     fprintf(stderr, "Problem reading csv");
    //     return 1;
    // }

    // /* total number of lines */
    // rinterpolate_counter_t L = NRows;

    // /* data table : it is up to you to set the data in here*/
    // rinterpolate_float_t *table = malloc(ND*L*sizeof(rinterpolate_float_t));

    // for (int i=0; i<NRows; i++){
    //     for (int j=0; j<NColumns; j++){
    //         table[i*ND+j] = csvTable[i][j];
    //         //printf("%lf ", csvTable[i][j]);
    //     }
    //     //printf("\n");
    // }

    // /*
    //     * Arrays for the interpolation location and
    //     * interpolated data. You need to set
    //     * the interpolation location, x.
    //     */
    // rinterpolate_float_t x[N],r[D];

    // /* choose whether to cache (0=no, 1=yes) */
    // int usecache = 0;
    // x[0] = 0.5; x[1] = 0.1;

    // /* do the interpolation */
    // int steps = 3000000;
    // for (int i=0; i<steps; i++){
    //     //x[0] = 0.1+i/(2.*steps);
    //     //x[1] = i/(2.*steps);
    //     //rinterpolate_free_data(rinterpolate_data);
    //     //free(rinterpolate_data);
    //     //rinterpolate_data = NULL;
    //     rinterpolate(table,
    //                 rinterpolate_data,
    //                 N,
    //                 D,
    //                 L,
    //                 x,
    //                 r,
    //                 usecache);
    //     //printf("Result: %lf %lf %lf\n", x[0], x[1], r[0]);
    // }


    // /* the array r contains the result */

    // /* ... rest of code ... */


    // /* free memory on exit */
    // rinterpolate_free_data(rinterpolate_data);
    // free(rinterpolate_data);
    // free2DArray((void**)header, NColumns);
}