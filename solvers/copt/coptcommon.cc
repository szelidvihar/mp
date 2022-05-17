#include "mp/format.h"
#include "coptcommon.h"

namespace mp {

void CoptCommon::OpenSolver() {
  int status = 0;
  copt_env* env_p;
  if (createEnv == nullptr)
    status = COPT_CreateEnv(&env_p);
  else
    status = createEnv(&env_p);
  set_env(env_p);
  if ( env() == NULL ) {
     throw std::runtime_error(
       fmt::format("Could not open COPT environment.\n{}", status) );
  }
  copt_prob* prob;
  status = COPT_CreateProb(env_p, &prob);
 
  /* Create an empty model */
  set_lp(prob);
  if (status)
    throw std::runtime_error( fmt::format(
          "Failed to create problem, error code {}.", status ) );
  COPT_CCALL(COPT_SetIntParam(prob, "Logging", 0));

}

void CoptCommon::CloseSolver() {
  if ( lp() != NULL ) {
    COPT_CCALL(COPT_DeleteProb(&lp_ref()) );
  }
  /* Free up the COPT env()ironment, if necessary */
  if ( env() != NULL ) {
    COPT_CCALL(COPT_DeleteEnv(&env_ref()) );
  }
}



int CoptCommon::getIntAttr(const char* name)  const {
  int value;
  COPT_CCALL(COPT_GetIntAttr(lp(), name, &value));
  return value;
}
double CoptCommon::getDblAttr(const char* name) const  {
  double value;
  COPT_CCALL(COPT_GetDblAttr(lp(), name, &value));
  return value;
}

int CoptCommon::NumLinCons() const {
  return getIntAttr(COPT_INTATTR_ROWS);
}

int CoptCommon::NumVars() const {
  return getIntAttr(COPT_INTATTR_COLS);
}

int CoptCommon::NumObjs() const {
  return 1;
}

int CoptCommon::NumQPCons() const {
  return getIntAttr(COPT_INTATTR_QCONSTRS);
}

int CoptCommon::NumSOSCons() const {
  return getIntAttr(COPT_INTATTR_SOSS);
}

int CoptCommon::NumIndicatorCons() const {
  return getIntAttr(COPT_INTATTR_INDICATORS);
}

void CoptCommon::GetSolverOption(const char* key, int &value) const {
  COPT_CCALL( COPT_GetIntParam(lp(), key, &value) );
}

void CoptCommon::SetSolverOption(const char* key, int value) {
  COPT_CCALL(COPT_SetIntParam(lp(), key, value));
}

void CoptCommon::GetSolverOption(const char* key, double &value) const {
  COPT_CCALL(COPT_GetDblParam(lp(), key, &value) );
}

void CoptCommon::SetSolverOption(const char* key, double value) {
  COPT_CCALL(COPT_SetDblParam(lp(), key, value) );
}

void CoptCommon::GetSolverOption(const char* key, std::string &value) const {
  throw std::runtime_error("Not implemented"); // TODO
}

void CoptCommon::SetSolverOption(const char* key, const std::string& value) {
  throw std::runtime_error("Not implemented"); // TODO
}


} // namespace mp