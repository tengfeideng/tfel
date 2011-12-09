/*! 
 * \file  stensor.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 19 sept. 2011
 */

#ifndef _LIB_TFEL_MATH_FORWARD_STENSOR_H_
#define _LIB_TFEL_MATH_FORWARD_STENSOR_H_ 

namespace tfel
{

  namespace math
  {

    template<unsigned short N, typename T>
    class StensorStatic;

    /*!
     * \class stensor
     * \brief finite size symmetric tensor.
     * \param N, the spatial dimension, see StensorDimeToSize for details. 
     * \param T, numerical type used, by default, double
     * \param Storage, type of storage used
     * \pre   This class is only defined for N=1u,2u and 3u.
     * \see   StensorDimeToSize and StensorSizeToDime. 
     * \see   StensorStatic, StensorExternalTab,StensorExternalVoigt.
     */
    template<unsigned short N,typename T=double,
	     template<unsigned short,typename> class Storage=StensorStatic>
    class stensor;

  } // end of namespace math

} // end of namespace tfel

#endif /* _LIB_TFEL_MATH_FORWARD_STENSOR_H */
