/*!
 * \file   src/Math/TensorTag.cxx
 * 
 * \brief    
 * \author Helfer Thomas
 * \date   09 Mar 2007
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#include<string>
#include"TFEL/Math/Tensor/TensorConcept.hxx"

namespace tfel{

  namespace math{

    std::string 
    TensorTag::getName(void)
    {
      return std::string("TensorTag");
    } // end of getName()
    
  } // end of namespace math

} // end of namespace tfel