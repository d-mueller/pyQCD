#ifndef ARRAY_EXPR_HPP
#define ARRAY_EXPR_HPP

/* This file provides expression templates for the Array class, so that
 * temporaries do not need to be created when performing arithmetic operations.
 */

#include <typeinfo>
#include <type_traits>

#include <utils/macros.hpp>
#include <utils/templates.hpp>

#include "../layout.hpp"
#include "array_traits.hpp"
#include "operators.hpp"


namespace pyQCD
{
  class ArrayObj { };

  // TODO: Eliminate need for second template parameter
  template <typename T1, typename T2>
  class ArrayExpr : public ArrayObj
  {
    // This is the main expression class from which all others are derived. It
    // uses CRTP to escape inheritance. Parameter T1 is the expression type
    // and T2 is the fundamental type contained in the Array. This allows
    // expressions to be abstracted to a nested hierarchy of types. When the
    // compiler goes through and does it's thing, the definitions of the
    // operations within these template classes are all spliced together.

  public:
    // CRTP magic - call functions in the Array class
    typename ExprReturnTraits<T1, T2>::type operator[](const int i)
    { return static_cast<T1&>(*this)[i]; }
    const typename ExprReturnTraits<T1, T2>::type operator[](const int i) const
    { return static_cast<const T1&>(*this)[i]; }

    unsigned long size() const { return static_cast<const T1&>(*this).size(); }
    const Layout* layout() const
    { return CrtpLayoutTraits<T1>::get_layout(static_cast<const T1&>(*this)); }

    operator T1&() { return static_cast<T1&>(*this); }
    operator T1 const&() const { return static_cast<const T1&>(*this); }
  };


  template <typename T>
  class ArrayConst
    : public ArrayExpr<ArrayConst<T>, T>
  {
    // Expression subclass for const operations
  public:
    // Need some SFINAE here to ensure no clash with copy/move constructor
    template <typename std::enable_if<
      not std::is_same<T, ArrayConst<T> >::value>::type* = nullptr>
    ArrayConst(const T& scalar) : scalar_(scalar) { }
    const T& operator[](const unsigned long i) const { return scalar_; }

  private:
    const T& scalar_;
  };


  template <typename T1, typename T2, typename Op>
  class ArrayUnary
    : public ArrayExpr<ArrayUnary<T1, T2, Op>,
        decltype(Op::apply(std::declval<T2>()))>
  {
  public:
    ArrayUnary(const ArrayExpr<T1, T2>& operand) : operand_(operand) { }

    const decltype(Op::apply(std::declval<T2>()))
    operator[](const unsigned int i) const { return Op::apply(operand_[i]); }

    unsigned long size() const { return operand_.size(); }
    const Layout* layout() const { return operand_.layout(); }

  private:
    typename OperandTraits<T1>::type operand_;
  };


  template <typename T1, typename T2, typename T3, typename T4, typename Op>
  class ArrayBinary
    : public ArrayExpr<ArrayBinary<T1, T2, T3, T4, Op>,
        decltype(Op::apply(std::declval<T3>(), std::declval<T4>()))>
  {
  // Expression subclass for binary operations
  public:
    ArrayBinary(const ArrayExpr<T1, T3>& lhs, const ArrayExpr<T2, T4>& rhs)
      : lhs_(lhs), rhs_(rhs)
    {
      pyQCDassert((BinaryOperandTraits<T1, T2>::equal_size(lhs_, rhs_)),
        std::out_of_range("ArrayBinary: lhs.size() != rhs.size()"));
      pyQCDassert((BinaryOperandTraits<T1, T2>::equal_layout(lhs_, rhs_)),
        std::bad_cast());
    }
    // Here we denote the actual arithmetic operation.
    const decltype(Op::apply(std::declval<T3>(), std::declval<T4>()))
    operator[](const unsigned long i) const
    { return Op::apply(lhs_[i], rhs_[i]); }

    unsigned long size() const
    { return BinaryOperandTraits<T1, T2>::size(lhs_, rhs_); }
    const Layout* layout() const
    { return BinaryOperandTraits<T1, T2>::layout(lhs_, rhs_); }

  private:
    // The members - the inputs to the binary operation
    typename OperandTraits<T1>::type lhs_;
    typename OperandTraits<T2>::type rhs_;
  };

  // Some macros for the operator overloads, as the code is almost
  // the same in each case. For the scalar multiplies I've used
  // some SFINAE to disable these more generalized functions when
  // a ArrayExpr is used.
#define ARRAY_EXPR_OPERATOR(op, trait)                                \
  template <typename T1, typename T2, typename T3, typename T4>       \
  const ArrayBinary<T1, T2, T3, T4, trait>                            \
  operator op(const ArrayExpr<T1, T3>& lhs,                           \
    const ArrayExpr<T2, T4>& rhs)                                     \
  {                                                                   \
    return ArrayBinary<T1, T2, T3, T4, trait>(lhs, rhs);              \
  }                                                                   \
                                                                      \
                                                                      \
  template <typename T1, typename T2, typename T3,                    \
    typename std::enable_if<                                          \
      not std::is_base_of<ArrayObj, T3>::value>::type* = nullptr>     \
  const ArrayBinary<T1, ArrayConst<T3>, T2, T3, trait>                \
  operator op(const ArrayExpr<T1, T2>& array, const T3& scalar)       \
  {                                                                   \
    return ArrayBinary<T1, ArrayConst<T3>, T2, T3, trait>             \
      (array, ArrayConst<T3>(scalar));  \
  }

  // This macro is for the + and * operators where the scalar can
  // be either side of the operator.
#define ARRAY_EXPR_OPERATOR_REVERSE_SCALAR(op, trait)                 \
  template <typename T1, typename T2, typename T3,                    \
    typename std::enable_if<                                          \
      not std::is_base_of<ArrayObj, T1>::value>::type* = nullptr>     \
  const ArrayBinary<ArrayConst<T1>, T2, T1, T3, trait>                \
  operator op(const T1& scalar, const ArrayExpr<T2, T3>& array)       \
  {                                                                   \
    return ArrayBinary<ArrayConst<T1>, T2, T1, T3, trait>             \
      (ArrayConst<T1>(scalar), array);  \
  }


  ARRAY_EXPR_OPERATOR(+, Plus);
  ARRAY_EXPR_OPERATOR_REVERSE_SCALAR(+, Plus);
  ARRAY_EXPR_OPERATOR(-, Minus);
  ARRAY_EXPR_OPERATOR(*, Multiplies);
  ARRAY_EXPR_OPERATOR_REVERSE_SCALAR(*, Multiplies);
  ARRAY_EXPR_OPERATOR(/, Divides);
}

#endif