#include "cplexmodelapi.h"

namespace mp {

CplexModelAPI::CplexModelAPI()
{

}

void CplexModelAPI::InitProblemModificationPhase() { }

void CplexModelAPI::AddVariables(const VarArrayDef& v) {
  std::vector<char> vtypes(v.size());
  for (size_t i=v.size(); i--; )
    vtypes[i] = var::Type::CONTINUOUS==v.ptype()[i] ?
          CPX_CONTINUOUS : CPX_INTEGER;
  CPLEX_CALL( CPXnewcols (env(), lp(), (int)v.size(), NULL,
                          v.plb(), v.pub(), vtypes.data(), NULL) );
}

void CplexModelAPI::SetLinearObjective( int iobj, const LinearObjective& lo ) {
  if (1>iobj) {
    CPLEX_CALL( CPXchgobjsen (env(), lp(),
                    obj::Type::MAX==lo.obj_sense() ? CPX_MAX : CPX_MIN) );
    CPLEX_CALL( CPXchgobj (env(), lp(), lo.num_terms(),
                           lo.vars().data(), lo.coefs().data()) );
  } else {
//    TODO
  }
}
void CplexModelAPI::AddConstraint(const RangeLinCon& lc) {
  char sense = 'E';                     // good to initialize things
  double rhs = lc.lb();
  if (lc.lb()==lc.ub())
    sense = 'E';
  else {                                // Let solver deal with lb>~ub etc.
    if (lc.lb()>MinusInfinity()) {
      sense = 'G';
    }
    if (lc.ub()<Infinity()) {
      if ('G'==sense)
        sense = 'R';
      else {
        sense = 'L';
        rhs = lc.ub();
      }
    }
  }
  int rmatbeg[] = { 0 };
  CPLEX_CALL( CPXaddrows (env(), lp(), 0, 1, lc.size(), &rhs,
                          &sense, rmatbeg, lc.pvars(), lc.pcoefs(),
                          NULL, NULL) );
  if ('R'==sense) {
    int indices = NumLinCons()-1;
    double range = lc.ub()-lc.lb();
    CPLEX_CALL( CPXchgrngval (env(), lp(), 1, &indices, &range) );
  }
}
void CplexModelAPI::AddConstraint(const LinConLE& lc) {
  char sense = 'L';                     // good to initialize things
  double rhs = lc.rhs();
  int rmatbeg[] = { 0 };
  CPLEX_CALL( CPXaddrows (env(), lp(), 0, 1, lc.size(), &rhs,
                          &sense, rmatbeg, lc.pvars(), lc.pcoefs(),
                          NULL, NULL) );
}
void CplexModelAPI::AddConstraint(const LinConEQ& lc) {
  char sense = 'E';                     // good to initialize things
  double rhs = lc.rhs();
  int rmatbeg[] = { 0 };
  CPLEX_CALL( CPXaddrows (env(), lp(), 0, 1, lc.size(), &rhs,
                          &sense, rmatbeg, lc.pvars(), lc.pcoefs(),
                          NULL, NULL) );
}
void CplexModelAPI::AddConstraint(const LinConGE& lc) {
  char sense = 'G';                     // good to initialize things
  double rhs = lc.rhs();
  int rmatbeg[] = { 0 };
  CPLEX_CALL( CPXaddrows (env(), lp(), 0, 1, lc.size(), &rhs,
                          &sense, rmatbeg, lc.pvars(), lc.pcoefs(),
                          NULL, NULL) );
}


void CplexModelAPI::AddConstraint(const IndicatorConstraintLinLE &ic)  {
  CPLEX_CALL( CPXaddindconstr (env(), lp(),
                               ic.get_binary_var(), !ic.get_binary_value(),
                               (int)ic.get_constraint().size(),
                               ic.get_constraint().rhs(), 'L',
                               ic.get_constraint().pvars(),
                               ic.get_constraint().pcoefs(), NULL) );
}
void CplexModelAPI::AddConstraint(const IndicatorConstraintLinEQ &ic)  {
  CPLEX_CALL( CPXaddindconstr (env(), lp(),
                               ic.get_binary_var(), !ic.get_binary_value(),
                               (int)ic.get_constraint().size(),
                               ic.get_constraint().rhs(), 'E',
                               ic.get_constraint().pvars(),
                               ic.get_constraint().pcoefs(), NULL) );
}

void CplexModelAPI::FinishProblemModificationPhase() {
}


} // namespace mp