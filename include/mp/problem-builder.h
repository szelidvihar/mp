/*
 A minimal implementation of the ProblemBuilder concept.

 Copyright (C) 2014 AMPL Optimization Inc

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies and that
 both that the copyright notice and this permission notice and warranty
 disclaimer appear in supporting documentation.

 The author and AMPL Optimization Inc disclaim all warranties with
 regard to this software, including all implied warranties of
 merchantability and fitness.  In no event shall the author be liable
 for any special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether in an
 action of contract, negligence or other tortious action, arising out
 of or in connection with the use or performance of this software.

 Author: Victor Zverovich
 */

#ifndef MP_PROBLEM_BUILDER_H_
#define MP_PROBLEM_BUILDER_H_

#include <cassert>
#include <cstring>
#include <memory>
#include <set>
#include <vector>

#include "mp/error.h"
#include "mp/problem-base.h"
#include "mp/suffix.h"

namespace mp {

// A minimal implementation of the ProblemBuilder concept.
template <typename Impl, typename ExprT>
class ProblemBuilder {
 private:
  SuffixManager suffixes_;

  struct ArgHandler {
    void AddArg(ExprT arg) { MP_UNUSED(arg); }
  };

 public:
  typedef ExprT Expr;
  typedef Expr NumericExpr;
  typedef Expr LogicalExpr;
  typedef Expr CountExpr;
  typedef Expr Variable;

  typedef Suffix *SuffixPtr;
  typedef mp::SuffixSet SuffixSet;

  SuffixSet &suffixes(int kind) { return suffixes_.get(kind); }

  static void ReportUnhandledConstruct(fmt::StringRef name) {
    throw Error("unsupported: {}", name);
  }

  void SetInfo(const ProblemInfo &) {}
  void EndBuild() {}

  // Adds a variable.
  void AddVar(double lb, double ub, var::Type type) {
    MP_UNUSED3(lb, ub, type);
    MP_DISPATCH(ReportUnhandledConstruct("variable"));
  }

  struct LinearExprBuilder {
    void AddTerm(int var_index, double coef) { MP_UNUSED2(var_index, coef); }
  };

  typedef LinearExprBuilder LinearObjBuilder;

  // Adds an objective.
  // Returns a handler for receiving linear terms in an objective.
  LinearObjBuilder AddObj(
      obj::Type type, NumericExpr expr, int num_linear_terms) {
    MP_UNUSED3(type, expr, num_linear_terms);
    MP_DISPATCH(ReportUnhandledConstruct("objective"));
    return LinearObjBuilder();
  }

  typedef LinearExprBuilder LinearConBuilder;

  // Adds an algebraic constraint.
  // Returns a handler for receiving linear terms in a constraint.
  LinearConBuilder AddCon(NumericExpr expr, double lb, double ub,
                          int num_linear_terms) {
    MP_UNUSED3(expr, lb, ub); MP_UNUSED(num_linear_terms);
    MP_DISPATCH(ReportUnhandledConstruct("algebraic constraint"));
    return LinearConBuilder();
  }

  // Adds a logical constraint.
  void AddCon(LogicalExpr expr) {
    MP_UNUSED(expr);
    MP_DISPATCH(ReportUnhandledConstruct("logical constraint"));
  }

  // Adds a common expression (defined variable).
  // Returns a handler for receiving linear terms in a defined variable
  // expression.
  LinearExprBuilder BeginCommonExpr(NumericExpr expr, int position,
                                    int num_linear_terms) {
    MP_UNUSED3(expr, position, num_linear_terms);
    MP_DISPATCH(ReportUnhandledConstruct("common expression"));
    return LinearExprBuilder();
  }

  NumericExpr EndCommonExpr(LinearExprBuilder builder) {
    MP_UNUSED(builder);
  }

  // Sets a complementarity relation.
  void SetComplement(int con_index, int var_index, int flags) {
    MP_UNUSED3(con_index, var_index, flags);
    MP_DISPATCH(ReportUnhandledConstruct("complementarity constraint"));
  }

  void SetInitialValue(int var_index, double value) {
    MP_UNUSED2(var_index, value);
    MP_DISPATCH(ReportUnhandledConstruct("initial value"));
  }
  void SetInitialDualValue(int con_index, double value) {
    MP_UNUSED2(con_index, value);
    MP_DISPATCH(ReportUnhandledConstruct("initial dual value"));
  }

  struct ColumnSizeHandler {
    void Add(int size) { MP_UNUSED(size); }
  };

  // Returns a handler that receives column sizes in Jacobian.
  ColumnSizeHandler GetColumnSizeHandler() {
    MP_DISPATCH(ReportUnhandledConstruct("Jacobian column size"));
    return ColumnSizeHandler();
  }

  // Sets a function at the given index.
  void SetFunction(int index, fmt::StringRef name,
                   int num_args, func::Type type) {
    MP_UNUSED3(index, name, num_args); MP_UNUSED(type);
    MP_DISPATCH(ReportUnhandledConstruct("function"));
  }

  struct SuffixHandler {
    void SetValue(int index, int value) { MP_UNUSED2(index, value); }
    void SetValue(int index, double value) { MP_UNUSED2(index, value); }
  };

  // Adds a suffix.
  SuffixHandler AddSuffix(int kind, int num_values, fmt::StringRef name) {
    MP_UNUSED3(kind, num_values, name);
    MP_DISPATCH(ReportUnhandledConstruct("suffix"));
    return SuffixHandler();
  }

  typedef ArgHandler NumericArgHandler;
  typedef ArgHandler LogicalArgHandler;
  typedef ArgHandler VarArgHandler;
  typedef ArgHandler CallArgHandler;
  typedef ArgHandler NumberOfArgHandler;

  NumericExpr MakeNumericConstant(double value) {
    MP_UNUSED(value);
    MP_DISPATCH(ReportUnhandledConstruct(
                  "numeric constant in nonlinear expression"));
    return NumericExpr();
  }

  Variable MakeVariable(int var_index) {
    MP_UNUSED(var_index);
    MP_DISPATCH(ReportUnhandledConstruct("variable in nonlinear expression"));
    return Variable();
  }

  NumericExpr MakeUnary(expr::Kind kind, NumericExpr arg) {
    MP_UNUSED2(kind, arg);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return NumericExpr();
  }

  NumericExpr MakeBinary(expr::Kind kind, NumericExpr lhs, NumericExpr rhs) {
    MP_UNUSED3(kind, lhs, rhs);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return NumericExpr();
  }

  NumericExpr MakeIf(LogicalExpr condition,
      NumericExpr true_expr, NumericExpr false_expr) {
    MP_UNUSED3(condition, true_expr, false_expr);
    MP_DISPATCH(ReportUnhandledConstruct("if expression"));
    return NumericExpr();
  }

  struct PLTermHandler {
    void AddSlope(double slope) { MP_UNUSED(slope); }
    void AddBreakpoint(double breakpoint) { MP_UNUSED(breakpoint); }
  };

  PLTermHandler BeginPLTerm(int num_breakpoints) {
    MP_UNUSED(num_breakpoints);
    MP_DISPATCH(ReportUnhandledConstruct("piecewise-linear term"));
    return PLTermHandler();
  }
  NumericExpr EndPLTerm(PLTermHandler handler, Variable var) {
    MP_UNUSED2(handler, var);
    MP_DISPATCH(ReportUnhandledConstruct("piecewise-linear term"));
    return NumericExpr();
  }

  CallArgHandler BeginCall(int func_index, int num_args) {
    MP_UNUSED2(func_index, num_args);
    MP_DISPATCH(ReportUnhandledConstruct("function call"));
    return CallArgHandler();
  }
  NumericExpr EndCall(CallArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("function call"));
    return NumericExpr();
  }

  VarArgHandler BeginVarArg(expr::Kind kind, int num_args) {
    MP_UNUSED2(kind, num_args);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return NumericArgHandler();
  }
  NumericExpr EndVarArg(VarArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("vararg expression"));
    return NumericExpr();
  }

  NumericArgHandler BeginSum(int num_args) {
    MP_UNUSED(num_args);
    MP_DISPATCH(ReportUnhandledConstruct("sum"));
    return NumericArgHandler();
  }
  NumericExpr EndSum(NumericArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("sum"));
    return NumericExpr();
  }

  LogicalArgHandler BeginCount(int num_args) {
    MP_UNUSED(num_args);
    MP_DISPATCH(ReportUnhandledConstruct("count expression"));
    return LogicalArgHandler();
  }
  NumericExpr EndCount(LogicalArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("count expression"));
    return NumericExpr();
  }

  NumberOfArgHandler BeginNumberOf(int num_args, NumericExpr value) {
    MP_UNUSED2(num_args, value);
    MP_DISPATCH(ReportUnhandledConstruct("numberof expression"));
    return NumberOfArgHandler();
  }
  NumericExpr EndNumberOf(NumberOfArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("numberof expression"));
    return NumericExpr();
  }

  LogicalExpr MakeLogicalConstant(bool value) {
    MP_UNUSED(value);
    MP_DISPATCH(ReportUnhandledConstruct("logical constant"));
    return LogicalExpr();
  }

  LogicalExpr MakeNot(LogicalExpr arg) {
    MP_UNUSED(arg);
    MP_DISPATCH(ReportUnhandledConstruct("logical not"));
    return LogicalExpr();
  }

  LogicalExpr MakeBinaryLogical(
      expr::Kind kind, LogicalExpr lhs, LogicalExpr rhs) {
    MP_UNUSED3(kind, lhs, rhs);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return LogicalExpr();
  }

  LogicalExpr MakeRelational(
      expr::Kind kind, NumericExpr lhs, NumericExpr rhs) {
    MP_UNUSED3(kind, lhs, rhs);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return LogicalExpr();
  }

  LogicalExpr MakeLogicalCount(
      expr::Kind kind, NumericExpr lhs, CountExpr rhs) {
    MP_UNUSED3(kind, lhs, rhs);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return LogicalExpr();
  }

  LogicalExpr MakeImplication(
      LogicalExpr condition, LogicalExpr true_expr, LogicalExpr false_expr) {
    MP_UNUSED3(condition, true_expr, false_expr);
    MP_DISPATCH(ReportUnhandledConstruct("implication expression"));
    return LogicalExpr();
  }

  LogicalArgHandler BeginIteratedLogical(expr::Kind kind, int num_args) {
    MP_UNUSED2(kind, num_args);
    MP_DISPATCH(ReportUnhandledConstruct(str(kind)));
    return LogicalArgHandler();
  }
  LogicalExpr EndIteratedLogical(LogicalArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("iterated logical expression"));
    return LogicalExpr();
  }

  typedef ArgHandler AllDiffArgHandler;

  AllDiffArgHandler BeginAllDiff(int num_args) {
    MP_UNUSED(num_args);
    MP_DISPATCH(ReportUnhandledConstruct("alldiff expression"));
    return AllDiffArgHandler();
  }
  LogicalExpr EndAllDiff(AllDiffArgHandler handler) {
    MP_UNUSED(handler);
    MP_DISPATCH(ReportUnhandledConstruct("alldiff expression"));
    return LogicalExpr();
  }

  // Constructs a StringLiteral object.
  // value: string value which may not be null-terminated.
  Expr MakeStringLiteral(fmt::StringRef value) {
    MP_UNUSED(value);
    MP_DISPATCH(ReportUnhandledConstruct("string literal"));
    return Expr();
  }
};
}  // namespace mp

#endif  // MP_PROBLEM_BUILDER_H_
