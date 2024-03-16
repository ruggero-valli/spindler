#include <rinterpolate.h>
#include <stdio.h>
#include <stdlib.h>
#include "csvreader.h"

struct spindler_interpolator_t {
    double * table;
    unsigned int number_of_interpolation_parameters;
    unsigned int number_of_interpolation_points;
    struct rinterpolate_data_t* rinterpolate_data;
};

int spindler_free_interpolator(struct spindler_interpolator_t* spindler_interpolator){
    free(spindler_interpolator->table);
    rinterpolate_free_data(spindler_interpolator->rinterpolate_data);
    free(spindler_interpolator->rinterpolate_data);
    return RETURN_NO_ERROR;
}

struct spindler_data_t {
    char model_name[128];
    struct spindler_interpolator_t* edot_interp;
    struct spindler_interpolator_t* adota_interp;
    struct spindler_interpolator_t* qdot_interp;
};

// Doesn't free the spindler_data struct itself
int spindler_free_data(struct spindler_data_t* spindler_data){
    spindler_free_interpolator(spindler_data->edot_interp);
    free(spindler_data->edot_interp);
    spindler_free_interpolator(spindler_data->adota_interp);
    free(spindler_data->adota_interp);
    spindler_free_interpolator(spindler_data->qdot_interp);
    free(spindler_data->qdot_interp);
    return RETURN_NO_ERROR;
}

double spindler_init(){
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
}

double spindler_interpolate(double q, double e, struct spindler_interpolator_t* interp, char* model_name){
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