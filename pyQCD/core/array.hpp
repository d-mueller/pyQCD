#ifndef ARRAY_HPP
#define ARRAY_HPP

/* This file provides an array class, optimized using expression templates and
 * operator overloading. This is probably the most fundamental class in the
 * package, and is the parent to LatticeBase.
 *
 * The reason this has been written from scratch, rather than using the Eigen
 * Array type, is because the Eigen type doesn't support multiplication by
 * generic types, i.e. you can't do something like Array<Matrix3cd> * double.
 *
 * The expression templates that are used by this class can be found in
 * array_expr.hpp
 */

#include <stdexcept>
#include <type_traits>
#include <vector>

#include <utils/macros.hpp>
#include <utils/templates.hpp>
#include "detail/array_expr.hpp"


namespace pyQCD
{
  template <typename T1, template <typename> class Alloc = std::allocator,
    typename T2 = EmptyType>
  class Array
    : public ArrayExpr<typename ArrayCrtpTrait<T2, T1, Alloc>::type, T1>
  {
  template <typename U1, typename U2, typename U3, typename U4, typename Op>
  friend class ArrayBinary;
  public:
    Array() { }
    Array(const int n, const T1& val) : data_(n, val) { }
    Array(const Array<T1, Alloc, T2>& array) = default;
    Array(Array<T1, Alloc, T2>&& array) = default;
    template <typename U1, typename U2>
    Array(const ArrayExpr<U1, U2>& expr)
    {
      data_.resize(expr.size());
      T1* ptr = &data_[0];
      for (unsigned long i = 0; i < expr.size(); ++i) {
        ptr[i] = static_cast<T1>(expr[i]);
      }
    }
    virtual ~Array() = default;

    T1& operator[](const unsigned long i) { return data_[i]; }
    const T1& operator[](const unsigned long i) const { return data_[i]; }

    void resize(const int size) { data_.resize(size); }
    ArrayConst<Array<T1, Alloc, T2> > broadcast() const
    { return ArrayConst<Array<T1, Alloc, T2> >(*this); }

    typename std::vector<T1>::iterator begin() { return data_.begin(); }
    typename std::vector<T1>::const_iterator begin() const
    { return data_.begin(); }
    typename std::vector<T1>::iterator end() { return data_.end(); }
    typename std::vector<T1>::const_iterator end() const { return data_.end(); }

    Array<T1, Alloc, T2>& operator=(const Array<T1, Alloc, T2>& array) = default;
    Array<T1, Alloc, T2>& operator=(Array<T1, Alloc, T2>&& array) = default;
    Array<T1, Alloc, T2>& operator=(const T1& rhs)
    {
      data_.assign(data_.size(), rhs);
      return *this;
    }
    template <typename U1, typename U2>
    Array<T1, Alloc, T2>& operator=(const ArrayExpr<U1, U2>& expr)
    {
      pyQCDassert ((data_.size() == expr.size()),
                   std::out_of_range("Array::data_"));
      T1* ptr = &data_[0];
      for (unsigned long i = 0; i < expr.size(); ++i) {
        ptr[i] = static_cast<T1>(expr[i]);
      }
      return *this;
    }

#define ARRAY_OPERATOR_ASSIGN_DECL(op)				                             \
    template <typename U,                                                  \
      typename std::enable_if<                                             \
		    not std::is_base_of<ArrayObj, U>::value>::type* = nullptr>         \
    Array<T1, Alloc, T2>& operator op ## =(const U& rhs);	                 \
    template <typename U>                                                  \
    Array<T1, Alloc, T2>& operator op ## =(const Array<U, Alloc, T2>& rhs);

    ARRAY_OPERATOR_ASSIGN_DECL(+);
    ARRAY_OPERATOR_ASSIGN_DECL(-);
    ARRAY_OPERATOR_ASSIGN_DECL(*);
    ARRAY_OPERATOR_ASSIGN_DECL(/);

    unsigned long size() const { return data_.size(); }

  protected:
    std::vector<T1, Alloc<T1> > data_;
  };


#define ARRAY_OPERATOR_ASSIGN_IMPL(op)                                      \
  template <typename T1, template <typename> class Alloc, typename T2>      \
  template <typename U,                                                     \
    typename std::enable_if<                                                \
      not std::is_base_of<ArrayObj, U>::value>::type*>                      \
  Array<T1, Alloc, T2>& Array<T1, Alloc, T2>::operator op ## =(const U& rhs)\
  {                                                                         \
    for (auto& item : data_) {                                              \
      item op ## = rhs;                                                     \
    }                                                                       \
    return *this;                                                           \
  }                                                                         \
                                                                            \
                                                                            \
  template <typename T1, template <typename> class Alloc, typename T2>      \
  template <typename U>                                                     \
  Array<T1, Alloc, T2>&                                                     \
  Array<T1, Alloc, T2>::operator op ## =(const Array<U, Alloc, T2>& rhs)    \
  {                                                                         \
    pyQCDassert (rhs.size() == data_.size(),                                \
      std::out_of_range("Arrays must be the same size"));                   \
    for (unsigned long i = 0; i < data_.size(); ++i) {                      \
      data_[i] op ## = rhs[i];                                              \
    }                                                                       \
    return *this;                                                           \
  }

  ARRAY_OPERATOR_ASSIGN_IMPL(+);
  ARRAY_OPERATOR_ASSIGN_IMPL(-);
  ARRAY_OPERATOR_ASSIGN_IMPL(*);
  ARRAY_OPERATOR_ASSIGN_IMPL(/);
}

#endif
