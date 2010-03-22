/*! 
 * \file  Kriging.ixx
 * \brief
 * \author Helfer Thomas
 * \brief 10 avr 2009
 */

#ifndef _LIB_TFEL_MATH_KRIGING_IXX__
#define _LIB_TFEL_MATH_KRIGING_IXX_ 

#include<algorithm>

#include"TFEL/Math/matrix.hxx"
#ifdef HAVE_ATLAS
#include"TFEL/Math/Bindings/atlas.hxx"
#else
#include"TFEL/Math/LUSolve.hxx"
#endif
#include"TFEL/Math/Kriging/KrigingErrors.hxx"

namespace tfel
{

  namespace math
  {

    namespace internals
    {
      
      template<unsigned short n,
	       unsigned short nb,
	       unsigned short N,
	       typename T,
	       typename Model>
      struct ApplySpecificationDrifts
      {
	static void
	apply(matrix<T>& m,
	      const typename tfel::math::vector<typename KrigingVariable<N,T>::type>::size_type n0,
	      const tfel::math::vector<typename KrigingVariable<N,T>::type>& x)
	{
	  typename matrix<T>::size_type i;
	  for(i=0;i!=x.size();++i){
	    m(n0+n,i) = m(i,n0+n) = (Model::drifts[n])(x[i]);
	  }
	  ApplySpecificationDrifts<n+1,nb,N,T,Model>::apply(m,n0,x);
	}
	static void
	apply(T& r,
	      typename tfel::math::vector<T>::const_iterator& pa,
	      const typename KrigingVariable<N,T>::type& xv)
	{
	  r+=(*pa)*(Model::drifts[n])(xv);
	  ApplySpecificationDrifts<n+1,nb,N,T,Model>::apply(r,++pa,xv);
	}
      }; // end of ApplySpecificationFunction
    
      template<unsigned short nb,
	       unsigned short N,
	       typename T,
	       typename Model>
      struct ApplySpecificationDrifts<nb,nb,N,T,Model>
      {
	static void
	apply(matrix<T>&,
	      const typename tfel::math::vector<typename KrigingVariable<N,T>::type>::size_type,
	      const tfel::math::vector<typename KrigingVariable<N,T>::type >&)
	{}
	static void
	apply(T&,
	      typename tfel::math::vector<T>::const_iterator&,
	      const typename KrigingVariable<N,T>::type&)
	{}
      }; // end of ApplySpecificationDrifts

    } // end of namespace internals

    template<unsigned short N,
	     typename T,
	     typename Model>
    Kriging<N,T,Model>::Kriging()
    {} // end of Kriging<N,T,Model>::Kriging()

    template<unsigned short N,
	     typename T,
	     typename Model>
    T
    Kriging<N,T,Model>::operator()(const typename KrigingVariable<N,T>::type& xv) const
    {
      using namespace tfel::math;
      using namespace tfel::math::internals;
      typename vector<T>::size_type i;
      typename vector<T>::const_iterator p = a.begin()+this->x.size();
      T r(0);
      for(i=0;i!=this->x.size();++i){
	r+=a[i]*Model::covariance(xv-this->x[i]);
      }
      ApplySpecificationDrifts<0,Model::nb,N,T,Model>::apply(r,p,xv);
      return r;
    } // end of Kriging<N,T,Model>::operator()

    template<unsigned short N,
	     typename T,
	     typename Model>
    void
    Kriging<N,T,Model>::addValue(const typename KrigingVariable<N,T>::type& xv,
				 const T& fv)
    {
      using namespace tfel::math;
      this->x.push_back(xv);
      this->f.push_back(fv);
    }

    template<unsigned short N,
	     typename T,
	     typename Model>
    void
    Kriging<N,T,Model>::buildInterpolation(void)
    {
      using namespace std;
      using namespace tfel::math;
      using namespace tfel::math::internals;
#ifdef HAVE_ATLAS
      using namespace tfel::math::atlas;
#endif
      using tfel::math::vector;
      typename vector<T>::size_type i;
      typename vector<T>::size_type j;
      if(x.size()!=f.size()){
	throw(KrigingErrorInvalidLength());
      }
      if(x.empty()){
	throw(KrigingErrorNoDataSpecified());
      }
      if(x.size()<=Model::nb){
	throw(KrigingErrorInsufficientData());
      }
      matrix<T> m(x.size()+Model::nb,
		  x.size()+Model::nb,T(0));       // temporary matrix
      this->a.resize(x.size()+Model::nb,T(0));    // unknowns
      copy(this->f.begin(),this->f.end(),this->a.begin()); // copy values
      // filling the matrix
      for(i=0;i!=x.size();++i){
	for(j=0;j!=i;++j){
	  m(i,j) = m(j,i) = Model::covariance(this->x[i]-this->x[j]);
	}
	m(i,i) = Model::nuggetEffect(i,x[i]);
      }
      ApplySpecificationDrifts<0,Model::nb,N,T,Model>::apply(m,this->x.size(),this->x);
      // computing unknown values
#ifdef HAVE_ATLAS
      gesv(m,this->a);
#else
      LUSolve::exe(m,this->a);
#endif
    }

  } // end of namespace math

} // end of namespace tfel

#endif /* _LIB_TFEL_MATH_KRIGING_HXX */
