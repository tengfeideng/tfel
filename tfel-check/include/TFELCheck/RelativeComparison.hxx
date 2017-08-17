/*!
 * \file RelativeComparison.hxx
 *
 *  Created on: 28 mai 2013
 *      Author: rp238441
 *
 *  \brief class that does a relative comparison between two columns
 *  \class RelativeComparison
 *
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_TFELCHECK_RELATIVECOMPARISON_HXX
#define LIB_TFELCHECK_RELATIVECOMPARISON_HXX

#include "TFELCheck/TFELCheckConfig.hxx"
#include "TFELCheck/Comparison.hxx"

namespace tfel{

namespace check{

  struct TFELCHECK_VISIBILITY_EXPORT RelativeComparison final
    : public Comparison
  {
    RelativeComparison();
    virtual void compare() override;
    virtual ~RelativeComparison();
  };

  } // end of namespace check

} // end of namespace tfel

#endif /* LIB_TFELCHECK_RELATIVECOMPARISON_HXX */
