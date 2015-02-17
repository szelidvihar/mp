/*
 Expression classes

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

#ifndef MP_EXPR_H_
#define MP_EXPR_H_

#include <cassert>
#include <memory>
#include <vector>

#include "mp/common.h"
#include "mp/error.h"
#include "mp/format.h"

namespace mp {

template <expr::Kind, expr::Kind>
class BasicExpr;

typedef BasicExpr<expr::FIRST_EXPR, expr::LAST_EXPR> Expr;

template <typename Alloc>
class BasicExprFactory;

namespace internal {
// Returns true if the non-null expression e is of type ExprType.
template <typename ExprType>
inline bool Is(expr::Kind k) {
  int kind = k;
  // If FIRST_KIND == LAST_KIND, then a decent optimizing compiler simplifies
  // this to kind == ExprType::FIRST_KIND (checked with GCC 4.8.2).
  // No need to do it ourselves.
  return ExprType::FIRST_KIND <= kind && kind <= ExprType::LAST_KIND;
}

// Casts an expression to type ExprType which must be a subclass of Expr.
// If assertions are enabled, it generates an assertion failure when
// e is not of runtime type ExprType. Otherwise no runtime check is
// performed.
template <typename ExprType>
ExprType Cast(Expr e);

template <typename ExprType>
class ExprIterator;

template<bool B, class T = void>
struct enable_if {};

template<class T>
struct enable_if<true, T> { typedef T type; };
}  // namespace internal

class ExprBase {
 protected:
  // The following members are protected rather than private because they
  // have to be accessible in its subclasses, BasicExpr<E>. This doesn't
  // violate encapsulation because this class is inherited privately and
  // can be thought of as a part of BasicExpr<E> that doesn't depend on
  // template parameter.

  class Impl {
   private:
    // Only ExprFactory should be able to set Impl::kind_.
    expr::Kind kind_;

    template <typename Alloc>
    friend class BasicExprFactory;

   public:
    expr::Kind kind() const { return kind_; }
  };

  const Impl *impl_;

  // Returns a pointer to the implementation.
  const Impl *impl() const { return impl_; }

  ExprBase() : impl_() {}

  template <typename ExprType>
  ExprBase(ExprType e) : impl_(e.impl_) {}

  // Creates an expression from an implementation.
  template <typename TargetExpr>
  static TargetExpr Create(const ExprBase::Impl *impl) {
    MP_ASSERT((!impl || internal::Is<TargetExpr>(impl->kind())),
              "invalid expression kind");
    TargetExpr expr;
    expr.impl_ = impl;
    return expr;
  }

  // Safe bool type.
  typedef void (ExprBase::*SafeBool)() const;

  template <typename ExprType>
  friend class internal::ExprIterator;

 private:
  // A member function representing the true value of SafeBool.
  void True() const {}

 public:
  // Returns the expression kind.
  expr::Kind kind() const { return impl_->kind(); }

  // Returns a value convertible to bool that can be used in conditions but not
  // in comparisons and evaluates to "true" if this expression is not null
  // and "false" otherwise.
  // Example:
  //   if (e) {
  //     // Do something if e is not null.
  //   }
  operator SafeBool() const { return impl_ != 0 ? &ExprBase::True : 0; }
};

// An expression.
// A BasicExpr object represents a reference to an expression so
// it is cheap to construct and pass by value. A type safe way to
// process expressions of different types is by using ExprVisitor.
template <expr::Kind FIRST, expr::Kind LAST = FIRST>
class BasicExpr : private ExprBase {
  friend class ExprBase;

  template <typename ExprType>
  friend ExprType internal::Cast(Expr e);

  template <typename Alloc>
  friend class BasicExprFactory;

 protected:
  typedef ExprBase::Impl Impl;
  using ExprBase::impl;
  using ExprBase::Create;
  typedef BasicExpr Base;

 public:
  enum { FIRST_KIND = FIRST, LAST_KIND = LAST };

  // Constructs a BasicExpr object representing a null reference to an
  // expression. The only operation permitted for such object is copying,
  // assignment and check whether it is null using operator SafeBool.
  BasicExpr() {}

  template <expr::Kind OTHER_FIRST, expr::Kind OTHER_LAST>
  BasicExpr(
      BasicExpr<OTHER_FIRST, OTHER_LAST> other,
      typename internal::enable_if<
        FIRST <= OTHER_FIRST && OTHER_LAST <= LAST, int>::type = 0)
    : ExprBase(other) {}

  using ExprBase::kind;
  using ExprBase::operator SafeBool;
};

template <typename ExprType>
inline ExprType internal::Cast(Expr e) {
  MP_ASSERT(Is<ExprType>(e.kind()), "invalid cast");
  ExprType expr;
  expr.impl_ = e.impl_;
  return expr;
}

// Casts an expression to type ExprType which must be a subclass of Expr.
// Returns a null expression if e is not of runtime type ExprType.
template <typename ExprType>
inline ExprType Cast(Expr e) {
  return internal::Is<ExprType>(e.kind()) ?
        internal::Cast<ExprType>(e) : ExprType();
}

#define MP_EXPR(ExprType) \
  const Impl *impl() const { \
    return static_cast<const Impl*>(ExprType::impl()); \
  } \
  template <typename Alloc> \
  friend class BasicExprFactory

// A numeric expression.
typedef BasicExpr<expr::FIRST_NUMERIC, expr::LAST_NUMERIC> NumericExpr;

// A logical expression.
typedef BasicExpr<expr::FIRST_LOGICAL, expr::LAST_LOGICAL> LogicalExpr;

// A numeric constant.
// Examples: 42, -1.23e-4
class NumericConstant : public BasicExpr<expr::NUMBER> {
 private:
  struct Impl : Base::Impl {
    double value;
  };
  MP_EXPR(Base);

 public:
  // Returns the value of this constant.
  double value() const { return impl()->value; }
};

// A reference to a variable or a common expression.
// Example: x
class Reference :
  public BasicExpr<expr::FIRST_REFERENCE, expr::LAST_REFERENCE> {
 private:
  struct Impl : Base::Impl {
    int index;
  };
  MP_EXPR(Base);

 public:
  // Returns the index of the referenced object.
  int index() const { return impl()->index; }
};

// A unary expression.
// Base: base expression class.
template <typename Arg, expr::Kind FIRST, expr::Kind LAST = FIRST>
class BasicUnaryExpr : public BasicExpr<FIRST, LAST> {
 private:
  typedef BasicExpr<FIRST, LAST> Base;
  struct Impl : Base::Impl {
    const typename Base::Impl *arg;
  };
  MP_EXPR(Base);

 public:
  // Returns the argument of this expression.
  Arg arg() const { return Base::template Create<Arg>(impl()->arg); }
};

// A unary numeric expression.
// Examples: -x, sin(x), where x is a variable.
typedef BasicUnaryExpr<
  NumericExpr, expr::FIRST_UNARY, expr::LAST_UNARY> UnaryExpr;

// A binary expression.
// Base: base expression class.
// Arg: argument expression class.
template <typename Arg, expr::Kind FIRST, expr::Kind LAST = FIRST>
class BasicBinaryExpr : public BasicExpr<FIRST, LAST> {
 private:
  typedef BasicExpr<FIRST, LAST> Base;
  struct Impl : Base::Impl {
    const typename Base::Impl *lhs;
    const typename Base::Impl *rhs;
  };
  MP_EXPR(Base);

 public:
  // Returns the left-hand side (the first argument) of this expression.
  Arg lhs() const { return Base::template Create<Arg>(impl()->lhs); }

  // Returns the right-hand side (the second argument) of this expression.
  Arg rhs() const { return Base::template Create<Arg>(impl()->rhs); }
};

// A binary numeric expression.
// Examples: x / y, atan2(x, y), where x and y are variables.
typedef BasicBinaryExpr<
  NumericExpr, expr::FIRST_BINARY, expr::LAST_BINARY> BinaryExpr;

template <typename Arg, expr::Kind K>
class BasicIfExpr : public BasicExpr<K> {
 private:
  typedef BasicExpr<K> Base;
  struct Impl : Base::Impl {
    const typename Base::Impl *condition;
    const typename Base::Impl *true_expr;
    const typename Base::Impl *false_expr;
  };
  MP_EXPR(Base);

 public:
  static const expr::Kind KIND = K;

  LogicalExpr condition() const {
    return Base::template Create<LogicalExpr>(impl()->condition);
  }

  Arg true_expr() const {
    return Base::template Create<Arg>(impl()->true_expr);
  }
  Arg false_expr() const {
    return Base::template Create<Arg>(impl()->false_expr);
  }
};

// An if-then-else expression.
// Example: if x != 0 then y else z, where x, y and z are variables.
typedef BasicIfExpr<NumericExpr, expr::IF> IfExpr;

// A piecewise-linear term.
// Example: <<0; -1, 1>> x, where x is a variable.
class PLTerm : public BasicExpr<expr::PLTERM> {
 private:
  struct Impl : Base::Impl {
    int num_breakpoints;
    const Base::Impl *arg;
    double data[1];
  };
  MP_EXPR(Base);

 public:
  // Returns the number of breakpoints in this term.
  int num_breakpoints() const { return impl()->num_breakpoints; }

  // Returns the number of slopes in this term.
  int num_slopes() const { return num_breakpoints() + 1; }

  // Returns a breakpoint with the specified index.
  double breakpoint(int index) const {
    MP_ASSERT(index >= 0 && index < num_breakpoints(), "index out of bounds");
    return impl()->data[2 * index + 1];
  }

  // Returns a slope with the specified index.
  double slope(int index) const {
    MP_ASSERT(index >= 0 && index < num_slopes(), "index out of bounds");
    return impl()->data[2 * index];
  }

  // Returns the argument (variable or common expression reference).
  Reference arg() const { return Base::Create<Reference>(impl()->arg); }
};

// A reference to a function.
class Function {
 private:
  struct Impl {
    func::Type type;
    int num_args;
    char name[1];
  };

  const Impl *impl_;

  // A member function representing the true value of SafeBool.
  void True() const {}

  // Safe bool type.
  typedef void (Function::*SafeBool)() const;

  explicit Function(const Impl *impl) : impl_(impl) {}

  friend class CallExpr;

  template <typename Alloc>
  friend class BasicExprFactory;

 public:
  // Constructs a Function object representing a null reference to a
  // function. The only operation permitted for such object is copying,
  // assignment and check whether it is null using operator SafeBool.
  Function() : impl_(0) {}

  // Returns a value convertible to bool that can be used in conditions but not
  // in comparisons and evaluates to "true" if this function is not null
  // and "false" otherwise.
  // Example:
  //   if (f) {
  //     // Do something if f is not null.
  //   }
  operator SafeBool() const { return impl_ != 0 ? &Function::True : 0; }

  // Returns the name of this function.
  const char *name() const { return impl_->name; }

  // Returns the number of arguments.
  int num_args() const { return impl_->num_args; }

  // Returns the type of this function.
  func::Type type() const { return impl_->type; }
};

namespace internal {
// An expression iterator.
template <typename ExprType>
class ExprIterator :
    public std::iterator<std::forward_iterator_tag, ExprType> {
 private:
  const ExprBase::Impl *const *ptr_;

  // An expression proxy used for implementing operator-> in iterators.
  class Proxy {
   private:
    ExprType expr_;

   public:
    explicit Proxy(const ExprBase::Impl *e)
      : expr_(ExprBase::Create<ExprType>(e)) {}

    const ExprType *operator->() const { return &expr_; }
  };

 public:
  explicit ExprIterator(const ExprBase::Impl *const *p = 0) : ptr_(p) {}

  ExprType operator*() const { return ExprBase::Create<ExprType>(*ptr_); }

  Proxy operator->() const { return Proxy(*ptr_); }

  ExprIterator &operator++() {
    ++ptr_;
    return *this;
  }

  ExprIterator operator++(int ) {
    ExprIterator it(*this);
    ++ptr_;
    return it;
  }

  bool operator==(ExprIterator other) const { return ptr_ == other.ptr_; }
  bool operator!=(ExprIterator other) const { return ptr_ != other.ptr_; }
};
}  // namespace internal

// A function call expression.
// Example: f(x), where f is a function and x is a variable.
class CallExpr : public BasicExpr<expr::CALL> {
 private:
  struct Impl : Base::Impl {
    const Function::Impl *func;
    int num_args;
    const Base::Impl *args[1];
  };
  MP_EXPR(Base);

 public:
  Function function() const { return Function(impl()->func); }

  typedef Expr Arg;

  // Returns the number of arguments.
  int num_args() const { return impl()->num_args; }

  // Returns an argument with the specified index.
  Expr arg(int index) {
    MP_ASSERT(index >= 0 && index < num_args(), "index out of bounds");
    return Create<Expr>(impl()->args[index]);
  }

  // An argument iterator.
  typedef internal::ExprIterator<Expr> iterator;

  iterator begin() const { return iterator(impl()->args); }
  iterator end() const { return iterator(impl()->args + num_args()); }
};

template <typename ArgType, expr::Kind FIRST, expr::Kind LAST = FIRST>
class BasicIteratedExpr : public BasicExpr<FIRST, LAST> {
 private:
  typedef BasicExpr<FIRST, LAST> Base;
  struct Impl : Base::Impl {
    int num_args;
    const typename Base::Impl *args[1];
  };
  MP_EXPR(Base);

 public:
  typedef ArgType Arg;

  // Returns the number of arguments.
  int num_args() const { return impl()->num_args; }

  // Returns an argument with the specified index.
  Arg arg(int index) {
    MP_ASSERT(index >= 0 && index < num_args(), "index out of bounds");
    return Base::template Create<Arg>(impl()->args[index]);
  }

  // An argument iterator.
  typedef internal::ExprIterator<Arg> iterator;

  iterator begin() const { return iterator(impl()->args); }
  iterator end() const { return iterator(impl()->args + num_args()); }
};

// A numeric iterated expression such as min, max, sum or numberof.
// Examples:
//   min{i in I} x[i],
//   sum{i in I} x[i],
//   numberof 42 in ({i in I} x[i]),
//   where I is a set and x is a variable.
typedef BasicIteratedExpr<
  NumericExpr, expr::FIRST_ITERATED, expr::LAST_ITERATED> IteratedExpr;

// A symbolic numberof expression.
// Example: numberof (if x then 'a' else 'b') in ('a', 'b', 'c'),
// where x is a variable.
typedef BasicIteratedExpr<Expr, expr::NUMBEROF_SYM> SymbolicNumberOfExpr;

// A count expression.
// Example: count{i in I} (x[i] >= 0), where I is a set and x is a variable.
typedef BasicIteratedExpr<LogicalExpr, expr::COUNT> CountExpr;

// A logical constant.
// Examples: 0, 1
class LogicalConstant : public BasicExpr<expr::BOOL> {
 private:
  struct Impl : Base::Impl {
    bool value;
  };
  MP_EXPR(Base);

 public:
  // Returns the value of this constant.
  bool value() const { return impl()->value; }
};

// A logical NOT expression.
// Example: not a, where a is a logical expression.
typedef BasicUnaryExpr<LogicalExpr, expr::NOT> NotExpr;

// A binary logical expression.
// Examples: a || b, a && b, where a and b are logical expressions.
typedef BasicBinaryExpr<LogicalExpr,
  expr::FIRST_BINARY_LOGICAL, expr::LAST_BINARY_LOGICAL> BinaryLogicalExpr;

// A relational expression.
// Examples: x < y, x != y, where x and y are variables.
typedef BasicBinaryExpr<
  NumericExpr, expr::FIRST_RELATIONAL, expr::LAST_RELATIONAL> RelationalExpr;

// A logical count expression.
// Examples: atleast 1 (x < y, x != y), where x and y are variables.
class LogicalCountExpr :
  public BasicExpr<expr::FIRST_LOGICAL_COUNT, expr::LAST_LOGICAL_COUNT> {
 private:
  struct Impl : Base::Impl {
    const Base::Impl *lhs;
    const Base::Impl *rhs;
  };
  MP_EXPR(Base);

 public:
  // Returns the left-hand side (the first argument) of this expression.
  NumericExpr lhs() const { return Create<NumericExpr>(impl()->lhs); }

  // Returns the right-hand side (the second argument) of this expression.
  CountExpr rhs() const { return Create<CountExpr>(impl()->rhs); }
};

// An implication expression.
// Example: a ==> b else c, where a, b and c are logical expressions.
typedef BasicIfExpr<LogicalExpr, expr::IMPLICATION> ImplicationExpr;

// An iterated logical expression.
// Example: exists{i in I} x[i] >= 0, where I is a set and x is a variable.
typedef BasicIteratedExpr<
  LogicalExpr, expr::FIRST_ITERATED_LOGICAL, expr::LAST_ITERATED_LOGICAL>
  IteratedLogicalExpr;

// A pairwise expression (alldiff or !alldiff).
// Example: alldiff{i in I} x[i], where I is a set and x is a variable.
typedef BasicIteratedExpr<
  NumericExpr, expr::FIRST_PAIRWISE, expr::LAST_PAIRWISE> PairwiseExpr;

class StringLiteral : public BasicExpr<expr::STRING> {
 private:
  struct Impl : Base::Impl {
    char value[1];
  };
  MP_EXPR(Base);

 public:
  const char *value() const { return impl()->value; }
};

// A symbolic if-then-else expression.
// Example: if x != 0 then 'a' else 0, where x is a variable.
typedef BasicIfExpr<Expr, expr::IFSYM> SymbolicIfExpr;

namespace internal {

// Expression types.
struct ExprTypes {
  typedef mp::Expr Expr;
  typedef mp::NumericExpr NumericExpr;
  typedef mp::LogicalExpr LogicalExpr;
  typedef mp::NumericConstant NumericConstant;
  typedef mp::Reference Variable;
  typedef mp::Reference CommonExpr;
  typedef mp::UnaryExpr UnaryExpr;
  typedef mp::BinaryExpr BinaryExpr;
  typedef mp::IfExpr IfExpr;
  typedef mp::PLTerm PLTerm;
  typedef mp::CallExpr CallExpr;
  typedef mp::IteratedExpr VarArgExpr;
  typedef mp::IteratedExpr SumExpr;
  typedef mp::IteratedExpr NumberOfExpr;
  typedef mp::SymbolicNumberOfExpr SymbolicNumberOfExpr;
  typedef mp::CountExpr CountExpr;
  typedef mp::LogicalConstant LogicalConstant;
  typedef mp::NotExpr NotExpr;
  typedef mp::BinaryLogicalExpr BinaryLogicalExpr;
  typedef mp::RelationalExpr RelationalExpr;
  typedef mp::LogicalCountExpr LogicalCountExpr;
  typedef mp::ImplicationExpr ImplicationExpr;
  typedef mp::IteratedLogicalExpr IteratedLogicalExpr;
  typedef mp::PairwiseExpr PairwiseExpr;
  typedef mp::StringLiteral StringLiteral;

  // Checked cast. See mp::Cast.
  template <typename ExprType>
  static ExprType Cast(Expr e) {
    return mp::Cast<ExprType>(e);
  }

  // Unchecked cast. See mp::internal::Cast.
  template <typename ExprType>
  static ExprType UncheckedCast(Expr e) {
    return mp::internal::Cast<ExprType>(e);
  }
};
}  // namespace internal

// An expression factory.
// Alloc: a memory allocator.
// Allocator requirements:
// 1. The allocate function should return a pointer suitably aligned to hold
//    an object of any fundamental alignment like ::operator new(std::size_t)
//    does.
// 2. The deallocate function should be able to handle 0 passed as the
//    second argument.
template <typename Alloc>
class BasicExprFactory : private Alloc {
 private:
  std::vector<const Expr::Impl*> exprs_;
  std::vector<const Function::Impl*> funcs_;

  FMT_DISALLOW_COPY_AND_ASSIGN(BasicExprFactory);

  // Allocates memory for an object of type ExprType::Impl.
  // extra_bytes: extra bytes to allocate at the end.
  template <typename ExprType>
  typename ExprType::Impl *Allocate(expr::Kind kind, int extra_bytes = 0) {
    // Call push_back first to make sure that the impl pointer doesn't leak
    // if push_back throws an exception.
    exprs_.push_back(0);
    typedef typename ExprType::Impl Impl;
    Impl *impl = reinterpret_cast<Impl*>(
          this->allocate(sizeof(Impl) + extra_bytes));
    impl->kind_ = kind;
    exprs_.back() = impl;
    return impl;
  }

  template <typename T>
  void Deallocate(const std::vector<T> &data);

  // Makes a reference expression.
  Reference MakeReference(expr::Kind kind, int index) {
    typename Reference::Impl *impl = Allocate<Reference>(kind);
    impl->index = index;
    return Expr::Create<Reference>(impl);
  }

  template <typename ExprType, typename Arg>
  ExprType MakeUnary(expr::Kind kind, Arg arg) {
    MP_ASSERT(arg != 0, "invalid argument");
    typename ExprType::Impl *impl = Allocate<ExprType>(kind);
    impl->arg = arg.impl_;
    return Expr::Create<ExprType>(impl);
  }

  template <typename ExprType, typename LHS, typename RHS>
  ExprType MakeBinary(expr::Kind kind, LHS lhs, RHS rhs) {
    MP_ASSERT(internal::Is<ExprType>(kind), "invalid expression kind");
    MP_ASSERT(lhs != 0 && rhs != 0, "invalid argument");
    typename ExprType::Impl *impl = Allocate<ExprType>(kind);
    impl->lhs = lhs.impl_;
    impl->rhs = rhs.impl_;
    return Expr::Create<ExprType>(impl);
  }

  template <typename ExprType, typename Arg>
  ExprType MakeIf(LogicalExpr condition, Arg true_expr, Arg false_expr) {
    // false_expr can be null.
    MP_ASSERT(condition != 0 && true_expr != 0, "invalid argument");
    typename ExprType::Impl *impl = Allocate<ExprType>(ExprType::KIND);
    impl->condition = condition.impl_;
    impl->true_expr = true_expr.impl_;
    impl->false_expr = false_expr.impl_;
    return Expr::Create<ExprType>(impl);
  }

  // A variable argument expression builder.
  template <typename ExprType>
  class BasicIteratedExprBuilder {
   private:
    typename ExprType::Impl *impl_;
    int arg_index_;

    friend class BasicExprFactory;

    explicit BasicIteratedExprBuilder(typename ExprType::Impl *impl)
      : impl_(impl), arg_index_(0) {}

   public:
    void AddArg(typename ExprType::Arg arg) {
      MP_ASSERT(arg_index_ < impl_->num_args, "too many arguments");
      MP_ASSERT(arg != 0, "invalid argument");
      impl_->args[arg_index_++] = arg.impl();
    }
  };

  template <typename ExprType>
  BasicIteratedExprBuilder<ExprType> BeginIterated(
        expr::Kind kind, int num_args) {
    MP_ASSERT(num_args >= 0, "invalid number of arguments");
    typename ExprType::Impl *impl =
        Allocate<ExprType>(kind, sizeof(Expr::Impl*) * (num_args - 1));
    impl->num_args = num_args;
    return BasicIteratedExprBuilder<ExprType>(impl);
  }

  template <typename ExprType>
  ExprType EndIterated(BasicIteratedExprBuilder<ExprType> builder) {
    typename ExprType::Impl *impl = builder.impl_;
    // Check that all arguments provided.
    MP_ASSERT(builder.arg_index_ == impl->num_args, "too few arguments");
    return Expr::Create<ExprType>(impl);
  }

  // Copies a string.
  // src: Reference to a source string that may not be null-terminated
  //      (.nl reader can generate such strings)
  static void Copy(fmt::StringRef src, char *dst) {
    const char *s = src.c_str();
    std::size_t size = src.size();
    std::copy(s, s + size, dst);
    dst[size] = 0;
  }

 public:
  explicit BasicExprFactory(Alloc alloc = Alloc()) : Alloc(alloc) {}

  virtual ~BasicExprFactory() {
    Deallocate(exprs_);
    Deallocate(funcs_);
  }

  // Adds a function.
  // name: Function name that may not be null-terminated.
  Function AddFunction(fmt::StringRef name, int num_args,
                       func::Type type = func::NUMERIC);

  // Makes a numeric constant.
  NumericConstant MakeNumericConstant(double value) {
    NumericConstant::Impl *impl = Allocate<NumericConstant>(expr::NUMBER);
    impl->value = value;
    return Expr::Create<NumericConstant>(impl);
  }

  // Makes a variable reference.
  Reference MakeVariable(int index) {
    return MakeReference(expr::VARIABLE, index);
  }

  // Makes a common expression reference.
  Reference MakeCommonExpr(int index) {
    return MakeReference(expr::COMMON_EXPR, index);
  }

  // Makes a unary expression.
  UnaryExpr MakeUnary(expr::Kind kind, NumericExpr arg) {
    MP_ASSERT(internal::Is<UnaryExpr>(kind), "invalid expression kind");
    return MakeUnary<UnaryExpr>(kind, arg);
  }

  // Makes a binary expression.
  BinaryExpr MakeBinary(expr::Kind kind, NumericExpr lhs, NumericExpr rhs) {
    return MakeBinary<BinaryExpr>(kind, lhs, rhs);
  }

  // Makes an if expression.
  IfExpr MakeIf(LogicalExpr condition,
                NumericExpr true_expr, NumericExpr false_expr) {
    return MakeIf<IfExpr>(condition, true_expr, false_expr);
  }

  // A piecewise-linear term builder.
  class PLTermBuilder {
   private:
    PLTerm::Impl *impl_;
    int slope_index_;
    int breakpoint_index_;

    friend class BasicExprFactory;

    explicit PLTermBuilder(PLTerm::Impl *impl)
      : impl_(impl), slope_index_(0), breakpoint_index_(0) {}

   public:
    void AddSlope(double slope) {
      MP_ASSERT(slope_index_ < impl_->num_breakpoints + 1, "too many slopes");
      impl_->data[2 * slope_index_] = slope;
      ++slope_index_;
    }

    void AddBreakpoint(double breakpoint) {
      MP_ASSERT(breakpoint_index_ < impl_->num_breakpoints,
                "too many breakpoints");
      impl_->data[2 * breakpoint_index_ + 1] = breakpoint;
      ++breakpoint_index_;
    }
  };

  // Begins building a piecewise-linear term.
  PLTermBuilder BeginPLTerm(int num_breakpoints) {
    MP_ASSERT(num_breakpoints > 0, "invalid number of breakpoints");
    PLTerm::Impl *impl = Allocate<PLTerm>(
          expr::PLTERM, sizeof(double) * num_breakpoints * 2);
    impl->num_breakpoints = num_breakpoints;
    return PLTermBuilder(impl);
  }

  // Ends building a piecewise-linear term.
  // arg: argument that should be either a variable or a common expression.
  PLTerm EndPLTerm(PLTermBuilder builder, Reference arg) {
    PLTerm::Impl *impl = builder.impl_;
    // Check that all slopes and breakpoints provided.
    MP_ASSERT(builder.slope_index_ == impl->num_breakpoints + 1,
              "too few slopes");
    MP_ASSERT(builder.breakpoint_index_ == impl->num_breakpoints,
              "too few breakpoints");
    MP_ASSERT(arg != 0, "invalid argument");
    impl->arg = arg.impl_;
    return Expr::Create<PLTerm>(impl);
  }

  typedef BasicIteratedExprBuilder<CallExpr> CallExprBuilder;

  // Begins building a call expression.
  CallExprBuilder BeginCall(Function func, int num_args) {
    MP_ASSERT(func != 0, "invalid function");
    CallExprBuilder builder = BeginIterated<CallExpr>(expr::CALL, num_args);
    builder.impl_->func = func.impl_;
    return builder;
  }

  // Ends building a call expression.
  CallExpr EndCall(CallExprBuilder builder) {
    return EndIterated<CallExpr>(builder);
  }

  typedef BasicIteratedExprBuilder<IteratedExpr> IteratedExprBuilder;

  // Begins building an iterated expression.
  IteratedExprBuilder BeginIterated(expr::Kind kind, int num_args) {
    MP_ASSERT(internal::Is<IteratedExpr>(kind), "invalid expression kind");
    return BeginIterated<IteratedExpr>(kind, num_args);
  }

  // Ends building an iterated expression.
  IteratedExpr EndIterated(IteratedExprBuilder builder) {
    return EndIterated<IteratedExpr>(builder);
  }

  typedef IteratedExprBuilder NumberOfExprBuilder;

  // Begins building a numberof expression.
  NumberOfExprBuilder BeginNumberOf(int num_args, NumericExpr arg0) {
    MP_ASSERT(num_args >= 1, "invalid number of arguments");
    IteratedExprBuilder builder =
        BeginIterated<IteratedExpr>(expr::NUMBEROF, num_args);
    builder.AddArg(arg0);
    return builder;
  }

  // Ends building a numberof expression.
  IteratedExpr EndNumberOf(NumberOfExprBuilder builder) {
    return EndIterated(builder);
  }

  typedef BasicIteratedExprBuilder<SymbolicNumberOfExpr>
          SymbolicNumberOfExprBuilder;

  // Begins building a numberof expression.
  SymbolicNumberOfExprBuilder BeginSymbolicNumberOf(int num_args, Expr arg0) {
    MP_ASSERT(num_args >= 1, "invalid number of arguments");
    SymbolicNumberOfExprBuilder builder =
        BeginIterated<SymbolicNumberOfExpr>(expr::NUMBEROF_SYM, num_args);
    builder.AddArg(arg0);
    return builder;
  }

  // Ends building a numberof expression.
  SymbolicNumberOfExpr EndSymbolicNumberOf(
      SymbolicNumberOfExprBuilder builder) {
    return EndIterated<SymbolicNumberOfExpr>(builder);
  }

  typedef BasicIteratedExprBuilder<CountExpr> CountExprBuilder;

  // Begins building a count expression.
  CountExprBuilder BeginCount(int num_args) {
    return BeginIterated<CountExpr>(expr::COUNT, num_args);
  }

  // Ends building a count expression.
  CountExpr EndCount(CountExprBuilder builder) {
    return EndIterated<CountExpr>(builder);
  }

  // Makes a logical constant.
  LogicalConstant MakeLogicalConstant(bool value) {
    LogicalConstant::Impl *impl = Allocate<LogicalConstant>(expr::BOOL);
    impl->value = value;
    return Expr::Create<LogicalConstant>(impl);
  }

  // Makes a logical NOT expression.
  NotExpr MakeNot(LogicalExpr arg) {
    return MakeUnary<NotExpr>(expr::NOT, arg);
  }

  // Makes a binary logical expression.
  BinaryLogicalExpr MakeBinaryLogical(
        expr::Kind kind, LogicalExpr lhs, LogicalExpr rhs) {
    return MakeBinary<BinaryLogicalExpr>(kind, lhs, rhs);
  }

  // Makes a relational expression.
  RelationalExpr MakeRelational(
        expr::Kind kind, NumericExpr lhs, NumericExpr rhs) {
    return MakeBinary<RelationalExpr>(kind, lhs, rhs);
  }

  // Makes a logical count expression.
  LogicalCountExpr MakeLogicalCount(
        expr::Kind kind, NumericExpr lhs, CountExpr rhs) {
    return MakeBinary<LogicalCountExpr>(kind, lhs, rhs);
  }

  // Makes an implication expression.
  ImplicationExpr MakeImplication(LogicalExpr condition, LogicalExpr true_expr,
                                  LogicalExpr false_expr) {
    return MakeIf<ImplicationExpr>(condition, true_expr, false_expr);
  }

  typedef BasicIteratedExprBuilder<IteratedLogicalExpr>
      IteratedLogicalExprBuilder;

  // Begins building an iterated logical expression.
  IteratedLogicalExprBuilder BeginIteratedLogical(
        expr::Kind kind, int num_args) {
    MP_ASSERT(internal::Is<IteratedLogicalExpr>(kind),
              "invalid expression kind");
    return BeginIterated<IteratedLogicalExpr>(kind, num_args);
  }

  // Ends building an iterated logical expression.
  IteratedLogicalExpr EndIteratedLogical(IteratedLogicalExprBuilder builder) {
    return EndIterated<IteratedLogicalExpr>(builder);
  }

  typedef BasicIteratedExprBuilder<PairwiseExpr> PairwiseExprBuilder;

  // Begins building a pairwise expression.
  PairwiseExprBuilder BeginPairwise(expr::Kind kind, int num_args) {
    return BeginIterated<PairwiseExpr>(kind, num_args);
  }

  // Ends building a pairwise expression.
  PairwiseExpr EndPairwise(PairwiseExprBuilder builder) {
    return EndIterated<PairwiseExpr>(builder);
  }

  // Makes a string literal.
  StringLiteral MakeStringLiteral(fmt::StringRef value) {
    // StringLiteral::Impl already has space for terminating null char so
    // we need to allocate extra size chars only.
    StringLiteral::Impl *impl =
        Allocate<StringLiteral>(expr::STRING, value.size());
    Copy(value, impl->value);
    return Expr::Create<StringLiteral>(impl);
  }

  // Makes a symbolic if expression.
  SymbolicIfExpr MakeSymbolicIf(LogicalExpr condition,
                                Expr true_expr, Expr false_expr) {
    return MakeIf<SymbolicIfExpr>(condition, true_expr, false_expr);
  }
};

template <typename Alloc>
template <typename T>
void BasicExprFactory<Alloc>::Deallocate(const std::vector<T> &data) {
  for (typename std::vector<T>::const_iterator
       i = data.begin(), end = data.end(); i != end; ++i) {
    this->deallocate(const_cast<char*>(reinterpret_cast<const char*>(*i)), 0);
  }
}

template <typename Alloc>
Function BasicExprFactory<Alloc>::AddFunction(
    fmt::StringRef name, int num_args, func::Type type) {
  // Call push_back first to make sure that the impl pointer doesn't leak
  // if push_back throws an exception.
  funcs_.push_back(0);
  // Function::Impl already has space for terminating null char so
  // we need to allocate extra size chars only.
  typedef Function::Impl Impl;
  Impl *impl = reinterpret_cast<Impl*>(
        this->allocate(sizeof(Impl) + name.size()));
  impl->type = type;
  impl->num_args = num_args;
  Copy(name, impl->name);
  funcs_.back() = impl;
  return Function(impl);
}

typedef BasicExprFactory< std::allocator<char> > ExprFactory;

void format(fmt::BasicFormatter<char> &f, const char *, NumericExpr e);

// Returns true iff e is a zero constant.
inline bool IsZero(NumericExpr e) {
  NumericConstant c = Cast<NumericConstant>(e);
  return c && c.value() == 0;
}
}  // namespace mp

#endif  // MP_EXPR_H_
