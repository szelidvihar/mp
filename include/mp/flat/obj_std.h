#ifndef STD_OBJ_H
#define STD_OBJ_H

#include <string>
#include <vector>

#include "mp/common.h"
#include "mp/flat/expr_quadratic.h"

namespace mp {

/// Linear objective incl. sense and name
class LinearObjective {
  obj::Type sense_;
  std::vector<double> coefs_;
  std::vector<int> vars_;
  std::string name_;
public:
  /// Construct
  template <class CoefVec=std::initializer_list<double>,
            class VarVec=std::initializer_list<int> >
  LinearObjective(obj::Type s, CoefVec&& c, VarVec&& v,
                  std::string nm = {}) noexcept :
    sense_(s),
    coefs_(std::forward<CoefVec>(c)), vars_(std::forward<VarVec>(v)),
    name_(std::move(nm)){ }
  /// Get sense
  obj::Type obj_sense() const { return sense_; }
  /// Get N terms
  int num_terms() const { assert(check()); return (int)vars_.size(); }
  /// Validate
  bool check() const { return coefs_.size()==vars_.size(); }
  /// Coefs vector
  const std::vector<double>& coefs() const { return coefs_; }
  /// Var vector
  const std::vector<int>& vars() const { return vars_; }
  /// Name
  const char* name() const { return name_.c_str(); }

  /// Testing API
  bool operator==(const LinearObjective& lc) const {
    return sense_==lc.sense_ && coefs_==lc.coefs_ && vars_==lc.vars_;
  }
};

/// Quadragtic objective
class QuadraticObjective : public LinearObjective {
  QuadTerms qt_;
public:
  /// Construct
  QuadraticObjective(LinearObjective&& lc, QuadTerms&& qt) :
    LinearObjective(std::move(lc)), qt_(std::move(qt)) { sort_qp_terms(); }

  /// Get QP terms
  const QuadTerms& GetQPTerms() const { return qt_; }

  /// Sort QP terms
  void sort_qp_terms() {
    qt_.sort_terms();
  }

  /// Testing API
  bool operator==(const QuadraticObjective& qc) const {
    return LinearObjective::operator==(qc) && qt_==qc.qt_;
  }
};

} // namespace mp

#endif // STD_OBJ_H
