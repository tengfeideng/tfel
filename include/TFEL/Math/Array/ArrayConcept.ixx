/*!
 * \file   include/TFEL/Math/Array/ArrayConcept.ixx
 * \author Helfer Thomas
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef _LIB_TFEL_MATH_ARRAY_CONCEPT_IMPL_
#define _LIB_TFEL_MATH_ARRAY_CONCEPT_IMPL_ 1

namespace tfel{

  namespace math{

    template<typename T>
    class ArrayConcept_impl<1u,T>
    {
      typedef ArrayTraits<T> traits;
      static const bool isTemporary = tfel::typetraits::IsTemporary<T>::cond;
      typedef typename tfel::meta::IF<isTemporary,
				      typename traits::NumType,
				      const typename traits::NumType&>::type ValueType;
    public:
      TFEL_MATH_INLINE 
      ValueType
      operator()(const unsigned int i) const
      {
	return static_cast<const T&>(*this).operator()(i);
      }
    };

    template<typename T>
    class ArrayConcept_impl<2u,T>
    {
      typedef ArrayTraits<T> traits;
      static const bool isTemporary = tfel::typetraits::IsTemporary<T>::cond;
      typedef typename tfel::meta::IF<isTemporary,
				      typename traits::NumType,
				      const typename traits::NumType&>::type ValueType;
    public:
      TFEL_MATH_INLINE 
      ValueType
      operator()(const unsigned int i,
		 const unsigned int j) const
      {
	return static_cast<const T&>(*this).operator()(i,j);
      }
    };

    template<typename T>
    struct ArrayConcept_impl<3u,T>
    {
      typedef ArrayTraits<T> traits;
      static const bool isTemporary = tfel::typetraits::IsTemporary<T>::cond;
      typedef typename tfel::meta::IF<isTemporary,
				      typename traits::NumType,
				      const typename traits::NumType&>::type ValueType;
    public:
      TFEL_MATH_INLINE 
      ValueType
      operator()(const unsigned int i,
		 const unsigned int j,
		 const unsigned int k) const
      {
	return static_cast<const T&>(*this).operator()(i,j,k);
      }
    };
    
  } // end of namespace math

} // end of namespace tfel

#endif /* _TFEL_MATH_ARRAY_CONCEPT_IMPL_ */