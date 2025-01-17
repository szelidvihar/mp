#ifndef INDICATOR_LE_H
#define INDICATOR_LE_H

#include "mp/flat/redef/redef_base.h"
#include "mp/flat/constr_std.h"

namespace mp {

/// Convert IndicatorLinLE
/// b==val ==> c'x<=d
template <class ModelConverter>
class IndicatorLinLEConverter_MIP :
    public BasicItemConverter<ModelConverter> {
public:
  /// Base class
  using Base = BasicItemConverter<ModelConverter>;
  /// Constructor
  IndicatorLinLEConverter_MIP(ModelConverter& mc) : Base(mc) { }
  /// Converted item type
  using ItemType = IndicatorConstraintLinLE;

  /// Conversion
  void Convert(const ItemType& indc, int ) {
    auto binvar=indc.get_binary_var();
    auto bnds = GetMC().ComputeBoundsAndType(
          indc.get_constraint().GetBody());
    ConvertImplicationLE(binvar, indc.get_binary_value(),
                         bnds.ub(), indc.get_constraint());
  }

protected:
  using Base::GetMC;

  /// Linearize (b==val ==> c'x<=d) via big-M
  void ConvertImplicationLE(int b, int val,
                   double body_ub, LinConLE con) {
    /// TODO fail if lb>0 +report .iis if requested
    /// TODO skip if ub<0
		if (body_ub >= GetMC().PracticallyInf()) {
      if ((body_ub = GetMC().bigMDefault())<=0.0)
        throw ConstraintConversionFailure( "IndicatorInfBound",
          "The redefinition of an indicator constraint"
          " \"bin_var==0/1 ==> c'x<=d\" into a big-M constraint failed"
          " due to the absence of a finite upper bound on c'x."
          " If the solver supports indicator constraints, it will be passed"
          " to the solver, otherwise this is a fatal error."
          " To remove this error/warning, the following options can be available:\n"
          "  1. Provide tight bounds on variables entering logical expressions;\n"
          "  2. Use option cvt:mip:bigM to set the default value of big-M (use with care);\n"
          "  3. If available, set acc:indle=2 for native handling of the constraint.");
    }
    if (0==val)                                // left condition is b==0
      con.GetBody().add_term(-body_ub+con.rhs(), b);
    else {
      con.GetBody().add_term(body_ub-con.rhs(), b);
      con.set_rhs(body_ub);
    }
    GetMC().AddConstraint(con);                // Big-M constraint
  }

};

} // namespace mp

#endif // INDICATOR_LE_H
