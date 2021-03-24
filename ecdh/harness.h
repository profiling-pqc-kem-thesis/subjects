#ifndef HARNESS_H
#define HARNESS_H

#include "../utilities/harness.h"

#ifdef INSTRUMENTED
#define NUMBER_OF_MEASUREMENTS 2
measurement_group_t *measurement_groups[NUMBER_OF_MEASUREMENTS];
#define measurement_create_curve measurement_groups[0]
#define measurement_generate_key measurement_groups[1]
#else
#define measurement_create_curve
#define measurement_generate_key
#endif

#endif
