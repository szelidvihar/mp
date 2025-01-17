#include "mp/format.h"

#include "cbcmpcommon.h"

#include "CbcSOS.hpp"

namespace mp {

void CbcmpCommon::GetCBCParamsList() const {
  // Print vanilla list for development
  for (auto p : lp()->cbcData->parameters_)
  {
  //   p.printString();
    fmt::print(p.name());

    if (p.type() >= CLP_PARAM_DBL_PRIMALTOLERANCE &&
      p.type() <= CBC_PARAM_DBL_DEXTRA5)
      fmt::print(" (double)");

    if (p.type() >= CLP_PARAM_INT_SOLVERLOGLEVEL &&
      p.type() <= CBC_PARAM_INT_MOREMOREMIPOPTIONS)
      fmt::print("(int)");
    if (p.type() >= CLP_PARAM_STR_DIRECTION &&
      p.type() <= CBC_PARAM_STR_SOSPRIORITIZE)
      fmt::print("(str)");

    fmt::print(": {}\n", p.longHelp());
  }
  // Print code
  for (auto p : lp()->cbcData->parameters_)
  {
   
    if (p.type() >= CLP_PARAM_STR_DIRECTION &&
      p.type() <= CBC_PARAM_STR_SOSPRIORITIZE)
    {
      fmt::print("\n\nAddSolverOption(\":{} {}\",\n \"{}\\n\"\n    \"\\n.. value-table::\\n\",\n", p.name(), p.name(), p.shortHelp());
      fmt::print("\"{}\", {}_values_, \"NULL\");", p.name(), p.name());
      fmt::print("\nstatic const mp::OptionValueInfo {}_values_[] = {{\n", p.name());
      for (auto a : p.definedKeywords())
        fmt::print("{{\"{}\", \"\", 0}},\n", a);
      fmt::print("}};\n");
    }
  }
 
}

int CbcmpCommon::getIntAttr(int name)  const {
  int value = 0;
  /* TODO Utility function to get the value of an integer attribute 
  * from the solver API 
  CBCMP_CCALL(CBCMP_GetIntAttr(lp_, name, &value)); */
  return 0;
}
double CbcmpCommon::getDblAttr(const char* name) const  {
  double value = 0;
  /* TODO Utility function to get the value of an integer attribute
 * from the solver API
  CBCMP_CCALL(CBCMP_GetDblAttr(lp_, name, &value)); */
  return value;
}

int CbcmpCommon::NumLinCons() const {
  return Cbc_getNumRows(lp());
}

int CbcmpCommon::NumVars() const {
  return Cbc_getNumCols(lp());
}

int CbcmpCommon::NumObjs() const {
  return 1;
}

int CbcmpCommon::NumSOSCons() const {
  auto obj = lp()->solver_->objects();
  auto numberObjects = lp()->solver_->numberObjects();
  int nSOS = 0;
  for (auto i= 0; i < numberObjects; i++) {
    CbcSOS* objSOS = dynamic_cast<CbcSOS*>(obj[i]);
    if (objSOS) nSOS++;
  }
  return nSOS;
}
int findIndexOf(const std::vector< std::string > cmdargs_, const char* key) {
  std::string argname = std::string("-") + key;
  for (int i = 0; (i < ((int)cmdargs_.size()) - 1); ++i)
    if (argname == cmdargs_[i])
      return i;
  return -1;
}
int CbcmpCommon::GetSolverOption(const char* key, int &value) const {
  int i = findIndexOf(lp()->cmdargs_, key);
  if (i) return i;
  value = atoi(lp()->cmdargs_[i + 1].c_str());
  return 0;
}

void CbcmpCommon::SetSolverOption(const char* key, int value) {
  std::string s = fmt::format("{}", value);
  Cbc_setParameter(lp(), key, s.data());
}

int CbcmpCommon::GetSolverOption(const char* key, double &value) const {
  int i = findIndexOf(lp()->cmdargs_, key);
  if (i) return i;
  value = atof(lp()->cmdargs_[i + 1].c_str());
  return 0;
}

void CbcmpCommon::SetSolverOption(const char* key, double value) {
  std::string s = fmt::format("{}", value);
  Cbc_setParameter(lp(), key, s.data());
}

int CbcmpCommon::GetSolverOption(const char* key, std::string &value) const {
  int i = findIndexOf(lp()->cmdargs_, key);
  if (i) return i;
  value = lp()->cmdargs_[i + 1];
  return 0;
}

const char * CbcmpCommon::GetSolverOption(const char* key) const {
  int i = findIndexOf(lp()->cmdargs_, key);
  if (i) return NULL;
  return lp()->cmdargs_[i + 1].c_str();
}

void CbcmpCommon::SetSolverOption(const char* key, std::string value) {
  Cbc_setParameter(lp(), key, value.data());
}


} // namespace mp
