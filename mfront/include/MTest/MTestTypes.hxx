/*! 
 * \file  mfront/include/MTest/MTestTypes.hxx
 * \brief
 * \author Helfer Thomas
 * \brief 05 avril 2013
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_MFRONT_MTESTTYPES_H_
#define LIB_MFRONT_MTESTTYPES_H_ 

#include<map>
#include<string>
#include<memory>

namespace mfront
{
  
  //! a simple alias
  typedef double real;
  
  //! forward declaration
  struct Evolution;
  // ! a simple alias
  typedef std::shared_ptr<Evolution> EvolutionPtr;
  // ! a simple alias
  typedef std::map<std::string,EvolutionPtr> EvolutionManager;

  /*!
   * \brief possible type of stiffness matrix
   */
  struct MTestStiffnessMatrixType
  {
    //! list of possible matrix type
    enum mtype{
      NOSTIFFNESS,
      ELASTIC,
      SECANTOPERATOR,
      TANGENTOPERATOR,
      CONSISTENTTANGENTOPERATOR,
      // This one is mostly for the umat behaviour
      ELASTICSTIFNESSFROMMATERIALPROPERTIES,
      UNSPECIFIEDSTIFFNESSMATRIXTYPE
    }; // end of struct StiffnessMatrixType
  };

} // end of namespace mfront

#endif /* LIB_MFRONT_MTESTTYPES_H_ */
