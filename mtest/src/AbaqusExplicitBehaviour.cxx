/*! 
 * \file  mfront/mtest/AbaqusExplicitBehaviour.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 07 avril 2013
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<cmath>
#include<sstream>
#include<algorithm>

#include"TFEL/Math/tensor.hxx"
#include"TFEL/Math/tmatrix.hxx"
#include"TFEL/Math/stensor.hxx"
#include"TFEL/Math/Matrix/tmatrixIO.hxx"
#include"TFEL/Math/Stensor/StensorConceptIO.hxx"
#include"TFEL/Math/Tensor/TensorConceptIO.hxx"
#include"TFEL/Math/Matrix/tmatrixIO.hxx"
#include"TFEL/Math/st2tost2.hxx"
#include"TFEL/System/ExternalLibraryManager.hxx"
#include"MFront/Abaqus/Abaqus.hxx"
#include"MFront/Abaqus/AbaqusComputeStiffnessTensor.hxx"

#include"MTest/CurrentState.hxx"
#include"MTest/BehaviourWorkSpace.hxx"
#include"MTest/UmatNormaliseTangentOperator.hxx"
#include"MTest/AbaqusExplicitBehaviour.hxx"

namespace mtest
{

  AbaqusExplicitBehaviour::AbaqusExplicitBehaviour(const Hypothesis h,
						   const std::string& l,
						   const std::string& b)
    : UmatBehaviourBase(h,l,b)
  {
    typedef tfel::system::ExternalLibraryManager ELM;
    auto& elm = ELM::getExternalLibraryManager();
    this->fct = elm.getAbaqusExplicitExternalBehaviourFunction(l,b);
    this->mpnames = elm.getUMATMaterialPropertiesNames(l,b,this->hypothesis);
    const auto eo = elm.getUMATRequiresStiffnessTensor(l,b,this->hypothesis);
    const auto to = elm.getUMATRequiresThermalExpansionCoefficientTensor(l,b,this->hypothesis);
    const auto etype = elm.getUMATElasticSymmetryType(l,b);
    auto tmp = std::vector<std::string>{};
    tmp.emplace_back("MassDensity");
    if(etype==0u){
      if(eo){
	tmp.insert(tmp.end(),{"YoungModulus","PoissonRatio"});
      }
      if(to){
	tmp.push_back("ThermalExpansion");
      }
    } else if(etype==1u){
      if((h==ModellingHypothesis::PLANESTRESS)||
	 (h==ModellingHypothesis::PLANESTRAIN)||
	 (h==ModellingHypothesis::AXISYMMETRICAL)||
	 (h==ModellingHypothesis::GENERALISEDPLANESTRAIN)){
	if(eo){
	  tmp.insert(tmp.end(),
		     {"YoungModulus1","YoungModulus2","YoungModulus3",
		      "PoissonRatio12","PoissonRatio23","PoissonRatio13"
		      "ShearModulus12"});
	}
	if(to){
	  tmp.insert(tmp.end(),
		     {"ThermalExpansion1","ThermalExpansion2","ThermalExpansion3"});
	}
      } else if(h==ModellingHypothesis::TRIDIMENSIONAL){
	if(eo){
	  tmp.insert(tmp.end(),
		     {"YoungModulus1","YoungModulus2","YoungModulus3",
		      "PoissonRatio12","PoissonRatio23","PoissonRatio13"
		      "ShearModulus12","ShearModulus23","ShearModulus13"});
	}
	if(to){
	  tmp.insert(tmp.end(),
		     {"ThermalExpansion1","ThermalExpansion2","ThermalExpansion3"});
	}
      } else { 
	throw(std::runtime_error("AbaqusExplicitBehaviour::AbaqusExplicitBehaviour : "
				 "unsupported modelling hypothesis"));
      }
    } else {
      throw(std::runtime_error("AbaqusExplicitBehaviour::AbaqusExplicitBehaviour : "
			       "unsupported behaviour type "
			       "(neither isotropic nor orthotropic)"));
    }
    this->mpnames.insert(this->mpnames.begin(),tmp.begin(),tmp.end());
  }

  void
  AbaqusExplicitBehaviour::getDrivingVariablesDefaultInitialValues(tfel::math::vector<real>& v) const
  {
    std::fill(v.begin(),v.end(),real(0));
    v[0] = v[1] = v[2] = real(1);
  }
  
  tfel::math::tmatrix<3u,3u,real>
  AbaqusExplicitBehaviour::getRotationMatrix(const tfel::math::vector<real>&,
					     const tfel::math::tmatrix<3u,3u,real>& r) const
  {
    return r;
  } // end of AbaqusExplicitBehaviour::getRotationMatrix

  void
  AbaqusExplicitBehaviour::allocate(BehaviourWorkSpace& wk,
				    const Hypothesis h) const
  {
    const auto ndv     = this->getDrivingVariablesSize(h);
    const auto nth     = this->getThermodynamicForcesSize(h);
    const auto nstatev = this->getInternalStateVariablesSize(h);
    wk.D.resize(nth,nth);
    wk.kt.resize(nth,ndv);
    wk.k.resize(nth,ndv);
    wk.mps.resize(this->mpnames.size()==0 ? 1u : this->mpnames.size(),real(0));
    wk.evs.resize(this->evnames.size());
    mtest::allocate(wk.cs,*this,h);
  } // end of AbaqusExplicitBehaviour::allocate

  StiffnessMatrixType
  AbaqusExplicitBehaviour::getDefaultStiffnessMatrixType(void) const
  {
    return StiffnessMatrixType::ELASTIC;
  }

  bool
  AbaqusExplicitBehaviour::doPackagingStep(CurrentState& s,
					   BehaviourWorkSpace& wk,
					   const Hypothesis h) const{
    using tfel::math::matrix;
    using abaqus::AbaqusInt;
    const auto ndv     = this->getDrivingVariablesSize(h);
    const auto nth = this->getThermodynamicForcesSize(h);
    matrix<real> K;
    if((h==ModellingHypothesis::PLANESTRESS)||
       (h==ModellingHypothesis::AXISYMMETRICAL)||
       (h==ModellingHypothesis::PLANESTRAIN)){
      K.resize(4u,4u,0);
    } else if(h==ModellingHypothesis::TRIDIMENSIONAL){
      K.resize(6u,6u,0);
    } else {
      throw(std::runtime_error("AbaqusExplicitBehaviour::doPackagingStep: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }
    const AbaqusInt nblock = 1;
    const AbaqusInt ndir = [&h](){
      switch(h){
      case ModellingHypothesis::PLANESTRESS:
      return 2;
      case ModellingHypothesis::AXISYMMETRICAL:
      case ModellingHypothesis::PLANESTRAIN:
      case ModellingHypothesis::TRIDIMENSIONAL:
      return 3;
      }
      throw(std::runtime_error("AbaqusExplicitBehaviour::doPackagingStep: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }();
    const AbaqusInt nshr = [&h](){
      switch(h){
      case ModellingHypothesis::PLANESTRESS:
      case ModellingHypothesis::AXISYMMETRICAL:
      case ModellingHypothesis::PLANESTRAIN:
      return 1;
      case ModellingHypothesis::TRIDIMENSIONAL:
      return 3;
      }
      throw(std::runtime_error("AbaqusExplicitBehaviour::doPackagingStep: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }();
    const auto nprops  = s.mprops1.size() == 1 ? 1 : static_cast<AbaqusInt>(s.mprops1.size())-1;
    const auto nstatv  = static_cast<AbaqusInt>(s.iv1.size());
    const auto nfieldv = static_cast<AbaqusInt>(s.esv0.size())-1;
    const auto density   = s.mprops1[0];
    const real stepTime  = 0;
    const real totalTime = 0;
    const real dt = 0;
    // using a local copy of material properties to handle the
    // case where s.mprops1 is empty
    copy(s.mprops1.begin()+1,s.mprops1.end(),wk.mps.begin());
    if(s.mprops1.size()==1){
      wk.mps[0] = real(0);
    }
    for(decltype(wk.evs.size()) i=0;i!=wk.evs.size();++i){
      wk.evs[i] = s.esv0(i)+s.desv(i);
    }
    std::copy(s.iv0.begin(),s.iv0.end(),s.iv1.begin());
    const real stressOld[6u] = {0,0,0,0,0,0};
    real stressNew[6u] = {0,0,0,0,0,0};
    const real stretchOld[6u] = {1,1,1,0,0,0};
    const real stretchNew[6u] = {1,1,1,0,0,0};
    const real defgradOld[9u]  = {1,1,1,0,0,0,0,0,0};
    const real defgradNew[9u]  = {1,1,1,0,0,0,0,0,0};
    const real enerInternOld = 0;
    const real enerInelasOld = 0;
    real enerInternNew = 0;
    real enerInelasNew = 0;
    for(AbaqusInt i=0;i!=ndir+nshr;++i){
      real strainInc[6u] = {0,0,0,0,0,0};
      strainInc[i] = 1;
      (this->fct)(&nblock,&ndir,&nshr,&nstatv,&nfieldv,&nprops,
		  nullptr,&stepTime,&totalTime,&dt,nullptr,
		  nullptr,nullptr,
		  &(wk.mps[0]),&density,
		  strainInc,nullptr,&(s.esv0[0]),stretchOld,
		  defgradOld,
		  s.esv0.size()==1 ? nullptr : &(s.esv0[0])+1,
		  stressOld,
		  s.iv0.empty() ? nullptr : &(s.iv0[0]),
		  &enerInternOld,&enerInelasOld,
		  &(wk.evs[0]),stretchNew,defgradNew,
		  wk.evs.size()==1 ? nullptr : &(wk.evs[0])+1,
		  stressNew,
		  s.iv1.empty()? nullptr : &(s.iv1[0]),
		  &enerInternNew,&enerInelasNew,0);
      if(h==ModellingHypothesis::PLANESTRESS){
	const auto idx = (i==2) ? 3 : i;
	K(0,idx) = stressNew[0];
	K(1,idx) = stressNew[1];
	K(2,idx) = 0;
	K(3,idx) = stressNew[2];
      } else if((h==ModellingHypothesis::AXISYMMETRICAL)||
		(h==ModellingHypothesis::PLANESTRAIN)){
	K(0,i) = stressNew[0];
	K(1,i) = stressNew[1];
	K(2,i) = stressNew[2];
	K(3,i) = stressNew[3];
      } else if(h==ModellingHypothesis::TRIDIMENSIONAL){
	K(0,i) = stressNew[0];
	K(1,i) = stressNew[1];
	K(2,i) = stressNew[2];
	K(3,i) = stressNew[3];
	K(4,i) = stressNew[5];
	K(5,i) = stressNew[4];
      } else {
	throw(std::runtime_error("AbaqusExplicitBehaviour::doPackagingStep: "
				 "unsupported hypothesis ("+
				 ModellingHypothesis::toString(h)+")"));
      }
    }
    // convertion to TFEL conventions and storage in packaging
    // information
    const std::string n = "InitialElasticStiffness";
    s.packaging_info.insert({n,matrix<real>()});
    auto& m = s.packaging_info.at(n).get<matrix<real>>();
    const real cste = std::sqrt(real{2});
    if((h==ModellingHypothesis::PLANESTRESS)||(h==ModellingHypothesis::AXISYMMETRICAL)||
       (h==ModellingHypothesis::PLANESTRAIN)){
      m.resize(4u,5u);
    } else if(h==ModellingHypothesis::TRIDIMENSIONAL){
      m.resize(6u,9u);
    } else {
      throw(std::runtime_error("AbaqusExplicitBehaviour::computePredictionOperator: "
			       "no 'InitialElasticStiffness' found. "
			       "Was the packaging step done ?"));
    }
#pragma message("AbaqusExplicitBehaviour::computePredictionOperator: finish filling final stiffness matrix")    
    for(unsigned short i=0;i!=3;++i){
      for(unsigned short j=0;j!=3;++j){
	m(i,j)=K(i,j);
      }
    }
    return true;
  }
  
  std::pair<bool,real>
  AbaqusExplicitBehaviour::computePredictionOperator(BehaviourWorkSpace& wk,
						     const CurrentState& s,
						     const Hypothesis,
						     const StiffnessMatrixType ktype) const
  {
    if(ktype==StiffnessMatrixType::ELASTIC){
      const auto p = s.packaging_info.find("InitialElasticStiffness");
      if(p==s.packaging_info.end()){
	throw(std::runtime_error("AbaqusExplicitBehaviour::computePredictionOperator: "
				 "no 'InitialElasticStiffness' found. "
				 "Was the packaging step done ?"));
      }
      wk.kt = p->second.get<tfel::math::matrix<real>>();
      return {true,real{1}};
    }
    return {false,real(-1)};
  }

  std::pair<bool,real>
  AbaqusExplicitBehaviour::integrate(CurrentState& s,
				     BehaviourWorkSpace& wk,
				     const Hypothesis h,
				     const real dt,
				     const StiffnessMatrixType ktype) const
  {
    using namespace tfel::math;
    using abaqus::AbaqusInt;
    static const real sqrt2 = std::sqrt(real(2));
    if(ktype==StiffnessMatrixType::ELASTIC){
      const auto p = s.packaging_info.find("InitialElasticStiffness");
      if(p==s.packaging_info.end()){
	throw(std::runtime_error("AbaqusExplicitBehavi8our::integrate: "
				 "no 'InitialElasticStiffness' found. "
				 "Was the packaging step done ?"));
      }
      wk.k = p->second.get<tfel::math::matrix<real>>();
    } else {
      throw(std::runtime_error("AbaqusExplicitBehaviour::integrate: "
			       "unsupported stiffness matrix type"));
    }
    const AbaqusInt nblock = 1;
    const AbaqusInt ndir = [&h](){
      switch(h){
      case ModellingHypothesis::PLANESTRESS:
      return 2;
      case ModellingHypothesis::AXISYMMETRICAL:
      case ModellingHypothesis::PLANESTRAIN:
      case ModellingHypothesis::TRIDIMENSIONAL:
      return 3;
      }
      throw(std::runtime_error("AbaqusExplicitBehaviour::integrate: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }();
    const AbaqusInt nshr = [&h](){
      switch(h){
      case ModellingHypothesis::PLANESTRESS:
      case ModellingHypothesis::AXISYMMETRICAL:
      case ModellingHypothesis::PLANESTRAIN:
      return 1;
      case ModellingHypothesis::TRIDIMENSIONAL:
      return 3;
      }
      throw(std::runtime_error("AbaqusExplicitBehaviour::integrate: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }();
    const auto nprops  = s.mprops1.size() == 1 ? 1 : static_cast<AbaqusInt>(s.mprops1.size())-1;
    const auto nstatv  = static_cast<AbaqusInt>(s.iv0.size());
    const auto nfieldv = static_cast<AbaqusInt>(s.esv0.size())-1;
    const auto density   = s.mprops1[0];
    const real stepTime  = 1;
    const real totalTime = 1;
    // using a local copy of material properties to handle the
    // case where s.mprops1 is empty
    copy(s.mprops1.begin()+1,s.mprops1.end(),wk.mps.begin());
    if(s.mprops1.size()==1){
      wk.mps[0] = real(0);
    }
    for(decltype(wk.evs.size()) i=0;i!=wk.evs.size();++i){
      wk.evs[i] = s.esv0(i)+s.desv(i);
    }
    std::copy(s.iv0.begin(),s.iv0.end(),s.iv1.begin());
    const real strainInc[6u] = {0,0,0,0,0,0};
    real stressOld[6u]       = {0,0,0,0,0,0};
    real stressNew[6u]       = {0,0,0,0,0,0};
    real stretchOld[6u]      = {1,1,1,0,0,0};
    real stretchNew[6u]      = {1,1,1,0,0,0};
    real defgradOld[9u]      = {1,1,1,0,0,0,0,0,0};
    real defgradNew[9u]      = {1,1,1,0,0,0,0,0,0};
    const real enerInternOld = 0;
    const real enerInelasOld = 0;
    real enerInternNew = 0;
    real enerInelasNew = 0;
    // rotation matrix
    tmatrix<3u,3u,real> r0;
    tmatrix<3u,3u,real> r1;
    if((h==ModellingHypothesis::PLANESTRESS)||
       (h==ModellingHypothesis::AXISYMMETRICAL)||
       (h==ModellingHypothesis::PLANESTRAIN)){
      tensor<2u,real>  F0(&s.e0[0]);
      tensor<2u,real>  F1(&s.e1[0]);
      tensor<2u,real>  R0;
      tensor<2u,real>  R1;
      stensor<2u,real> U0;
      stensor<2u,real> U1;
      stensor<2u,real> s0(&(s.s0[0]));
      polar_decomposition(R0,U0,F0);
      polar_decomposition(R1,U1,F1);
      tfel::fsalgo::copy<5u>::exe(F0.begin(),defgradOld);
      tfel::fsalgo::copy<5u>::exe(F1.begin(),defgradNew);
      U0.exportTab(stretchOld);
      U1.exportTab(stretchNew);
      r0 = matrix_view(R0);
      r1 = matrix_view(R1);
      s0.changeBasis(r1);
      s0.exportTab(stressOld);
      s0.exportTab(stressNew);
    } else if(h==ModellingHypothesis::TRIDIMENSIONAL){
      tensor<3u,real>  F0(&s.e0[0]);
      tensor<3u,real>  F1(&s.e1[0]);
      tensor<3u,real>  R0;
      tensor<3u,real>  R1;
      stensor<3u,real> U0;
      stensor<3u,real> U1;
      stensor<3u,real> s0(&(s.s0[0]));
      polar_decomposition(R0,U0,F0);
      polar_decomposition(R1,U1,F1);
      tfel::fsalgo::copy<9u>::exe(F0.begin(),defgradOld);
      tfel::fsalgo::copy<9u>::exe(F1.begin(),defgradNew);
      U0.exportTab(stretchOld);
      U1.exportTab(stretchNew);
      std::swap(stretchOld[4],stretchOld[5]);
      std::swap(stretchNew[4],stretchNew[5]);
      r0 = matrix_view(R0);
      r1 = matrix_view(R1);
      s0.changeBasis(r1);
      s0.exportTab(stressOld);
      s0.exportTab(stressNew);
      std::swap(stressOld[5],stressOld[4]);
      std::swap(stressNew[5],stressNew[4]);
    } else {
      throw(std::runtime_error("AbaqusExplicitBehaviour::integrate: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }
    (this->fct)(&nblock,&ndir,&nshr,&nstatv,&nfieldv,&nprops,
		nullptr,&stepTime,&totalTime,&dt,nullptr,
		nullptr,nullptr,
		&(wk.mps[0]),&density,
		strainInc,nullptr,&(s.esv0[0]),stretchOld,
		defgradOld,
		s.esv0.size()==1 ? nullptr : &(s.esv0[0])+1,
		stressOld,
		s.iv0.empty() ? nullptr : &(s.iv0[0]),
		&enerInternOld,&enerInelasOld,
		&(wk.evs[0]),stretchNew,defgradNew,
		wk.evs.size()==1 ? nullptr : &(wk.evs[0])+1,
		stressNew,
		s.iv1.empty() ? nullptr : &(s.iv1[0]),
		&enerInternNew,&enerInelasNew,0);
    if((h==ModellingHypothesis::PLANESTRESS)||
       (h==ModellingHypothesis::AXISYMMETRICAL)||
       (h==ModellingHypothesis::PLANESTRAIN)){
      stensor<2u,real> sig;
      if(h==ModellingHypothesis::PLANESTRESS){
	sig[0] = stressNew[0];
	sig[1] = stressNew[1];
	sig[2] = 0;
	sig[3] = stressNew[2]*sqrt2;
      } else {
	sig[0] = stressNew[0];
	sig[1] = stressNew[1];
	sig[2] = stressNew[2];
	sig[3] = stressNew[3]*sqrt2;
      }
      sig.changeBasis(transpose(r1));
      tfel::fsalgo::copy<4u>::exe(sig.begin(),s.s1.begin());
    } else if(h==ModellingHypothesis::TRIDIMENSIONAL){
      stensor<3u,real> sig = {stressNew[0],stressNew[1],stressNew[2],
			      stressNew[3]*sqrt2,stressNew[5]*sqrt2,
			      stressNew[4]*sqrt2};
      sig.changeBasis(transpose(r1));
      tfel::fsalgo::copy<6u>::exe(sig.begin(),s.s1.begin());
    } else {
      throw(std::runtime_error("AbaqusExplicitBehaviour::integrate: "
			       "unsupported hypothesis ("+
			       ModellingHypothesis::toString(h)+")"));
    }
    return {true,real(1)};
  } // end of AbaqusExplicitBehaviour::integrate

  AbaqusExplicitBehaviour::~AbaqusExplicitBehaviour()
  {}
  
} // end of namespace mtest
