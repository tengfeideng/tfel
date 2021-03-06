/*!
 * \file   include/TFEL/Math/ST2toT2/st2tot2ResultType.hxx
 * \brief  this file implements partial specialisations of the
 * ResultType class for st2tot2.
 * \author Thomas Helfer
 * \date   19 November 2013
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_TFEL_MATH_ST2TOT2_RESULT_TYPE_HXX
#define LIB_TFEL_MATH_ST2TOT2_RESULT_TYPE_HXX 

#include"TFEL/Math/st2tot2.hxx"
#include"TFEL/Math/Forward/stensor.hxx"
#include"TFEL/Math/Forward/t2tot2.hxx"
#include"TFEL/Math/Forward/t2tost2.hxx"
#include"TFEL/Math/Forward/st2tost2.hxx"

namespace tfel{
  
  namespace math {

    /*
     * Partial specialisation for negation
     */
    template<unsigned short N,typename T>
    class UnaryResultType<st2tot2<N,T>,OpNeg>
    {
      typedef typename UnaryResultType<T,OpNeg>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };
    
    /*!
     * \brief Partial specialisation for st2tot2 and 
     * scalars operations
     * \see   ResultType_
     */
    template<unsigned short N, typename T,typename Scal>
    class ResultType_<ST2toT2Tag,ScalarTag,st2tot2<N,T>,Scal,OpMult>
    {
      typedef typename ResultType<T,Scal,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
    				      tfel::meta::InvalidType,
    				      st2tot2<N,ResBase_> >::type type;
    };

    /*!
     * \brief Partial specialisation for st2tot2 and 
     * scalars operations
     * \see   ResultType_
     */
    template<unsigned short N, typename T,typename Scal>
    class ResultType_<ST2toT2Tag,ScalarTag,st2tot2<N,T>,Scal,OpDiv>
    {
      typedef typename ResultType<T,Scal,OpDiv>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };
    
    /*!
     * \brief Partial specialisation for st2tot2 and 
     * scalars operations
     * \see   ResultType_
     */
    template<typename Scal,unsigned short N,typename T>
    class ResultType_<ScalarTag,ST2toT2Tag,Scal,st2tot2<N,T>,OpMult>
    {
      typedef typename ResultType<Scal,T,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };

    /*!
     * \brief Partial specialisation for st2tot2
     * \see   ResultType
     */
    template<unsigned short N,typename T,typename T2>
    class ResultType<st2tot2<N,T>,st2tot2<N,T2>,OpPlus>
    {
      typedef typename ResultType<T,T2,OpPlus>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };

    /*!
     * \brief Partial specialisation for st2tot2
     * \see   ResultType
     */
    template<unsigned short N,typename T,typename T2>
    class ResultType<st2tot2<N,T>,st2tot2<N,T2>,OpMinus>
    {
      typedef typename ResultType<T,T2,OpMinus>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };

    /*!
     * \brief Partial specialisation for stensor
     * \see   ResultType
     */
    template<unsigned short N,typename T,typename T2>
    class ResultType<tensor<N,T>,stensor<N,T2>,OpDiadicProduct>
    {
      typedef typename ResultType<T,T2,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };

    template<unsigned short N,typename T,typename T2>
    class ResultType<t2tot2<N,T>,st2tot2<N,T2>,OpMult>
    {
      typedef typename ResultType<T,T2,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };

    template<unsigned short N,typename T,typename T2>
    class ResultType<st2tot2<N,T>,t2tost2<N,T2>,OpMult>
    {
      typedef typename ResultType<T,T2,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      t2tot2<N,ResBase_> >::type type;
    };

    template<unsigned short N,typename T,typename T2>
    class ResultType<st2tot2<N,T>,st2tost2<N,T2>,OpMult>
    {
      typedef typename ResultType<T,T2,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tot2<N,ResBase_> >::type type;
    };

    template<unsigned short N,typename T,typename T2>
    class ResultType<t2tost2<N,T>,st2tot2<N,T2>,OpMult>
    {
      typedef typename ResultType<T,T2,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
				      tfel::meta::InvalidType,
				      st2tost2<N,ResBase_> >::type type;
    };

    /*!
     * \brief Partial specialisation for st2tot2 and stensor multiplication
     * \see   ResultType
     */
    template<unsigned short N,typename T,typename T2>
    class ResultType<st2tot2<N,T>,stensor<N,T2>,OpMult>
    {
      typedef typename ResultType<T,T2,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
    				      tfel::meta::InvalidType,
    				      tensor<N,ResBase_> >::type type;
    };

    /*!
     * \brief Partial specialisation for tensor and st2tot2 multiplication
     * \see   ResultType
     */
    template<unsigned short N,typename T,typename T2>
    class ResultType<tensor<N,T2>,st2tot2<N,T>,OpMult>
    {
      typedef typename ResultType<T2,T,OpMult>::type ResBase_;
    public:
      typedef typename std::conditional<tfel::typetraits::IsInvalid<ResBase_>::cond,
    				      tfel::meta::InvalidType,
    				      stensor<N,ResBase_> >::type type;
    };


  } // end of namespace math

} // end of namespace tfel
  
#endif /* LIB_TFEL_MATH_ST2TOT2_RESULT_TYPE_HXX */

