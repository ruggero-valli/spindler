#include <rinterpolate.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include "csvreader.h"

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
    free2DArray(interp->parameter_names, interp->number_of_interpolation_parameters);
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
    char model_name[128];
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
    spindler_free_interpolator(spindler_data->edot_interp);
    free(spindler_data->edot_interp);
    spindler_free_interpolator(spindler_data->adota_interp);
    free(spindler_data->adota_interp);
    spindler_free_interpolator(spindler_data->qdot_interp);
    free(spindler_data->qdot_interp);
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
        fprintf(stderr, "Problem reading csv");
        return SPINDLER_READ_FILE_FAILED;
    }

    /* Allocate rinterpolate_data */
    interp->rinterpolate_data = NULL;
    rinterpolate_alloc_dataspace(&(interp->rinterpolate_data));
    if (interp->rinterpolate_data == NULL){
        fprintf(stderr, "Memory allocation failed");
        free2DArray(csvTable, NRows);
        free2DArray(header, NColumns);
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
        fprintf(stderr, "Memory allocation failed");
        free2DArray(csvTable, NRows);
        free2DArray(header, NColumns);
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
    free2DArray(csvTable, NRows);
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
    char dir_path[128], filename[128];
    sprintf(dir_path, "tables/%s", model_name);
    if (stat(dir_path, &st) == -1) {
        fprintf(stderr, "Directory does not exist: %s\n", dir_path);
        return SPINDLER_DIR_NOT_FOUND;
    }

    /* edot */
    spindler_data->edot_interp = NULL;
    sprintf(filename, "%s/edot.csv", dir_path);
    /* check if file exists */
    if (access(filename, F_OK) == 0) {
        /* Allocate interpolator */
        spindler_data->edot_interp = calloc(1, sizeof(struct spindler_interpolator_t));
        if (spindler_data->edot_interp == NULL){
            fprintf(stderr, "Memory allocation failed");
            return SPINDLER_ALLOC_FAILED;
        }
        /* Initialize interpolator */
        if (spindler_init_interpolator(filename, spindler_data->edot_interp) != SPINDLER_NO_ERROR){
            fprintf(stderr, "Failed initializing edot interpolator");
            return SPINDLER_INIT_FAILED;
        }
    }

    /* qdot */
    spindler_data->qdot_interp = NULL;
    sprintf(filename, "%s/qdot.csv", dir_path);
    /* check if file exists */
    if (access(filename, F_OK) == 0) {
        /* Allocate interpolator */
        spindler_data->qdot_interp = calloc(1, sizeof(struct spindler_interpolator_t));
        if (spindler_data->qdot_interp == NULL){
            fprintf(stderr, "Memory allocation failed");
            return SPINDLER_ALLOC_FAILED;
        }
        /* Initialize interpolator */
        if (spindler_init_interpolator(filename, spindler_data->qdot_interp) != SPINDLER_NO_ERROR){
            fprintf(stderr, "Failed initializing qdot interpolator");
            return SPINDLER_INIT_FAILED;
        }
    }

    /* adota */
    spindler_data->adota_interp = NULL;
    sprintf(filename, "%s/adota.csv", dir_path);
    /* check if file exists */
    if (access(filename, F_OK) == 0) {
        /* Allocate interpolator */
        spindler_data->adota_interp = calloc(1, sizeof(struct spindler_interpolator_t));
        if (spindler_data->adota_interp == NULL){
            fprintf(stderr, "Memory allocation failed");
            return SPINDLER_ALLOC_FAILED;
        }
        /* Initialize interpolator */
        if (spindler_init_interpolator(filename, spindler_data->adota_interp) != SPINDLER_NO_ERROR){
            fprintf(stderr, "Failed initializing adota interpolator");
            return SPINDLER_INIT_FAILED;
        }
    }
}

/**
 * @brief Compute the interpolation
 * 
 * @param q 
 * @param e 
 * @param interp 
 * @param model_name 
 * @return double 
 */
double spindler_interpolate(double q, double e, struct spindler_interpolator_t* interp, char* model_name){
    /* The table is missing */
    if (interp == NULL){
        fprintf(stderr, "");
        return 0;
    }

    rinterpolate_float_t *table = interp->table;
    rinterpolate_counter_t N = interp->number_of_interpolation_parameters;
    rinterpolate_counter_t D = 1;
    rinterpolate_counter_t L = interp->number_of_interpolation_points;
    rinterpolate_float_t *x = calloc(N, sizeof(rinterpolate_float_t));
    rinterpolate_float_t r[1];
    int usecache = 0;
    
    /* Fill the x array with the coordinates of the point to interpolate*/
    if (strcmp(model_name, "Siwek23") == 0){
        x[0] = q; x[1] = e;
    } else if ((strcmp(model_name, "DD21") || 
                strcmp(model_name, "Zrake21"))){
        x[0] = e;
        if (fabs(q-1) > 0.01){
            printf("Warning: ignoring mass ratio %.3lf != 1.0", q);
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
    rinterpolate_free_data(interp->rinterpolate_data);
    free(interp->rinterpolate_data);
    return r[0];
}

double spindler_get_De(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* interp = (spindler_data->edot_interp);
    char* model_name = (spindler_data->model_name);
    double De;
    if (e != 0){
        return De = spindler_interpolate(q, e, interp, model_name)/e;
    } else {
        return De = 0;
    }
    return De;
}

double spindler_get_Dq(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* interp = (spindler_data->qdot_interp);
    char* model_name = (spindler_data->model_name);
    double Dq = spindler_interpolate(q, e, interp, model_name)/q;
    return Dq;
}

double spindler_get_Da(double q, double e, struct spindler_data_t* spindler_data){
    struct spindler_interpolator_t* interp = (spindler_data->adota_interp);
    char* model_name = (spindler_data->model_name);
    double Da = spindler_interpolate(q, e, interp, model_name);
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
    /* Number of parameters */
    const rinterpolate_counter_t N = 2;

        /* Number of data */
    const rinterpolate_counter_t D = 1;

    /* length of each line (in doubles) i.e. N+D */
    const rinterpolate_counter_t ND = N + D;

    /* make rinterpolate data (for cache etc.) */
    struct rinterpolate_data_t * rinterpolate_data = NULL;
    rinterpolate_counter_t status = rinterpolate_alloc_dataspace(&rinterpolate_data);

    char *filename = "tables/Siwek23/edot.csv";
    double **csvTable = NULL;
    int NColumns;
    int NRows;
    bool readHeader = true;
    char **header = NULL;
    bool isWhitespaceSeparated = false;
    char separator = ',';
    if (read_csv(filename, &csvTable, &NColumns, &NRows, readHeader, &header,
        isWhitespaceSeparated, separator) != 0){
        fprintf(stderr, "Problem reading csv");
        return 1;
    }

    /* total number of lines */
    rinterpolate_counter_t L = NRows;

    /* data table : it is up to you to set the data in here*/
    rinterpolate_float_t *table = malloc(ND*L*sizeof(rinterpolate_float_t));

    for (int i=0; i<NRows; i++){
        for (int j=0; j<NColumns; j++){
            table[i*ND+j] = csvTable[i][j];
            //printf("%lf ", csvTable[i][j]);
        }
        //printf("\n");
    }

    /*
        * Arrays for the interpolation location and
        * interpolated data. You need to set
        * the interpolation location, x.
        */
    rinterpolate_float_t x[N],r[D];

    /* choose whether to cache (0=no, 1=yes) */
    int usecache = 0;
    x[0] = 0.5; x[1] = 0.1;

    /* do the interpolation */
    int steps = 3000000;
    for (int i=0; i<steps; i++){
        //x[0] = 0.1+i/(2.*steps);
        //x[1] = i/(2.*steps);
        //rinterpolate_free_data(rinterpolate_data);
        //free(rinterpolate_data);
        //rinterpolate_data = NULL;
        rinterpolate(table,
                    rinterpolate_data,
                    N,
                    D,
                    L,
                    x,
                    r,
                    usecache);
        //printf("Result: %lf %lf %lf\n", x[0], x[1], r[0]);
    }


    /* the array r contains the result */

    /* ... rest of code ... */


    /* free memory on exit */
    rinterpolate_free_data(rinterpolate_data);
    free(rinterpolate_data);
    free2DArray((void**)header, NColumns);
}