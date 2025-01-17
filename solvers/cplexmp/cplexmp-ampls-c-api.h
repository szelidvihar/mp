#ifndef CPLEXAMPLSCAPI_H
#define CPLEXAMPLSCAPI_H

/*
 * C API for MP/CPLEX
 */

#include <ilcplex/cplex.h>

#include "mp/ampls-c-api.h"

/*
 * Below are CPLEX-specific AMPLS API functions.
 * They complement the 'public' AMPLS API defined in ampls-c-api.h.
 */

/// Initialize AMPLS CPLEX.
/// @param slv_opt: a string of solver options
/// (normally provided in the <solver>_options string).
/// Can be NULL.
/// @return pointer to struct AMPLS_MP_Solver to be populated, NULL otherwise
AMPLS_MP_Solver* Open_cplexmp(const char* slv_opt, CCallbacks cb);

/// Shut down solver instance
AMPLS_C_EXPORT void AMPLSClose_cplexmp(AMPLS_MP_Solver* slv);

/// Extract the CPLEX model handle
AMPLS_C_EXPORT CPXLPptr AMPLSGetModel_cplexmp(AMPLS_MP_Solver* slv);

/// Extract the CPLEX environment handle
AMPLS_C_EXPORT CPXENVptr AMPLSGetEnv_cplexmp(AMPLS_MP_Solver* slv);

#endif // CPLEXAMPLSCAPI_H
