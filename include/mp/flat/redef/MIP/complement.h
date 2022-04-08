#ifndef COMPLEMENT_H
#define COMPLEMENT_H

#include <cmath>

#include "mp/flat/redef/redef_base.h"
#include "mp/flat/constraints_std.h"

namespace mp {

/// Converts Complementatrity for MIP
template <class ModelConverter, class AlgCon>
class ComplementarityConverter_MIP :
    public BasicFuncConstrCvt<
      ComplementarityConverter_MIP<ModelConverter, AlgCon>,
      ModelConverter> {
public:
  /// Base class
  using Base = BasicFuncConstrCvt<
    ComplementarityConverter_MIP<ModelConverter, AlgCon>,
    ModelConverter>;
  /// Constructor
  ComplementarityConverter_MIP(ModelConverter& mc) : Base(mc) { }
  /// Converted item type
  using ItemType = ComplementarityConstraint<AlgCon>;

  /// Convert in any context
  void Convert(const ItemType& cc, int ) {
    auto alg_con = cc.GetConstraint();                  // make a copy
    auto compl_var = cc.GetVariable();

    double
        con_lb = alg_con.lb(),
        con_ub = alg_con.ub(),
        var_lb = GetMC().lb(compl_var),
        var_ub = GetMC().ub(compl_var);

    bool
        fin_con_lb = std::isfinite(con_lb),
        fin_con_ub = std::isfinite(con_ub),
        fin_var_lb = std::isfinite(var_lb),
        fin_var_ub = std::isfinite(var_ub);

    if (fin_con_lb) {
      assert(!fin_con_ub && fin_var_lb && !fin_var_ub);
      /// res1 = (var <= var_lb)
      auto res_neg_var_lb = GetMC().AssignResultVar2Args(
            LE0Constraint{ { {{1.0}, {compl_var}}, -var_lb } });
      /// res2 = (body <= lb)
      auto res_neg_con_lb = GetMC().AssignResultVar2Args(        // TODO wrong for QuadCon
            LE0Constraint{ { alg_con.GetBody(), -con_lb } });
      /// res3 = (res1 \/ res2)
      auto res_disj = GetMC().AssignResultVar2Args(
            OrConstraint{ { res_neg_var_lb, res_neg_con_lb } });
      GetMC().FixAsTrue(res_disj);
      /// Add the algebraic constraint
      GetMC().AddConstraint( std::move(alg_con) );
    } else if (fin_con_ub) {
      assert(!fin_con_lb && fin_var_ub && !fin_var_lb);
      /// res1 = (var >= var_ub)
      auto res_neg_var_ub = GetMC().AssignResultVar2Args(
            LE0Constraint{ { {{-1.0}, {compl_var}}, var_ub } });
      /// res2 = (body >= ub)
      auto lt = alg_con.GetBody();
      lt.negate();
      auto res_neg_con_ub = GetMC().AssignResultVar2Args(        // TODO wrong for QuadCon
            LE0Constraint{ { std::move(lt), con_ub } });
      /// res3 = (res1 \/ res2)
      auto res_disj = GetMC().AssignResultVar2Args(
            OrConstraint{ { res_neg_var_ub, res_neg_con_ub } });
      GetMC().FixAsTrue(res_disj);
      /// Add the algebraic constraint
      GetMC().AddConstraint( std::move(alg_con) );
    }

  }

  /// Reuse the stored ModelConverter
  using Base::GetMC;
};

/// Typedef linear compl cvt
template <class MC>
using ComplCvtLin_MIP = ComplementarityConverter_MIP<MC, RangeLinCon>;

/// Typedef quadratic compl cvt
template <class MC>
using ComplCvtQuad_MIP = ComplementarityConverter_MIP<MC, QuadraticConstraint>;

} // namespace mp

#endif // COMPLEMENT_H
