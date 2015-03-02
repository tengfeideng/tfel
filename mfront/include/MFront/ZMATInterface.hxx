/*! 
 * \file  mfront/include/MFront/ZMATInterface.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 23 mai 2014
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_MFRONT_MFRONTZMATINTERFACE_H_
#define LIB_MFRONT_MFRONTZMATINTERFACE_H_ 

#include<set>
#include<string>

#include"TFEL/Utilities/CxxTokenizer.hxx"

#include"MFront/SupportedTypes.hxx"
#include"MFront/BehaviourDescription.hxx"
#include"MFront/AbstractBehaviourInterface.hxx"

namespace mfront{

  /*!
   * This class provides some helper function for behaviours
   * interfaces based on the umat standard
   */
  struct ZMATInterface
    : public SupportedTypes,
      public AbstractBehaviourInterface
  {
    //! a simple alias
    typedef tfel::material::ModellingHypothesis ModellingHypothesis;
    //! a simple alias
    typedef ModellingHypothesis::Hypothesis Hypothesis;

    /*!
     * \return the name of the interface
     */
    static std::string
    getName(void);

    ZMATInterface();

    /*!
     * set if dynamically allocated arrays are allowed
     * \param[in] b : boolean
     */
    virtual void
    allowDynamicallyAllocatedArrays(const bool);

    virtual std::pair<bool,tfel::utilities::CxxTokenizer::TokensContainer::const_iterator>
    treatKeyword(const std::string&,
		 tfel::utilities::CxxTokenizer::TokensContainer::const_iterator,
		 const tfel::utilities::CxxTokenizer::TokensContainer::const_iterator);
    /*!
     * \return true if the interface handles the given modelling hypothesis
     * \param[in] h  : modelling hypothesis
     * \param[in] mb : mechanical behaviour description
     */
    virtual bool
    isModellingHypothesisHandled(const Hypothesis,
				 const BehaviourDescription&) const;
    
    virtual std::set<Hypothesis>
    getModellingHypothesesToBeTreated(const BehaviourDescription&) const;
    /*!
     * write interface specific includes
     * \param[in] out : output file
     * \param[in] mb  : mechanical behaviour description
     */
    virtual void 
    writeInterfaceSpecificIncludes(std::ofstream&,
				   const BehaviourDescription&) const;

    virtual void
    exportMechanicalData(std::ofstream&,
			 const Hypothesis,
			 const BehaviourDescription&) const;
    /*!
     * write the behaviour constructor associated with the law
     * \param[in] behaviourFile           : output file
     * \param[in] mb                      : mechanical behaviour description
     * \param[in] initStateVarsIncrements : constructor part assigning
     *                                      default value (zero) to state
     *                                      variable increments
     */
    virtual void
    writeBehaviourConstructor(std::ofstream&,
			      const BehaviourDescription&,
			      const std::string&) const;

    virtual void
    writeBehaviourDataConstructor(std::ofstream&,
				  const Hypothesis,
				  const BehaviourDescription&) const;

    /*!
     * write the initialisation of the driving variables
     * \param[in] behaviourFile : output file
     * \param[in] mb            : mechanical behaviour description
     */
    virtual void 
    writeBehaviourDataMainVariablesSetters(std::ofstream&,
					   const BehaviourDescription&) const;
    
    virtual void
    writeIntegrationDataConstructor(std::ofstream&,
				    const Hypothesis,
				    const BehaviourDescription&) const;
    /*!
     * write the initialisation of the driving variables
     * \param[in] behaviourFile : output file
     * \param[in] mb            : mechanical behaviour description
     */
    virtual void 
    writeIntegrationDataMainVariablesSetters(std::ofstream&,
					     const BehaviourDescription&) const;
    /*!
     * \brief write output files
     * \param[in] mb        : mechanical behaviour description
     * \param[in] fd        : mfront file description
     */
    virtual void
    endTreatement(const BehaviourDescription&,
		  const FileDescription&) const;
    /*!
     * \param[in] mb : mechanical behaviour description
     */
    virtual std::map<std::string,std::vector<std::string> >
    getGlobalIncludes(const BehaviourDescription&) const;
    /*!
     * \param[in] mb : mechanical behaviour description
     */
    virtual std::map<std::string,std::vector<std::string> >
    getGlobalDependencies(const BehaviourDescription&) const;

    /*!
     * \param[in] mb : mechanical behaviour description
     */
    virtual std::map<std::string,std::vector<std::string> >
    getGeneratedSources(const BehaviourDescription&) const;

    /*!
     * \param[in] mb : mechanical behaviour description
     */
    virtual std::vector<std::string>
    getGeneratedIncludes(const BehaviourDescription&) const;

    /*!
     * \param[in] mb : mechanical behaviour description
     */
    virtual std::map<std::string,std::vector<std::string> >
    getLibrariesDependencies(const BehaviourDescription&) const;

    virtual void
    reset(void);
    
    virtual ~ZMATInterface();

  protected:

    /*!
     * \brief write behaviour initialisation for the given hypothesis
     * \param[out] out : output file
     * \param[in]  mb  : mechancial behaviour description
     * \param[in]  h   : modelling hypothesis
     */
    void
    writeBehaviourInitialisation(std::ostream&,
				 const BehaviourDescription&,
				 const ZMATInterface::Hypothesis) const;
    /*!
     * \brief write parameters initialisation for the given hypothesis
     * \param[out] out : output file
     * \param[in]  mb  : mechancial behaviour description
     * \param[in]  h   : modelling hypothesis
     */
    void
    writeParametersInitialisation(std::ostream&,
				  const BehaviourDescription&,
				  const ZMATInterface::Hypothesis) const;
    /*!
     * \brief write material properties initialisation for the given hypothesis
     * \param[out] out : output file
     * \param[in]  mb  : mechancial behaviour description
     * \param[in]  h   : modelling hypothesis
     */
    void
    writeMaterialPropertiesInitialisation(std::ostream&,
					  const BehaviourDescription&,
					  const ZMATInterface::Hypothesis) const;
    /*!
     * \brief write behaviour initialisation for the given hypothesis
     * \param[out] out : output file
     * \param[in]  mb  : mechancial behaviour description
     * \param[in]  h   : modelling hypothesis
     */
    void
    writeCallMFrontBehaviour(std::ostream&,
			     const BehaviourDescription&,
			     const ZMATInterface::Hypothesis) const;

    
  }; //end of struct ZMATInterface

} // end of namespace mfront

#endif /* LIB_MFRONT_MFRONTZMATINTERFACE_H_ */
