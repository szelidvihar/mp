#include "gurobicommon.h"

namespace mp {

void GurobiCommon::copy_handlers_from_other_gurobi() {
  env_ = other_gurobi()->env();
  model_ = other_gurobi()->model();
}

void GurobiCommon::copy_handlers_to_other_gurobi() {
  assert(other_gurobi());
  other_gurobi()->set_env(env_);
  other_gurobi()->set_model(model_);
}

int GurobiCommon::NumLinCons() const {
  return GrbGetIntAttr(GRB_INT_ATTR_NUMCONSTRS);
}

int GurobiCommon::NumQPCons() const {
  return GrbGetIntAttr(GRB_INT_ATTR_NUMQCONSTRS);
}

int GurobiCommon::NumSOSCons() const {
  return GrbGetIntAttr(GRB_INT_ATTR_NUMSOS);
}

int GurobiCommon::NumGenCons() const {
  return GrbGetIntAttr(GRB_INT_ATTR_NUMGENCONSTRS);
}

int GurobiCommon::NumVars() const {
  return GrbGetIntAttr(GRB_INT_ATTR_NUMVARS);
}

int GurobiCommon::NumObjs() const {
  return GrbGetIntAttr(GRB_INT_ATTR_NUMOBJ);
}

int GurobiCommon::ModelSense() const {
  return GrbGetIntAttr(GRB_INT_ATTR_MODELSENSE);
}


void GurobiCommon::GetSolverOption(const char *key, int &value) const {
  GRB_CALL( GRBgetintparam(GRBgetenv(model_), key, &value) );
}

void GurobiCommon::SetSolverOption(const char *key, int value) {
  GRB_CALL( GRBsetintparam(GRBgetenv(model_), key, value) );
}

void GurobiCommon::GetSolverOption(const char *key, double &value) const {
  GRB_CALL( GRBgetdblparam(GRBgetenv(model_), key, &value) );
}

void GurobiCommon::SetSolverOption(const char *key, double value) {
  GRB_CALL( GRBsetdblparam(GRBgetenv(model_), key, value) );
}

void GurobiCommon::GetSolverOption(const char *key, std::string &value) const {
  char buffer[GRB_MAX_STRLEN];
  GRB_CALL( GRBgetstrparam(GRBgetenv(model_), key, buffer) );
  value = buffer;
}

void GurobiCommon::SetSolverOption(const char *key, const std::string& value) {
  GRB_CALL( GRBsetstrparam(GRBgetenv(model_), key, value.c_str()) );
}


/// Shortcuts for attributes
int GurobiCommon::GrbGetIntAttr(const char* attr_id, bool *flag) const {
  int tmp=0;
  int error = GRBgetintattr(model_, attr_id, &tmp);
  if (flag)
    *flag = (0==error);
  else if (error)
    MP_RAISE( fmt::format("Failed to obtain attribute {}, error code {}",
                       attr_id, error ) );
  return tmp;
}

double GurobiCommon::GrbGetDblAttr(const char* attr_id, bool *flag) const {
  double tmp=0.0;
  int error = GRBgetdblattr(model_, attr_id, &tmp);
  if (flag)
    *flag = (0==error);
  else if (error)
    MP_RAISE( fmt::format("Failed to obtain attribute {}, error code {}",
                       attr_id, error ) );
  return tmp;
}

void GurobiCommon::GrbSetIntAttr(
    const char *attr_id, int val) {
  GRB_CALL( GRBsetintattr(model_, attr_id, val) );
}

void GurobiCommon::GrbSetDblAttr(
    const char *attr_id, double val) {
  GRB_CALL( GRBsetdblattr(model_, attr_id, val) );
}

std::vector<int> GurobiCommon::GrbGetIntAttrArray(const char* attr_id,
    std::size_t size, std::size_t offset) const
{ return GrbGetIntAttrArray(model_, attr_id, size, offset); }

std::vector<int> GurobiCommon::GrbGetIntAttrArray(
    GRBmodel* mdl, const char* attr_id,
    std::size_t size, std::size_t offset) const {
  std::vector<int> res(size);
  int error = GRBgetintattrarray(mdl, attr_id,
    0, (int)(size-offset), res.data()+offset);
  if (error)
    res.clear();
  return res;
}

std::vector<double> GurobiCommon::GrbGetDblAttrArray(const char* attr_id,
    std::size_t size, std::size_t offset) const
{ return GrbGetDblAttrArray(model_, attr_id, size, offset); }

std::vector<double> GurobiCommon::GrbGetDblAttrArray(
    GRBmodel* mdl, const char* attr_id,
    std::size_t size, std::size_t offset ) const {
  std::vector<double> res(size);
  int error = GRBgetdblattrarray(mdl, attr_id,
    0, (int)(size-offset), res.data()+offset);
  if (error)
    res.clear();
  return res;
}

std::vector<double> GurobiCommon::GrbGetDblAttrArray_VarCon(
    const char* attr, int varcon) const
{ return GrbGetDblAttrArray_VarCon(model_, attr, varcon); }

std::vector<double> GurobiCommon::GrbGetDblAttrArray_VarCon(
    GRBmodel* mdl, const char* attr, int varcon) const {
  return GrbGetDblAttrArray(mdl, attr,
                            varcon ? NumLinCons() :
                                     NumVars());
}


void GurobiCommon::GrbSetIntAttrArray(
    const char *attr_id, ArrayRef<int> values, std::size_t start) {
  if (values)
    GRB_CALL( GRBsetintattrarray(model_, attr_id,
              (int)start, (int)values.size(), (int*)values.data()) );
}

void GurobiCommon::GrbSetDblAttrArray(
    const char *attr_id, ArrayRef<double> values, std::size_t start) {
  if (values)
    GRB_CALL( GRBsetdblattrarray(model_, attr_id,
              (int)start, (int)values.size(), (double*)values.data()) );
}

void GurobiCommon::GrbSetIntAttrList(const char *attr_id,
                                      const std::vector<int> &idx,
                                      const std::vector<int> &val) {
  assert(idx.size()==val.size());
  if (idx.size())
    GRB_CALL( GRBsetintattrlist(model_, attr_id,
                (int)idx.size(), (int*)idx.data(), (int*)val.data()) );
}

void GurobiCommon::GrbSetDblAttrList(const char *attr_id,
                                      const std::vector<int> &idx,
                                      const std::vector<double> &val) {
  assert(idx.size()==val.size());
  if (idx.size())
    GRB_CALL( GRBsetdblattrlist(model_, attr_id,
                                (int)idx.size(),
                                (int*)idx.data(), (double*)val.data()) );
}

int GurobiCommon::GrbGetIntParam(const char *key) const {
  int v;
  GetSolverOption(key, v);
  return v;
}
double GurobiCommon::GrbGetDblParam(const char *key) const {
  double v;
  GetSolverOption(key, v);
  return v;
}
std::string GurobiCommon::GrbGetStrParam(const char *key) const {
  std::string v;
  GetSolverOption(key, v);
  return v;
}
void GurobiCommon::GrbSetIntParam(const char *key, int value) {
  SetSolverOption(key, value);
}
void GurobiCommon::GrbSetDblParam(const char *key, double value) {
  SetSolverOption(key, value);
}
void GurobiCommon::GrbSetStrParam(const char *key, const std::string& value) {
  SetSolverOption(key, value);
}



} // namespace mp