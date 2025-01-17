#ifndef AMPLS_C_INTERFACE_H
#define AMPLS_C_INTERFACE_H

#include <stddef.h>
#include "mp/ampls-ccallbacks.h"
/*
 * A generic C API for MP
 */

#ifdef _WIN32
#define AMPLS_C_EXPORT __declspec(dllexport)
#else
#define AMPLS_C_EXPORT __attribute__((visibility("default")))
#endif

/// An AMPLS solver instance
typedef struct AMPLS_MP_Solver_T {
  /// AMPLS internal info
  void *internal_info_;
  /// Extra info, managed by the specific solver
  void *solver_info_;
  /// User info, free to assign
  void *user_info_;
} AMPLS_MP_Solver;


  /// Load model incl. suffixes.
  /// The method assumes that slv has been initialized by
  /// a solver-specific API using functions of ampls-cpp-api.h.
  /// @return 0 on success
  AMPLS_C_EXPORT int AMPLSLoadNLModel(AMPLS_MP_Solver* slv,
    const char* nl_filename);

  /// Report results.
  /// The kind of results reported is influenced by solver option 
  /// `wantsol`.
  /// @return 0 on success
  AMPLS_C_EXPORT int AMPLSReportResults(AMPLS_MP_Solver* slv, const char* solFileName);

  /// Add message
  AMPLS_C_EXPORT void AMPLSAddMessage(AMPLS_MP_Solver* slv, const char* msg);

  /// Retrieve messages, 0-terminated array
  const char* const* AMPLSGetMessages(AMPLS_MP_Solver* slv);


  typedef struct AMPLSCOption_T {
    const char* name;
    const char* description;
    int type; // 0=int, 1=bool, 2=double, 3=string, 4=undefined - aliases
  } AMPLS_C_Option;
  
  AMPLS_C_EXPORT AMPLS_C_Option* AMPLSGetOptions(AMPLS_MP_Solver* slv);
  /// Returns 0 on success
  AMPLS_C_EXPORT int AMPLSSetIntOption(AMPLS_MP_Solver*, const char* name, int value);

  AMPLS_C_EXPORT int AMPLSGetIntOption(AMPLS_MP_Solver*, const char* name, int *value);

  AMPLS_C_EXPORT int AMPLSSetDblOption(void* slv, const char* name, double value);
  AMPLS_C_EXPORT int AMPLSGetDblOption(void* slv, const char* name, double* value);
  AMPLS_C_EXPORT int AMPLSSetStrOption(void* slv, const char* name, const char* value);
  AMPLS_C_EXPORT int AMPLSGetStrOption(void* slv, const char* name, const char* const* value);

#endif // AMPLS_C_INTERFACE_H
