/*! 
 * \file  mfront/mtest/MTest.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 12 avril 2013
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */


#include<map>
#include<cmath>
#include<string>
#include<vector>
#include<memory>
#include<sstream>
#include<cstdlib>
#include<iterator>
#include<algorithm>
#include<stdexcept>

#include"tfel-config.hxx"
#include"TFEL/Utilities/TextData.hxx"
#include"TFEL/Utilities/ArgumentParserBase.hxx"
#include"TFEL/Utilities/TerminalColors.hxx"
#include"TFEL/Math/matrix.hxx"
#include"TFEL/Math/vector.hxx"
#include"TFEL/Math/tvector.hxx"
#include"TFEL/Math/stensor.hxx"
#include"TFEL/Math/tmatrix.hxx"

#include"MFront/MFrontLogStream.hxx"

#include"MTest/MTest.hxx"
#include"MTest/Behaviour.hxx"
#include"MTest/MTestParser.hxx"

#include"MTest/AnalyticalTest.hxx"
#include"MTest/ReferenceFileComparisonTest.hxx"

#include"MTest/Evolution.hxx"
#include"MTest/FunctionEvolution.hxx"
#include"MTest/CastemEvolution.hxx"

#include"MTest/Constraint.hxx"
#include"MTest/ImposedThermodynamicForce.hxx"
#include"MTest/ImposedDrivingVariable.hxx"

#ifdef min
#undef min
#endif

namespace mtest
{

  static unsigned short
  getStensorSize(const unsigned short d)
  {
    if(d==1){
      return 3;
    } else if(d==2){
      return 4;
    } else if(d==3){
      return 6;
    }
    throw(std::runtime_error("mtest::getStensorSize: "
			     "invalid dimenstion"));
  }

  static unsigned short
  getTensorSize(const unsigned short d)
  {
    if(d==1){
      return 3;
    } else if(d==2){
      return 5;
    } else if(d==3){
      return 9;
    }
    throw(std::runtime_error("mtest::getTensorSize: "
			     "invalid dimenstion"));
  }

  MTest::MTestCurrentState::MTestCurrentState() = default;
  MTest::MTestCurrentState::MTestCurrentState(const MTestCurrentState&) = default;
  MTest::MTestCurrentState::MTestCurrentState(MTestCurrentState&&)      = default;
  MTest::MTestCurrentState::~MTestCurrentState() noexcept = default;

  MTest::MTestWorkSpace::MTestWorkSpace() = default;
  MTest::MTestWorkSpace::~MTestWorkSpace() noexcept =  default;

  MTest::UTest::~UTest()
  {}

  MTest::MTest()
    : oprec(-1),
      rprec(-1),
      rm(real(0)),
      isRmDefined(false),
      eeps(-1.),
      seps(-1.),
      handleThermalExpansion(true),
      toeps(-1),
      pv(-1),
      cto(false)
  {}

  void MTest::readInputFile(const std::string& f){
    MTestParser p;
    p.execute(*this,f);
  } // end of MTest::readInputFile

  void
  MTest::setHandleThermalExpansion(const bool b1)
  {
    if(!this->handleThermalExpansion){
      throw(std::runtime_error("MTest::setHandleThermalExpansion: "
			       "thermal expansion is not handled"));
    }
    this->handleThermalExpansion = b1;
  }

  std::string
  MTest::name(void) const
  {
    if(this->tname.empty()){
      return "unit behaviour test";
    }
    return this->tname;
  } // end of MTest::name
  
  std::string
  MTest::classname(void) const
  {
    return "MTest";
  }

  void
  MTest::addConstraint(const std::shared_ptr<Constraint> c)
  {
    this->constraints.push_back(c);
  }

  void
  MTest::setDrivingVariableEpsilon(const double e)
  {
    using namespace std;
    if(this->eeps>0){
      string msg("MTest::setDrivingVariableEpsilon : the epsilon "
    		 "value has already been declared");
      throw(runtime_error(msg));
    }
    if(e < 100*numeric_limits<real>::min()){
      string msg("MTest::setDrivingVariableEpsilon : invalid value");
      throw(runtime_error(msg));
    }
    this->eeps = e;
  }

  void
  MTest::setThermodynamicForceEpsilon(const double s)
  {
    using namespace std;
      if(this->seps>0){
      string msg("MTest::setThermodynamicForceEpsilon : the epsilon "
    		 "value has already been declared");
      throw(runtime_error(msg));
    }
    if(s < 100*numeric_limits<real>::min()){
      string msg("MTest::setThermodynamicForceEpsilon : invalid value");
      throw(runtime_error(msg));
    }
    this->seps = s;
  }

  void
  MTest::setRotationMatrix(const tfel::math::tmatrix<3u,3u,real>& r,
			   const bool bo)
  {
    using namespace tfel::math;
    if((this->isRmDefined)&&(!bo)){
      throw(std::runtime_error("MTest::setRotationMatrix: "
			       "rotation matrix already defined"));
    }
    if(this->b.get()==nullptr){
      throw(std::runtime_error("MTest::setRotationMatrix: "
			       "no behaviour defined"));
    }
    if(this->b->getSymmetryType()!=1){
      throw(std::runtime_error("MTest::setRotationMatrix: "
			       "rotation matrix may only be defined "
			       "for orthotropic behaviours"));
    }
    this->isRmDefined = true;
    // checking that the given matrix is a rotation one
    const tvector<3u,real> c0 = r.column_view<0>();
    const tvector<3u,real> c1 = r.column_view<1>();
    const tvector<3u,real> c2 = r.column_view<2>();
    if((abs(norm(c0)-real(1))>100*std::numeric_limits<real>::epsilon())||
       (abs(norm(c1)-real(1))>100*std::numeric_limits<real>::epsilon())||
       (abs(norm(c2)-real(1))>100*std::numeric_limits<real>::epsilon())){
      throw(std::runtime_error("MTest::setRotationMatrix: "
			       "at least one column is not normalised"));
    }
    if((abs(c0|c1)>100*std::numeric_limits<real>::epsilon())||
       (abs(c0|c2)>100*std::numeric_limits<real>::epsilon())||
       (abs(c1|c2)>100*std::numeric_limits<real>::epsilon())){
      throw(std::runtime_error("MTest::setRotationMatrix : "
			       "at least two columns are not orthogonals"));
    }
    this->rm = r;
  }

  void
  MTest::getVariableTypeAndPosition(UTest::TestedVariable& type,
				    unsigned short& pos,
				    const std::string& n)
  {
    if(this->b.get()==nullptr){
      throw(std::runtime_error("MTest::getVariableTypeAndPosition : "
			       "no behaviour defined"));
    }
    const auto enames = this->b->getDrivingVariablesComponents(this->hypothesis);
    const auto snames = this->b->getThermodynamicForcesComponents(this->hypothesis);
    auto p=find(enames.begin(),enames.end(),n);
    if(p!=enames.end()){
      pos  = static_cast<unsigned short>(p-enames.begin());
      type = MTest::UTest::DRIVINGVARIABLE;
      return;
    } 
    p=find(snames.begin(),snames.end(),n);
    if(p!=snames.end()){
      pos  = static_cast<unsigned short>(p-snames.begin());
      type = MTest::UTest::THERMODYNAMICFORCE;
      return;
    } 
    p=find(this->ivfullnames.begin(),this->ivfullnames.end(),n);
    if(p!=this->ivfullnames.end()){
      pos  = static_cast<unsigned short>(p-this->ivfullnames.begin());
      type = MTest::UTest::INTERNALSTATEVARIABLE;
      return;
    } 
    throw(std::runtime_error("MTest::getVariableTypeAndPosition : "
			     "no variable name '"+n+"'"));
  } // end of MTest::getVariableTypeAndPosition

  void
  MTest::setDefaultHypothesis(void)
  {
    typedef tfel::material::ModellingHypothesis MH;
    if(this->hypothesis!=MH::UNDEFINEDHYPOTHESIS){
      throw(std::runtime_error("MTest::setDefaultHypothesis: "
			       "internal error : the modelling "
			       "hypothesis is already defined"));
    }
    if(this->b.get()!=nullptr){
      throw(std::runtime_error("MTest::setDefaultHypothesis: "
			       "behaviour already defined"));
    }
    if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL1){
      auto& log = mfront::getLogStream();
      log << "No hypothesis defined, using default" << std::endl;
    }
    this->hypothesis = MH::TRIDIMENSIONAL;
    this->dimension  = 3u;
  }

  void
  MTest::setOutputFileName(const std::string& o)
  {
    if(!this->output.empty()){
      throw(std::runtime_error("MTest::setOutputFileName: "
			       "output file name already defined"));
    }
    this->output = o;
  }

  void
  MTest::setOutputFilePrecision(const unsigned int p)
  {
    if(this->oprec!=-1){
      throw(std::runtime_error("MTest::setOutputFileName: "
			       "output file precision already defined"));
    }
    this->oprec = static_cast<int>(p);
  }

  void
  MTest::setResidualFileName(const std::string& o)
  {
    if(!this->residualFileName.empty()){
      throw(std::runtime_error("MTest::setResidualFileName : "
			       "residual file name already defined"));
    }
    this->residualFileName = o;
  }

  void
  MTest::setResidualFilePrecision(const unsigned int p)
  {
    if(this->rprec!=-1){
      throw(std::runtime_error("MTest::setResidualFileName: "
			       "residual file precision already defined"));
    }
    this->rprec = static_cast<int>(p);
  }

  void
  MTest::addTest(const std::shared_ptr<MTest::UTest> t)
  {
    this->tests.push_back(t);
  }

  void
  MTest::setDrivingVariablesInitialValues(const std::vector<real>& v)
  {
    using namespace std;
    if(!this->e_t0.empty()){
      string msg("MTest::setDrivingVariablesInitialValues : ");
      msg += "the initial values of the strains have already been declared";
      throw(runtime_error(msg));
    }
    const unsigned short N = this->b->getDrivingVariablesSize(this->hypothesis);
    if(v.size()!=N){
      string msg("MTest::setDrivingVariablesInitialValues : ");
      msg += "invalid initial values size";
      throw(runtime_error(msg));
    }
    this->e_t0.resize(N,0);
    copy(v.begin(),v.end(),this->e_t0.begin());
  }
  
  void
  MTest::setThermodynamicForcesInitialValues(const std::vector<real>& v)
  {
    using namespace std;
    if(!this->s_t0.empty()){
      string msg("MTest::setThermodynamicForcesInitialValues : ");
      msg += "the initial values of the strains have already been declared";
      throw(runtime_error(msg));
    }
    const unsigned short N = this->b->getThermodynamicForcesSize(this->hypothesis);
    if(v.size()!=N){
      string msg("MTest::setThermodynamicForcesInitialValues : ");
      msg += "invalid initial values size";
      throw(runtime_error(msg));
    }
    this->s_t0.resize(N,0);
    copy(v.begin(),v.end(),this->s_t0.begin());
  } // end of MTest::setThermodynamicForcesInitialValues
  
  void
  MTest::setScalarInternalStateVariableInitialValue(const std::string& n,
						    const real v)
  {
    using namespace std;
    if(this->b.get()==nullptr){
      string msg("MTest::setScalarInternalStateVariableInitialValue : ");
      msg += "no behaviour defined";
      throw(runtime_error(msg));
    }
    const auto& ivsnames = this->b->getInternalStateVariablesNames();
    if(find(ivsnames.begin(),ivsnames.end(),n)==ivsnames.end()){
      string msg("MTest::setScalarInternalStateVariableInitialValue : ");
      msg += "the behaviour don't declare an internal state variable named '";
      msg += n+"'";
      throw(runtime_error(msg));
    }
    const int type           = this->b->getInternalStateVariableType(n);
    const unsigned short pos = this->b->getInternalStateVariablePosition(this->hypothesis,n);
    if(type!=0){
      string msg("MTest::setScalarInternalStateVariableInitialValue : ");
      msg += "internal state variable '"+n+"' is not defined";
      throw(runtime_error(msg));
    }
    if(this->iv_t0.size()<=pos){
      this->iv_t0.resize(pos+1,0.);
    }
    this->iv_t0[pos] = v;
  }

  void
  MTest::setStensorInternalStateVariableInitialValues(const std::string& n,
						      const std::vector<real>& v)
  {
    using namespace std;
    if(this->b.get()==nullptr){
      string msg("MTest::setStensorInternalStateVariableInitialValue : ");
      msg += "no behaviour defined";
      throw(runtime_error(msg));
    }
    const auto& ivsnames = this->b->getInternalStateVariablesNames();
    if(find(ivsnames.begin(),ivsnames.end(),n)==ivsnames.end()){
      string msg("MTest::setStensorInternalStateVariableInitialValue : ");
      msg += "the behaviour don't declare an internal state variable named '";
      msg += n+"'";
      throw(runtime_error(msg));
    }
    const int type           = this->b->getInternalStateVariableType(n);
    const unsigned short pos = this->b->getInternalStateVariablePosition(this->hypothesis,n);
    if(type!=1){
      string msg("MTest::setStensorInternalStateVariableInitialValue : ");
      msg += "internal state variable '"+n+"' is not defined";
      throw(runtime_error(msg));
    }
    const unsigned short N = getStensorSize(this->dimension);
    if(v.size()!=N){
      string msg("MTest::setStensorInternalStateVariableInitialValues : "
		 "invalid values size");
      throw(runtime_error(msg));
    }
    if(this->iv_t0.size()<pos+N){
      this->iv_t0.resize(pos+N,0.);
    }
    copy(v.begin(),v.end(),
	 this->iv_t0.begin()+pos);
  } // end of MTest::setStensorInternalStateVariableInitialValue

  void
  MTest::setTensorInternalStateVariableInitialValues(const std::string& n,
						      const std::vector<real>& v)
  {
    using namespace std;
    if(this->b.get()==nullptr){
      throw(runtime_error("MTest::setTensorInternalStateVariableInitialValue: "
			  "no behaviour defined"));
    }
    const auto& ivsnames = this->b->getInternalStateVariablesNames();
    if(find(ivsnames.begin(),ivsnames.end(),n)==ivsnames.end()){
      string msg("MTest::setTensorInternalStateVariableInitialValue : ");
      msg += "the behaviour don't declare an internal state variable named '";
      msg += n+"'";
      throw(runtime_error(msg));
    }
    const int type           = this->b->getInternalStateVariableType(n);
    const unsigned short pos = this->b->getInternalStateVariablePosition(this->hypothesis,n);
    if(type!=3){
      string msg("MTest::setTensorInternalStateVariableInitialValue : ");
      msg += "internal state variable '"+n+"' is not defined";
      throw(runtime_error(msg));
    }
    const unsigned short N = getTensorSize(this->dimension);
    if(v.size()!=N){
      string msg("MTest::setTensorInternalStateVariableInitialValues : "
		 "invalid values size");
      throw(runtime_error(msg));
    }
    if(this->iv_t0.size()<pos+N){
      this->iv_t0.resize(pos+N,0.);
    }
    copy(v.begin(),v.end(),
	 this->iv_t0.begin()+pos);
  } // end of MTest::setTensorInternalStateVariableInitialValue

  void
  MTest::completeInitialisation()
  {
    using namespace std;
    using namespace tfel::material;
    typedef tfel::material::ModellingHypothesis MH;
    EvolutionManager::const_iterator pev;
    SchemeBase::completeInitialisation();
    // output
    if(!this->output.empty()){
      this->out.open(this->output.c_str());
      if(!this->out){
	throw(runtime_error("MTest::completeInitialisation : "
			    "can't open file '"+this->output+"'"));
      }
      this->out.exceptions(ofstream::failbit|ofstream::badbit);
      if(this->oprec!=-1){
	this->out.precision(static_cast<streamsize>(this->oprec));
      }
    }
    // post-processing
    unsigned short cnbr = 2;
    const char* dvn;
    const char* thn;
    this->out << "# first column : time" << endl;
    if(this->b->getBehaviourType()==MechanicalBehaviourBase::SMALLSTRAINSTANDARDBEHAVIOUR){
      dvn = "strain";
      thn = "stress";
    } else if(this->b->getBehaviourType()==MechanicalBehaviourBase::FINITESTRAINSTANDARDBEHAVIOUR){
      dvn = "deformation gradient";
      thn = "Cauchy stress";
    } else if(this->b->getBehaviourType()==MechanicalBehaviourBase::COHESIVEZONEMODEL){
      dvn = "opening displacement";
      thn = "cohesive force";
    } else {
      dvn = "driving variables";
      thn = "thermodynamic forces";
    }
    const unsigned short ndv = this->b->getDrivingVariablesSize(this->hypothesis);
    const unsigned short nth = this->b->getThermodynamicForcesSize(this->hypothesis);
    for(unsigned short i=0;i!=ndv;++i){
      this->out << "# " << cnbr << " column : " << i+1 << "th component of the " << dvn << endl;
      ++cnbr;
    }
    for(unsigned short i=0;i!=nth;++i){
      this->out << "# " << cnbr << " column : " << i+1 << "th component of the " << thn << endl;
      ++cnbr;
    }
    const auto& ivdes = this->b->getInternalStateVariablesDescriptions(this->hypothesis);
    if(ivdes.size()!=this->b->getInternalStateVariablesSize(this->hypothesis)){
      throw(std::runtime_error("MTest::completeInitialisation : internal error "
			       "(the number of descriptions given by "
			       "the mechanical behaviour don't match "
			       "the number of internal state variables)"));
    }
    for(std::vector<string>::size_type i=0;i!=ivdes.size();++i){
      this->out << "# " << cnbr << " column : " << ivdes[i] << endl;
      ++cnbr;
    }
    // convergence criterium value for driving variables
    if(this->eeps<0){
      this->eeps = 1.e-12;
    }
    // convergence criterium value for thermodynamic forces
    if(this->seps<0){
      this->seps = 1.e-3;
    }
    // tangent operator comparison criterium
    if(this->toeps<0){
      this->toeps = (this->seps/1e-3)*1.e7;
    }
    // perturbation value
    if(this->pv<0){
      this->pv = 10*this->eeps;
    }
    // additional constraints
    if(this->hypothesis==MH::PLANESTRAIN){
      shared_ptr<Evolution>  eev(new ConstantEvolution(0.));
      shared_ptr<Constraint> ec(new ImposedDrivingVariable(2,eev));
      this->constraints.push_back(ec);
    }
    if(this->hypothesis==MH::AXISYMMETRICALGENERALISEDPLANESTRESS){
      // shall be in the behaviour
      if((this->b->getBehaviourType()==MechanicalBehaviourBase::SMALLSTRAINSTANDARDBEHAVIOUR)||
	 (this->b->getBehaviourType()==MechanicalBehaviourBase::FINITESTRAINSTANDARDBEHAVIOUR)){
	shared_ptr<Evolution>  eev(new ConstantEvolution(0.));
	shared_ptr<Constraint> ec(new ImposedDrivingVariable(1,eev));
	shared_ptr<Evolution>  sev;
	pev = this->evm->find("AxialStress");
	if(pev!=this->evm->end()){
	  sev = pev->second;
	} else {
	  sev = shared_ptr<Evolution>(new ConstantEvolution(0.));
	}
	shared_ptr<Constraint> sc(new ImposedThermodynamicForce(1,sev));
	this->constraints.push_back(ec);
	this->constraints.push_back(sc);
      } else {
	throw(std::runtime_error("MTest::completeInitialisation : "
				 "generalised plane stress is only "
				 "handled for small and finite strain behaviours"));
      }
    }
    if(this->hypothesis==MH::PLANESTRESS){
      // shall be in the behaviour
      if((this->b->getBehaviourType()==MechanicalBehaviourBase::SMALLSTRAINSTANDARDBEHAVIOUR)||
	 (this->b->getBehaviourType()==MechanicalBehaviourBase::FINITESTRAINSTANDARDBEHAVIOUR)){
	shared_ptr<Evolution>  eev(new ConstantEvolution(0.));
	shared_ptr<Constraint> ec(new ImposedDrivingVariable(2,eev));
	shared_ptr<Evolution>  sev(new ConstantEvolution(0.));
	shared_ptr<Constraint> sc(new ImposedThermodynamicForce(2,sev));
	this->constraints.push_back(ec);
	this->constraints.push_back(sc);
      } else {
	throw(std::runtime_error("MTest::completeInitialisation : "
				 "plane stress is only handled for small and "
				 "finite strain behaviours"));
      }
    }
    if(!this->isRmDefined){
      for(unsigned short i=0;i!=3;++i){
	rm(i,i) = real(1);
      }
    }
    // thermal expansion reference temperature
    pev = this->evm->find("ThermalExpansionReferenceTemperature");
    if(pev!=this->evm->end()){
      const Evolution& ev = *(pev->second);
      if(!ev.isConstant()){
	string msg("MTest::completeInitialisation : 'ThermalExpansionReferenceTemperature' "
		   "must be a constant evolution");
	throw(runtime_error(msg));
      }
    }
    //
    if(!this->residualFileName.empty()){
      this->residual.open(this->residualFileName.c_str());
      if(!this->residual){
	string msg("MTest::completeInitialisation : "
		   "unable to open file '"+this->residualFileName+"'");
	throw(runtime_error(msg));
      }
      this->residual.exceptions(ofstream::failbit|ofstream::badbit);
      if(this->rprec!=-1){
	this->residual.precision(static_cast<streamsize>(this->rprec));
      } else {
	if(this->oprec!=-1){
	  this->residual.precision(static_cast<streamsize>(this->oprec));
	}
      }
      this->residual << "#first  column : iteration number"             << endl;
      this->residual << "#second column : driving variable residual"    << endl;
      this->residual << "#third  column : thermodynamic force residual" << endl;
      this->residual << "#The following columns are the components of the driving variable" << endl;
    }
  }

  void
  MTest::initializeCurrentState(MTestCurrentState& s) const
  {
    using namespace std;
    // getting the names of the materials properties
    const auto mpnames(this->b->getMaterialPropertiesNames());
    const auto esvnames(this->b->getExternalStateVariablesNames());
    const auto psz = this->getNumberOfUnknowns();
    // clear
    s.s_1.clear();
    s.s0.clear();
    s.s1.clear();
    s.e0.clear();
    s.e1.clear();
    s.e_th0.clear();
    s.e_th1.clear();
    s.u_1.clear();
    s.u0.clear();
    s.u1.clear();
    s.u10.clear();
    s.mprops0.clear();
    s.mprops1.clear();
    s.iv_1.clear();
    s.iv0.clear();
    s.iv1.clear();
    s.esv0.clear();
    s.desv.clear();
    // resizing
    const auto ndv = this->b->getDrivingVariablesSize(this->hypothesis);
    const auto nth = this->b->getThermodynamicForcesSize(this->hypothesis);
    s.s_1.resize(nth,0.);
    s.s0.resize(nth,0.);
    s.s1.resize(nth,0.);
    s.e0.resize(ndv,0.);
    s.e1.resize(ndv,0.);
    s.e_th0.resize(ndv,0.);
    s.e_th1.resize(ndv,0.);
    s.u_1.resize(psz,0.);
    s.u0.resize(psz,0.);
    s.u1.resize(psz,0.);
    s.u10.resize(psz,0.);
    s.mprops0.resize(mpnames.size());
    s.mprops1.resize(mpnames.size());
    s.iv_1.resize(this->b->getInternalStateVariablesSize(this->hypothesis),0.);
    s.iv0.resize(s.iv_1.size(),0.);
    s.iv1.resize(s.iv0.size(),0.);
    s.esv0.resize(esvnames.size(),0.);
    s.desv.resize(esvnames.size(),0.);
    // setting the intial  values of strains
    this->b->getDrivingVariablesDefaultInitialValues(s.u_1);
    copy(this->e_t0.begin(),this->e_t0.end(),s.u_1.begin());
    s.u0 = s.u_1;
    // setting the intial  values of stresses
    copy(this->s_t0.begin(),this->s_t0.end(),s.s0.begin());
    // getting the initial values of internal state variables
    if(this->iv_t0.size()>s.iv0.size()){
      string msg("MTest::initializeCurrentState : the number of initial values declared "
		 "by the user for the internal state variables exceeds the "
		 "number of internal state variables declared by the behaviour");
      throw(runtime_error(msg));
    }
    copy(this->iv_t0.begin(),this->iv_t0.end(),s.iv_1.begin());
    copy(this->iv_t0.begin(),this->iv_t0.end(),s.iv0.begin());
    // mandatory for time step management
    s.period = 1u;
    s.dt_1   = 0.;
    // reference temperature
    const auto pev = this->evm->find("ThermalExpansionReferenceTemperature");
    if(pev!=this->evm->end()){
      const Evolution& ev = *(pev->second);
      if(!ev.isConstant()){
	string msg("MTest::initializeCurrentState : "
		   "'ThermalExpansionReferenceTemperature' "
		   "must be a constant evolution");
	throw(runtime_error(msg));
      }
      s.Tref = ev(0);
    } else {
      s.Tref = real(293.15);
    }
  } // end of MTest::initializeCurrentState

  void
  MTest::initializeWorkSpace(MTestWorkSpace& wk) const
  {
    const auto psz = this->getNumberOfUnknowns();
    const auto ndv = this->b->getDrivingVariablesSize(this->hypothesis);
    const auto nth = this->b->getThermodynamicForcesSize(this->hypothesis);
    const auto nstatev = this->b->getInternalStateVariablesSize(this->hypothesis);
    // clear
    wk.K.clear();
    wk.Kt.clear();
    wk.Kp.clear();
    wk.p_lu.clear();
    wk.x.clear();
    wk.r.clear();
    wk.du.clear();
    //resizing
    wk.K.resize(psz,psz);
    wk.Kt.resize(nth,ndv);
    wk.nKt.resize(nth,ndv);
    wk.tKt.resize(nth,ndv);
    wk.s1.resize(nth);
    wk.s2.resize(nth);
    wk.statev.resize(nstatev);
    wk.Kp.resize(nth,ndv);
    wk.p_lu.resize(psz);
    wk.x.resize(psz);
    wk.r.resize(psz,0.);
    wk.du.resize(psz,0.);
    wk.first = true;
    wk.a = 0;
  } // end of MTest::initializeWorkSpace
  
  size_t
  MTest::getNumberOfUnknowns() const
  {
    using tfel::math::vector;
    // number of components of the driving variables
    const unsigned short N = this->b->getDrivingVariablesSize(this->hypothesis);
    // getting the total number of unknowns
    size_t s = N;
    for(const auto& pc : this->constraints){
      const Constraint& c = *pc;
      s += c.getNumberOfLagrangeMultipliers();
    }
    return s;
  } // end of MTest::getNumberOfUnknowns

  tfel::tests::TestResult
  MTest::execute(void)
  {
    using namespace std;
    using namespace tfel::tests;
    using tfel::math::vector;
    TestResult tr;
    vector<real>::const_iterator pt;
    vector<real>::const_iterator pt2;
    vector<shared_ptr<UTest> >::iterator ptest;
    // some checks
    if(times.empty()){
      string msg("MTest::execute : no times defined");
      throw(runtime_error(msg));
    }
    if(times.size()<2){
      string msg("MTest::execute :"
		 "invalid number of times defined");
      throw(runtime_error(msg));
    }
    // finish initialization
    this->completeInitialisation();
    // initialize current state and work space
    MTestCurrentState state;
    MTestWorkSpace    wk;
    this->initializeCurrentState(state);
    this->initializeWorkSpace(wk);
    // integrating over the loading path
    pt  = this->times.begin();
    pt2 = pt+1;
    this->printOutput(*pt,state);
    // real work begins here
    while(pt2!=this->times.end()){
      // allowing subdivisions of the time step
      this->execute(state,wk,*pt,*pt2);
      this->printOutput(*pt2,state);
      ++pt;
      ++pt2;
    }
    for(ptest=this->tests.begin();ptest!=this->tests.end();++ptest){
      const UTest& test = *(*ptest);
      tr.append(test.getResults());
    }
    return tr; 
  }

  void
  MTest::setCompareToNumericalTangentOperator(const bool bo)
  {
    this->cto = bo;
  } // end of MTest::setCompareToNumericalTangentOperator

  void
  MTest::setTangentOperatorComparisonCriterium(const real v)
  {
    using namespace std;
    if(v<100*numeric_limits<real>::min()){
      string msg("MTest::setTangentOperatorComparisonCriterium : " );
      msg += "invalid comparison criterium";
      throw(runtime_error(msg));
    }
    this->toeps=v;
  } // end of MTest::handleTangentOperatorComparisonCriterium

  void
  MTest::setNumericalTangentOperatorPerturbationValue(const real v)
  {
    using namespace std;
    if(v<100*numeric_limits<real>::min()){
      string msg("MTest::setNumericalTangentOperatorPerturbationValue : " );
      msg += "invalid perturbation value";
      throw(runtime_error(msg));
    }
    this->pv=v;
  } // end of MTest::setNumericalTangentOperatorPerturbationValue

  void
  MTest::execute(MTestCurrentState& state,
		 MTestWorkSpace& wk,
		 const real ti,
		 const real te)
  {
    using namespace std;
    using namespace tfel::utilities;
    using namespace tfel::math;
    using namespace tfel::material;
    using tfel::tests::TestResult;
    using tfel::math::vector;
    vector<string>::const_iterator p;
    vector<shared_ptr<Constraint> >::const_iterator pc;
    EvolutionManager::const_iterator pev;
    EvolutionManager::const_iterator pev2;
    EvolutionManager::const_iterator pev3;
    EvolutionManager::const_iterator pev4;
    vector<shared_ptr<UTest> >::iterator ptest;
    // getting the names of the materials properties
    std::vector<string> mpnames(this->b->getMaterialPropertiesNames());
    // getting the names of the external state variables
    std::vector<string> esvnames(this->b->getExternalStateVariablesNames());
    unsigned short subStep = 0;
    // // number of components of the driving variables and the thermodynamic forces
    const unsigned short ndv = this->b->getDrivingVariablesSize(this->hypothesis);
    const unsigned short nth = this->b->getThermodynamicForcesSize(this->hypothesis);
    real t  = ti;
    real dt = te-ti;
    if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL2){
      auto& log = mfront::getLogStream();
      log << "number of driving variables : "    << ndv << endl;
      log << "number of thermodynamic forces : " << nth << endl;
      log << "number of constraints       : " << this->constraints.size() << endl;
    }
    while((abs(te-t)>0.5*dt)&&
	  (subStep!=this->mSubSteps)){
      unsigned short i,j;
      // evaluations of the materials properties at the end of the
      // time step
      for(i=0,p=mpnames.begin();p!=mpnames.end();++p,++i){
	pev = this->evm->find(*p);
	if(pev!=this->evm->end()){
	  const Evolution& ev = *(pev->second);
	  state.mprops1[i] = ev(t+dt);
	} else {
	  pev = this->defaultMaterialPropertiesValues->find(*p);
	  if(pev!=this->evm->end()){
	    const Evolution& ev = *(pev->second);
	    state.mprops1[i] = ev(t+dt);
	  } else {
	    string msg("MTest::execute : no evolution named '"+*p+"'");
	    throw(runtime_error(msg));
	  }
	}
      }
      for(i=0,p=esvnames.begin();p!=esvnames.end();++p,++i){
	pev = this->evm->find(*p);
	if(pev==this->evm->end()){
	  string msg("MTest::execute : no evolution named '"+*p+"'");
	  throw(runtime_error(msg));
	}
	const Evolution& ev = *(pev->second);
	const real evt = ev(t);
	state.esv0[i] = evt;
	state.desv[i] = ev(t+dt)-evt;
      }
      if(this->handleThermalExpansion){
	if(this->b->getBehaviourType()==MechanicalBehaviourBase::SMALLSTRAINSTANDARDBEHAVIOUR){
	  // thermal expansion (isotropic case)
	  pev   = this->evm->find("Temperature");
	  pev2  = this->evm->find("ThermalExpansion");
	  if((pev!=this->evm->end())&&(pev2!=this->evm->end())){
	    const Evolution& T_ev = *(pev->second);
	    const Evolution& a_ev = *(pev2->second);
	    const real eth0 = a_ev(t)*(T_ev(t)-state.Tref);
	    const real eth1 = a_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	    for(i=0;i!=3;++i){
	      state.e_th0[i] = eth0;
	      state.e_th1[i] = eth1;
	    }
	  }
	  pev2  = this->evm->find("ThermalExpansion1");
	  pev3  = this->evm->find("ThermalExpansion2");
	  pev4  = this->evm->find("ThermalExpansion3");
	  if((pev!=this->evm->end())&&
	     ((pev2!=this->evm->end())||
	      (pev3!=this->evm->end())||
	      (pev4!=this->evm->end()))){
	    if((pev2==this->evm->end())||
	       (pev3==this->evm->end())||
	       (pev4==this->evm->end())){
	      string msg("MTest::execute : at least one of the three "
			 "thermal expansion coefficient is defined and "
			 "at least one is not");
	      throw(runtime_error(msg));
	    }
	    const Evolution& T_ev  = *(pev->second);
	    const Evolution& a1_ev = *(pev2->second);
	    const Evolution& a2_ev = *(pev3->second);
	    const Evolution& a3_ev = *(pev4->second);
	    if(this->dimension==1u){
	      state.e_th0[0u] = a1_ev(t)*(T_ev(t)-state.Tref);
	      state.e_th1[0u] = a1_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	      state.e_th0[1u] = a2_ev(t)*(T_ev(t)-state.Tref);
	      state.e_th1[1u] = a2_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	      state.e_th0[2u] = a3_ev(t)*(T_ev(t)-state.Tref);
	      state.e_th1[2u] = a3_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	    } else if((this->dimension==2u)||
		      (this->dimension==3u)){
	      // thermal expansion tensors in the material referential
	      stensor<3u,real> se_th0(real(0));
	      stensor<3u,real> se_th1(real(0));
	      se_th0[0u] = a1_ev(t)*(T_ev(t)-state.Tref);
	      se_th1[0u] = a1_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	      se_th0[1u] = a2_ev(t)*(T_ev(t)-state.Tref);
	      se_th1[1u] = a2_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	      se_th0[2u] = a3_ev(t)*(T_ev(t)-state.Tref);
	      se_th1[2u] = a3_ev(t+dt)*(T_ev(t+dt)-state.Tref);
	      // backward rotation matrix
	      tmatrix<3u,3u,real> brm = transpose(this->b->getRotationMatrix(state.mprops1,this->rm));
	      se_th0.changeBasis(brm);
	      se_th1.changeBasis(brm);
	      copy(se_th0.begin(),se_th0.begin()+getStensorSize(this->dimension),state.e_th0.begin());
	      copy(se_th1.begin(),se_th1.begin()+getStensorSize(this->dimension),state.e_th1.begin());
	    } else {
	      string msg("MTest::execute : invalid dimension");
	      throw(runtime_error(msg));
	    }
	  }
	}
      }
      // driving variables at the beginning of the time step
      for(i=0;i!=ndv;++i){
	state.e0[i] = state.u0[i]-state.e_th0[i];
      }
      // resolution
      bool converged = false;
      unsigned short iter = 0;
      if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL1){
	auto& log = mfront::getLogStream();
	log << "resolution from " << t << " to " << t+dt << endl;
      }
      if(this->residual){
	this->residual << endl << "#resolution from " << t << " to " << t+dt << endl;
      }
      real ne  = 0.;  // norm of the increment of the driving variables
      real nep = 0.;  // norm of the increment of the driving variables at the
      // previous iteration
      real nep2 = 0.; // norm of the increment of the driving variables two
      // iterations before...
      /* prediction */
      if(this->ppolicy==LINEARPREDICTION){
	if(state.period>1){
	  state.u1  = state.u0 +(state.u0 -state.u_1)*dt/state.dt_1;
	  state.iv1 = state.iv0+(state.iv0-state.iv_1)*dt/state.dt_1;
	  state.s1  = state.s0 +(state.s0 -state.s_1)*dt/state.dt_1;
	} else {
	  state.u1  = state.u0;
	  state.s1  = state.s0;
	  state.iv1 = state.iv0;
	}
      } else if((this->ppolicy==ELASTICPREDICTION)||
		(this->ppolicy==ELASTICPREDICTIONFROMMATERIALPROPERTIES)||
		(this->ppolicy==SECANTOPERATORPREDICTION)||
		(this->ppolicy==TANGENTOPERATORPREDICTION)){
	// evaluations of the materials properties at the beginning
	// of the time step
	for(i=0,p=mpnames.begin();p!=mpnames.end();++p,++i){
	  pev = this->evm->find(*p);
	  if(pev!=this->evm->end()){
	    const Evolution& ev = *(pev->second);
	    state.mprops0[i] = ev(t);
	  } else {
	    pev = this->defaultMaterialPropertiesValues->find(*p);
	    if(pev!=this->evm->end()){
	      const Evolution& ev = *(pev->second);
	      state.mprops0[i] = ev(t);
	    } else {
	      string msg("MTest::execute : no evolution named '"+*p+"'");
	      throw(runtime_error(msg));
	    }
	  }
	}
	fill(wk.Kp.begin(),wk.Kp.end(),0.);
	bool sp(true);
	if(this->ppolicy==ELASTICPREDICTION){
	  sp = this->b->computePredictionOperator(wk.Kp,this->rm,state.e0,
						  state.s0,state.mprops0,
						  state.iv0,state.esv0,
						  this->hypothesis,
						  StiffnessMatrixType::ELASTIC);
	} else if(this->ppolicy==ELASTICPREDICTIONFROMMATERIALPROPERTIES){
	  sp = this->b->computePredictionOperator(wk.Kp,this->rm,state.e0,
						  state.s0,state.mprops0,
						  state.iv0,state.esv0,
						  this->hypothesis,
						  StiffnessMatrixType::ELASTICSTIFNESSFROMMATERIALPROPERTIES);
	} else if (this->ppolicy==SECANTOPERATORPREDICTION){
	  sp = this->b->computePredictionOperator(wk.Kp,this->rm,state.e0,
						  state.s0,state.mprops0,
						  state.iv0,state.esv0,
						  this->hypothesis,
						  StiffnessMatrixType::SECANTOPERATOR);
	} else if (this->ppolicy==TANGENTOPERATORPREDICTION){
	  sp = this->b->computePredictionOperator(wk.Kp,this->rm,state.e0,
						  state.s0,state.mprops0,
						  state.iv0,state.esv0,
						  this->hypothesis,
						  StiffnessMatrixType::TANGENTOPERATOR);
	}
	if(sp){
	  fill(wk.K.begin(),wk.K.end(),0.);
	  fill(wk.r.begin(),wk.r.end(),0.);
	  state.u1 = state.u0;
	  for(i=0;i!=nth;++i){
	    wk.r(i) += state.s0(i);
	    for(j=0;j!=ndv;++j){
	      wk.K(i,j)+=wk.Kp(i,j);
	      // free dilatation treatment
	      wk.r(i) -= wk.Kp(i,j)*(state.e_th1[j]-state.e_th0[j]);
	    }
	  }
	  if(wk.first){
	    wk.a = *(max_element(wk.K.begin(),wk.K.end()));
	    wk.first = false;
	  }
	  unsigned short pos = ndv;
	  for(pc =this->constraints.begin();
	      pc!=this->constraints.end();++pc){
	    const Constraint& c = *(*pc);
	    c.setValues(wk.K,wk.r,state.u0,state.u1,pos,
			this->dimension,t,dt,wk.a);
	    pos = static_cast<unsigned short>(pos+c.getNumberOfLagrangeMultipliers());
	  }
	  wk.du = wk.r;
	  LUSolve::exe(wk.K,wk.du,wk.x,wk.p_lu);
	  state.u1 -= wk.du;
	} else {
	  if(mfront::getVerboseMode()>mfront::VERBOSE_QUIET){
	    auto& log = mfront::getLogStream();
	    log << "MTest::execute : behaviour compute prediction matrix failed" << endl;
	  }
	  state.u1  = state.u0;
	  state.s1  = state.s0;
	  state.iv1 = state.iv0;
	}
      } else {
	if(this->ppolicy != NOPREDICTION){
	  string msg("MTest::execute : "
		     "internal error, unsupported "
		     "prediction policy");
	  throw(runtime_error(msg));
	}	    
	state.u1  = state.u0;
	state.s1  = state.s0;
	state.iv1 = state.iv0;
      }
      /* resolution */
      bool cont = true;
      vector<string> failed_criteria;
      if(this->aa.get()!=nullptr){
	this->aa->preExecuteTasks();
      }
      while((!converged)&&(iter!=this->iterMax)&&(cont)){
	failed_criteria.clear();
	++iter;
	nep2 = nep;
	nep  = ne;
	fill(wk.K.begin(), wk.K.end(),0.);
	fill(wk.Kt.begin(),wk.Kt.end(),0.);
	fill(wk.r.begin(), wk.r.end(),0.);
	for(i=0;i!=ndv;++i){
	  state.e1[i] = state.u1[i]-state.e_th1[i];
	}
	const bool bi = this->b->integrate(wk.Kt,state.s1,state.iv1,this->rm,
					   state.e0,state.e1,state.s0,
					   state.mprops1,state.iv0,
					   state.esv0,state.desv,
					   this->hypothesis,dt,
					   this->ktype);
	if(this->cto){
	  fill(wk.nKt.begin(),wk.nKt.end(),real(0));
	  bool ok = true;
	  for(j=0;(j!=ndv)&&(ok);++j){
	    copy(state.s0.begin(),state.s0.end(),wk.s1.begin());
	    copy(state.s0.begin(),state.s0.end(),wk.s2.begin());
	    copy(state.iv0.begin(),state.iv0.end(),wk.statev.begin());
	    state.e1[j]+=this->pv;
	    const bool ok1 = this->b->integrate(wk.tKt,wk.s1,wk.statev,this->rm,
						state.e0,state.e1,state.s0,
						state.mprops1,state.iv0,
						state.esv0,state.desv,
						this->hypothesis,dt,
						StiffnessMatrixType::NOSTIFFNESS);
	    copy(state.iv0.begin(),state.iv0.end(),wk.statev.begin());
	    state.e1[j]-=2*(this->pv);
	    const bool ok2 = this->b->integrate(wk.tKt,wk.s2,wk.statev,this->rm,
						state.e0,state.e1,state.s0,
						state.mprops1,state.iv0,
						state.esv0,state.desv,
						this->hypothesis,dt,
						StiffnessMatrixType::NOSTIFFNESS);
	    state.e1[j]+=this->pv;
	    ok = ok1 && ok2;
	    if(ok){
	      for(i=0;i!=nth;++i){
		wk.nKt(i,j) = (wk.s1(i)-wk.s2(i))/(2*(this->pv));
	      }
	    }
	  }
	  if(ok){
	    real merr(0);
	    for(i=0;i!=nth;++i){
	      for(j=0;j!=ndv;++j){
		merr = max(merr,abs(wk.Kt(i,j)-wk.nKt(i,j)));
	      }
	    }
	    if(merr>this->toeps){
	      auto& log = mfront::getLogStream();
	      log << "Comparison to numerical jacobian failed (error : " << merr << ", criterium " << this->toeps << ")." << endl;
	      log << "Tangent operator returned by the behaviour : " << endl;
	      for(i=0;i!=nth;++i){
		for(j=0;j!=ndv;){
		  log << wk.Kt(i,j);
		  if(++j!=ndv){
		    log << " ";
		  }
		}
		log << "]" << endl;
	      }
	      log << "Numerical tangent operator (perturbation value : " << this-> pv << ") : " << endl;
	      for(i=0;i!=nth;++i){
		for(j=0;j!=ndv;){
		  log << wk.nKt(i,j);
		  if(++j!=ndv){
		    log << " ";
		  }
		}
		log << "]" << endl;
	      }
	    }
	  } else {
	    auto& log = mfront::getLogStream();
	    log << "Numerical evalution of tangent operator failed." << endl << endl;
	  }
	}
	if(bi){
	  // behaviour integration is a success
	  if(this->b->getBehaviourType()==MechanicalBehaviourBase::FINITESTRAINSTANDARDBEHAVIOUR){
	    for(i=0;i!=3u;++i){
	      wk.r(i) += state.s1(i);
	      for(j=0;j!=ndv;++j){
		wk.K(i,j) += wk.Kt(i,j);
	      }
	    }
	    if(this->dimension>1u){
	      for(i=0;i!=nth-3u;++i){
		wk.r(2*i+3u)   += state.s1(i);
		wk.r(2*i+3u+1) += state.s1(i);
		for(j=0;j!=ndv;++j){
		  wk.K(2*i+3u,j)   += wk.Kt(i+3u,j);
		  wk.K(2*i+3u+1,j) += wk.Kt(i+3u,j);
		}
	      }
	    }
	  } else {
	    for(i=0;i!=nth;++i){
	      wk.r(i) += state.s1(i);
	      for(j=0;j!=ndv;++j){
		wk.K(i,j) += wk.Kt(i,j);
	      }
	    }
	  }
	  if(wk.first){
	    wk.a = *(max_element(wk.K.begin(),wk.K.end()));
	    wk.first = false;
	  }
	  unsigned short pos = ndv;
	  for(pc =this->constraints.begin();
	      pc!=this->constraints.end();++pc){
	    const Constraint& c = *(*pc);
	    c.setValues(wk.K,wk.r,state.u0,state.u1,pos,
			this->dimension,t,dt,wk.a);
	    pos = static_cast<unsigned short>(pos+c.getNumberOfLagrangeMultipliers());
	  }
	  if(mfront::getVerboseMode()>=mfront::VERBOSE_DEBUG){
	    auto& log = mfront::getLogStream();
	    for(i=0;i!=wk.K.getNbRows();++i){
	      for(j=0;j!=wk.K.getNbCols();++j){
		log << wk.K(i,j) << " ";
	      }
	      log << endl;
	    }
	    log << endl;
	  }
	  wk.du = wk.r;
	  LUSolve::exe(wk.K,wk.du,wk.x,wk.p_lu);
	  state.u1 -= wk.du;
	  real nr = 0.;
	  for(i=0;i!=nth;++i){
	    nr = max(nr,abs(wk.r(i)));
	  }
	  ne = 0;
	  for(i=0;i!=ndv;++i){
	    ne = max(ne,abs(wk.du(i)));
	  }
	  if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL1){
	    auto& log = mfront::getLogStream();
	    log << "iteration " << iter << " : " << ne << " " 
		<< nr << " (";
	    for(i=0;i!=ndv;){
	      log << state.u1(i);
	      if(++i!=ndv){
		log << " ";
	      }
	    }
	    log << ")" << endl;
	  }
	  if(this->residual){
	    this->residual << iter << " " << ne << " " 
			   << nr << " ";
	    for(i=0;i!=ndv;){
	      this->residual << state.u1(i);
	      if(++i!=ndv){
		this->residual << " ";
	      }
	    }
	    this->residual << endl;
	  }
	  converged = (ne<this->eeps)&&(nr<this->seps);
	  // BEGIN : to have convergence only with respect to driving variable
          // uncomment following lines
	  // converged = (ne<this->eeps);
          // END

	  // BEGIN : to have convergence only with respect to driving variable
          // comment following lines
	  for(pc =this->constraints.begin();
	      (pc!=this->constraints.end())&&(converged);++pc){
	    const Constraint& c = *(*pc);
	    converged = c.checkConvergence(state.u1,state.s1,
	  				   this->eeps,this->seps,
	  				   t,dt);
	  }
          // END
	  if((!converged)&&(iter==this->iterMax)){
	    if(ne>this->eeps){
	      ostringstream msg;
	      msg << "test on driving variables (error : " << ne
		  << ", criteria : " << this->eeps << ")";
	      failed_criteria.push_back(msg.str());
	    }
	    if(nr>this->seps){
	      ostringstream msg;
	      msg << "test on thermodynamic forces (error : " << nr
		  << ", criteria : " << this->seps << ")";
	      failed_criteria.push_back(msg.str());
	    }
	    // BEGIN : to have convergence only with respect to driving variable
	    // comment following lines
	    for(pc =this->constraints.begin();
	    	pc!=this->constraints.end();++pc){
	      const Constraint& c = *(*pc);
	      bool bc = c.checkConvergence(state.u1,state.s1,
	    				   this->eeps,this->seps,
	    				   t,dt);
	      if(!bc){
	    	failed_criteria.push_back(c.getFailedCriteriaDiagnostic(state.u1,state.s1,
	    								this->eeps,this->seps,
	    								t,dt));
	      }
	    }
	    // END
	  }
	  if(this->ppolicy == NOPREDICTION){
	    converged = (converged) && (iter>1);
	  }
	  if(!converged){
	    if(this->aa.get()!=nullptr){
	      this->aa->execute(state.u1,wk.du,wk.r,this->eeps,this->seps,iter);
	      if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL2){
		real nit = 0;
		for(i=0;i!=ndv;++i){
		  nit = max(nit,abs(state.u1(i)-state.u10(i)));
		}
		//ne = nit ; // for order of convergence??
		auto& log = mfront::getLogStream();
		log << "accelerated-sequence-convergence "<<iter<<" "<<nit << " (";
		for(i=0;i!=ndv;){
		  log << state.u1(i);
		  if(++i!=ndv){
		    log << " ";
		  }
		}
		log << ")" << endl;
	      }
	    }
	    state.u10 = state.u1;
	  }
	} else {
	  if(mfront::getVerboseMode()>mfront::VERBOSE_QUIET){
	    auto& log = mfront::getLogStream();
	    log << "MTest::execute : behaviour intregration failed" << endl;
	  }
	  cont = false;
	}
      }
      if(!converged){
	// no convergence
	if(iter==this->iterMax){
	  if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL1){
	    auto& log = mfront::getLogStream();
	    log << "No convergence, the following criteria were not met : " << endl;
	    for(vector<string>::const_iterator pfc=failed_criteria.begin();
		pfc!=failed_criteria.end();++pfc){
	      log << "- " << *pfc << endl;
	    }
	    log << endl;
	  }
	}
	++subStep;
	if(subStep==this->mSubSteps){
	  string msg("MTest::execute : behaviour "
		     "maximum number of sub stepping reached");
	  throw(runtime_error(msg));
	}
	if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL1){
	  auto& log = mfront::getLogStream();
	  log << "Dividing time "
	      << "step by two" << endl << endl;
	}
	dt *= 0.5;
	state.u1  = state.u0;
	state.s1  = state.s0;
	state.iv1 = state.iv0;
      } else {
	if(mfront::getVerboseMode()>=mfront::VERBOSE_LEVEL1){
	  if(iter==1u){
	    auto& log = mfront::getLogStream();
	    log << "convergence, after one iteration "
		<< endl << endl;
	  } else {
	    auto& log = mfront::getLogStream();
	    if((iter>=3)&&
	       (ne  >100*numeric_limits<real>::min())&&
	       (nep >100*numeric_limits<real>::min())&&
	       (nep2>100*numeric_limits<real>::min())){
	      log << "convergence, after " << iter << " iterations, "
		  << "order " << std::log(ne/nep)/std::log(nep/nep2)
		  << endl << endl;
	    } else {
	      log << "convergence, after " << iter << " iterations, "
		  << "order undefined"
		  << endl << endl;
	    }
	  }
	  if(this->aa.get()!=nullptr){
	    this->aa->postExecuteTasks();
	  }
	}
	// testing
	for(ptest=this->tests.begin();ptest!=this->tests.end();++ptest){
	  UTest& test = *(*ptest);
	  test.check(state.u1,state.s1,state.iv1,t,dt,state.period);
	}
	// update variables
	state.u_1  = state.u0;
	state.s_1  = state.s0;
	state.iv_1 = state.iv0;
	state.dt_1 = dt;
	state.u0   = state.u1;
	state.s0   = state.s1;
	state.iv0  = state.iv1;
	t+=dt;
	++state.period;
      }
    }
  }

  void
  MTest::printOutput(const real t,const MTestCurrentState& s)
  {
    using namespace std;
    if(this->out){
      // number of components of the driving variables and the thermodynamic forces
      const unsigned short ndv = this->b->getDrivingVariablesSize(this->hypothesis);
      const unsigned short nth = this->b->getThermodynamicForcesSize(this->hypothesis);
      unsigned short i;
      this->out << t << " ";
      for(i=0;i!=ndv;++i){
	this->out << s.u0[i] << " ";
      }
      for(i=0;i!=nth;++i){
	this->out << s.s0[i] << " ";
      }
      copy(s.iv0.begin(),s.iv0.end(),
	   ostream_iterator<real>(this->out," "));
      this->out << endl;
    }
  }
  
  MTest::~MTest()
  {}

} // end namespace mtest