/*!
 * \file   mfront/src/SupportedTypes.cxx
 * 
 * \brief    
 * \author Helfer Thomas
 * \date   12 Jan 2007
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * re served. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<utility>
#include<sstream>
#include<stdexcept>

#include"MFront/MFrontDebugMode.hxx"
#include"MFront/VariableDescription.hxx"
#include"MFront/SupportedTypes.hxx"

namespace mfront{

  /*!
   * \return a map between type names and Supported::TypeFlags
   */
  static std::map<std::string,SupportedTypes::TypeFlag> &
  SupportedTypes_getFlags()
  {
    using namespace std;
    using TypeFlag = SupportedTypes::TypeFlag;
    static map<string,TypeFlag> flags = {{"real",SupportedTypes::Scalar},
					 {"frequency",SupportedTypes::Scalar},
					 {"stress",SupportedTypes::Scalar},
					 {"length",SupportedTypes::Scalar},
					 {"time",SupportedTypes::Scalar},
					 //    {"stressrate",SupportedTypes::Scalar},
					 {"strain",SupportedTypes::Scalar},
					 {"strainrate",SupportedTypes::Scalar},
					 {"temperature",SupportedTypes::Scalar},
					 {"energy_density",SupportedTypes::Scalar},
					 {"thermalexpansion",SupportedTypes::Scalar},
					 {"massdensity",SupportedTypes::Scalar},
					 {"TVector",SupportedTypes::TVector},
					 {"Stensor",SupportedTypes::Stensor},
					 {"Tensor",SupportedTypes::Tensor},
					 {"StressStensor",SupportedTypes::Stensor},
					 {"StressRateStensor",SupportedTypes::Stensor},
					 {"StrainStensor",SupportedTypes::Stensor},
					 {"StrainRateStensor",SupportedTypes::Stensor},
					 // CZM
					 {"DisplacementTVector",SupportedTypes::TVector},
					 {"ForceTVector",SupportedTypes::TVector},
					 // Finite Strain
					 {"DeformationGradientTensor",SupportedTypes::Tensor}};
    return flags;
  } // end of SupportedTypes_getFlags

  const std::map<std::string,SupportedTypes::TypeFlag>&
  SupportedTypes::getTypeFlags(){
    return SupportedTypes_getFlags();
  } // end of SupportedTypes::getTypeFlags
  
  SupportedTypes::TypeSize::TypeSize() = default;
  
  SupportedTypes::TypeSize::TypeSize(TypeSize&&) = default;

  SupportedTypes::TypeSize::TypeSize(const TypeSize&) = default;
  
  SupportedTypes::TypeSize::TypeSize(const int a,const int b,
				     const int c,const int d)				     
    : scalarSize(a),
      tvectorSize(b),
      stensorSize(c),
      tensorSize(d)
  {}
  
  SupportedTypes::TypeSize&
  SupportedTypes::TypeSize::operator=(TypeSize&&) = default;

  SupportedTypes::TypeSize&
  SupportedTypes::TypeSize::operator=(const TypeSize&) = default;

  SupportedTypes::TypeSize&
  SupportedTypes::TypeSize::operator+=(const TypeSize& rhs)
  {
    this->scalarSize  += rhs.scalarSize;
    this->tvectorSize += rhs.tvectorSize;
    this->stensorSize += rhs.stensorSize;
    this->tensorSize  += rhs.tensorSize;
    return *this;
  }
  
  SupportedTypes::TypeSize&
  SupportedTypes::TypeSize::operator-=(const TypeSize& rhs)
  {
    this->scalarSize  -= rhs.scalarSize;
    this->tvectorSize -= rhs.tvectorSize;
    this->stensorSize -= rhs.stensorSize;
    this->tensorSize  -= rhs.tensorSize;
    return *this;
  }

  int
  SupportedTypes::TypeSize::getValueForDimension(const unsigned short d) const
  {
    switch(d){
    case 1:
      return scalarSize+tvectorSize+3*(stensorSize+tensorSize);
    case 2:
      return scalarSize+2*tvectorSize+4*stensorSize+5*tensorSize;
    case 3:
      return scalarSize+3*tvectorSize+6*stensorSize+9*tensorSize;
    }
    throw(std::runtime_error("SupportedTypes::TypeSize::getValueForDimension : "
			     "invalid type size"));
  } // end of SupportedTypes::TypeSize::getValueForDimension

  int
  SupportedTypes::TypeSize::getValueForModellingHypothesis(const Hypothesis h) const{
    return this->getValueForDimension(tfel::material::getSpaceDimension(h));
  }
  
  std::ostream&
  operator << (std::ostream& os,const SupportedTypes::TypeSize& size)
  {
    bool first = true;
    if(size.scalarSize!=0){
      os << size.scalarSize;
      first = false;
    }
    if(size.tvectorSize!=0){
      if((!first)&&(size.tvectorSize>=0)){
	os << "+";
      }
      if(size.tvectorSize==1){
	os << "TVectorSize";
      } else {
	os << size.tvectorSize << "*TVectorSize";
      }
      first = false;
    }
    if(size.stensorSize!=0){
      if((!first)&&(size.stensorSize>=0)){
	os << "+";
      }
      if(size.stensorSize==1){
	os << "StensorSize";
      } else {
	os << size.stensorSize << "*StensorSize";
      }
      first = false;
    }
    if(size.tensorSize!=0){
      if((!first)&&(size.tensorSize>=0)){
	os << "+";
      }
      if(size.tensorSize==1){
	os << "TensorSize";
      } else {
	os << size.tensorSize << "*TensorSize";
      }
      first = false;
    }
    if(first){
      os << "0";
    }
    return os;
  }
  
  SupportedTypes::SupportedTypes() = default;

  bool SupportedTypes::isSupportedType(const std::string& t) const
  {
    const auto& flags = SupportedTypes_getFlags();
    return flags.find(t)!=flags.end();
  } // end of SupportedTypes::isSupportedType
  
  SupportedTypes::TypeFlag
  SupportedTypes::getTypeFlag(const std::string& name)
  {
    const auto& flags = SupportedTypes_getFlags();
    const auto p = flags.find(name);
    if(p==flags.end()){
      throw(std::runtime_error("SupportedTypes::getTypeTag : "
			       "'"+name+"' is not a supported type."));
    }
    return p->second;
  }

  SupportedTypes::TypeSize SupportedTypes::getTypeSize(const std::string& type,
						       const unsigned short a)
  {
    TypeSize res;
    switch(SupportedTypes::getTypeFlag(type)){
    case Scalar : 
      res=TypeSize(a,0u,0u,0u);
      break;
    case TVector : 
      res=TypeSize(0u,a,0u,0u);
      break;
    case Stensor :
      res=TypeSize(0u,0u,a,0u);
      break;
    case Tensor : 
      res=TypeSize(0u,0u,0u,a);
      break;
    default : 
      throw(std::runtime_error("SupportedTypes::getTypeSize: internal error."));
    }
    return res;
  }

  std::string
  SupportedTypes::getTimeDerivativeType(const std::string& type) const
  {
    if (type=="real"){
      return "frequency";
    } else if(type=="strain"){
      return "strainrate";
    } else if (type=="stress"){
      return "stressrate";
    } else if (type=="Stensor"){
      return "FrequencyStensor";
    } else if (type=="StressStensor"){
      return "StressRateStensor";
    } else if (type=="StrainStensor"){
      return "StrainRateStensor";
    } else if (type=="DeformationGradientTensor"){
      return "DeformationGradientRateTensor";
    } else {
      throw(std::runtime_error("SupportedTypes::getTimeDerivativeType: "
			       "internal error, unsupported type '"+type+"'"));
    }
  }

  int SupportedTypes::TypeSize::getScalarSize() const
  {
    return this->scalarSize;
  }

  int SupportedTypes::TypeSize::getTVectorSize() const
  {
    return this->tvectorSize;
  }
    
  int SupportedTypes::TypeSize::getStensorSize() const
  {
    return this->stensorSize;
  }

  int SupportedTypes::TypeSize::getTensorSize() const
  {
    return this->tensorSize;
  }

  bool SupportedTypes::TypeSize::operator==(const TypeSize& rhs) const
  {
    return ((this->getScalarSize()==rhs.getScalarSize())&&
	    (this->getStensorSize()==rhs.getStensorSize())&&
	    (this->getTVectorSize()==rhs.getTVectorSize())&&
	    (this->getTensorSize()==rhs.getTensorSize()));
  } // end of SupportedTypes::TypeSize::operator==

  bool SupportedTypes::TypeSize::operator!=(const TypeSize& rhs) const
  {
    return !this->operator==(rhs);
  } // end of SupportedTypes::TypeSize::operator!=

  bool SupportedTypes::TypeSize::isNull() const
  {
    return ((this->getScalarSize()==0)  && (this->getStensorSize()==0)&&
	    (this->getTVectorSize()==0) && (this->getTensorSize()==0));
  } // end of SupportedTypes::TypeSize::isNull

  SupportedTypes::~SupportedTypes() = default;

} // end of namespace mfront
