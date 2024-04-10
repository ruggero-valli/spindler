#ifndef SPINDLER_H
#define SPINDLER_H

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
void spindler_free_data(struct spindler_data_t* spindler_data);

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
int spindler_init(char* model_name, struct spindler_data_t* spindler_data);

/**
 * @brief Computes the derivative of the eccentricity e with respect to the
 *  mass of the binary m.
 * 
 * @param q mass ratio
 * @param e eccentricity
 * @param spindler_data data struct for the interpolation
 * @return dloge/dlogm 
 */
double spindler_get_De(double q, double e, struct spindler_data_t* spindler_data);

/**
 * @brief Computes the derivative of the mass ratio q with respect to the
 *  mass of the binary m.
 * 
 * @param q mass ratio
 * @param e eccentricity
 * @param spindler_data data struct for the interpolation
 * @return dlogq/dlogm 
 */
double spindler_get_Dq(double q, double e, struct spindler_data_t* spindler_data);

/**
 * @brief Computes the derivative of the semimajor axis a with respect to the
 *  mass of the binary m.
 * 
 * @param q mass ratio
 * @param e eccentricity
 * @param spindler_data data struct for the interpolation
 * @return dloga/dlogm 
 */
double spindler_get_Da(double q, double e, struct spindler_data_t* spindler_data);

/**
 * @brief Computes the derivative of the orbital energy E with respect to the
 *  mass of the binary m.
 * 
 * @param q mass ratio
 * @param e eccentricity
 * @param spindler_data data struct for the interpolation
 * @return dlogE/dlogm 
 */
double spindler_get_DE(double q, double e, struct spindler_data_t* spindler_data);

/**
 * @brief Computes the derivative of orbital angular momentum J with respect to the
 *  mass of the binary m.
 * 
 * @param q mass ratio
 * @param e eccentricity
 * @param spindler_data data struct for the interpolation
 * @return dlogJ/dlogm 
 */
double spindler_get_DJ(double q, double e, struct spindler_data_t* spindler_data);

#endif /* SPINDLER_H */
