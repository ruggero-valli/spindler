#include <stdio.h>
#include <stdlib.h>
#include "spindler.h"

int main(){
    char* model_name = "Zrake21";
    struct spindler_data_t* spindler_data = calloc(1, sizeof(struct spindler_data_t));
    spindler_init(model_name, spindler_data);

    double q=0.5, e=0.5;
    double Dq, De, Da, DE, DJ;
    Dq = spindler_get_Dq(q, e, spindler_data);
    De = spindler_get_De(q, e, spindler_data);
    Da = spindler_get_Da(q, e, spindler_data);
    DE = spindler_get_DE(q, e, spindler_data);
    DJ = spindler_get_DJ(q, e, spindler_data);

    printf("Dq: %lf, De: %lf, Da: %lf, DE: %lf, DJ: %lf\n", Dq, De, Da, DE, DJ);
mpg
    spindler_free_data(spindler_data);
    free(spindler_data);
    return 0;
}