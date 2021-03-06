/*!
 * \file   include/TFEL/Math/ST2toT2/ST2toT2Concept.hxx  
 * \brief    
 * \author Thomas Helfer
 * \date   19 November 2013
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_TFEL_MATH_ST2TOT2CONCEPT_HXX
#define LIB_TFEL_MATH_ST2TOT2CONCEPT_HXX 1

#include<type_traits>

#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/Metaprogramming/Implements.hxx"
#include"TFEL/Metaprogramming/InvalidType.hxx"
#include"TFEL/Math/General/Abs.hxx"
#include"TFEL/Math/General/ConceptRebind.hxx"
#include"TFEL/Math/Forward/ST2toT2Concept.hxx"

namespace tfel{

  namespace math{

    template<class T>
    struct ST2toT2Traits{
      typedef tfel::meta::InvalidType NumType;
      static constexpr unsigned short dime = 0u;
    };
    //! a simple alias
    template<class T>
    using ST2toT2NumType = typename ST2toT2Traits<T>::NumType;
    /*!
     * \class ST2toT2Tag
     * \brief Helper class to characterise st2tot2.
     */ 
    struct ST2toT2Tag
    {};

    template<class T>
    struct ST2toT2Concept 
    {
      typedef ST2toT2Tag ConceptTag;

      typename ST2toT2Traits<T>::NumType
      operator()(const unsigned short,
		 const unsigned short) const;

    protected:
      ST2toT2Concept() = default;
      ST2toT2Concept(ST2toT2Concept&&) = default;
      ST2toT2Concept(const ST2toT2Concept&) = default;
      ST2toT2Concept&
      operator=(const ST2toT2Concept&) = default;
      ~ST2toT2Concept() = default;
    };

    //! paratial specialisation for symmetric tensors
    template<typename Type>
    struct ConceptRebind<ST2toT2Tag,Type>
    {
      using type = ST2toT2Concept<Type>;
    };

    template<typename T>
    struct ST2toT2Type{
      typedef T type;
    };

    template<typename ST2toT2Type>
    typename std::enable_if<
      tfel::meta::Implements<ST2toT2Type,ST2toT2Concept>::cond,
      typename tfel::typetraits::AbsType<typename ST2toT2Traits<ST2toT2Type>::NumType>::type
    >::type
    abs(const ST2toT2Type&);

  } // end of namespace math

} // end of namespace tfel

#include"TFEL/Math/ST2toT2/ST2toT2Concept.ixx"

#endif /* LIB_TFEL_MATH_ST2TOT2CONCEPT_HXX */
