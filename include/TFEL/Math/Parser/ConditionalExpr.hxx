/*!
 * \file   include/TFEL/Math/Parser/ConditionalExpr.hxx
 * \brief  
 * 
 * \author Thomas Helfer
 * \date   13 jan 2009
 * \copyright Copyright (C) 2006-2018 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_TFEL_CONDITIONALEXPR_HXX
#define LIB_TFEL_CONDITIONALEXPR_HXX 

#include<vector>
#include<string>

#include"TFEL/Math/Parser/Expr.hxx"
#include"TFEL/Math/Parser/LogicalExpr.hxx"

namespace tfel
{

  namespace math
  {

    namespace parser
    {

      struct ConditionalExpr final
	: public Expr
      {
	ConditionalExpr(const std::shared_ptr<LogicalExpr>,
			const std::shared_ptr<Expr>,
			const std::shared_ptr<Expr>);
	double getValue() const override;
	/*!
	 * \return a string representation of the evaluator suitable to
	 * be integrated in a C++ code.
	 * \param[in] m: a map used to change the names of the variables
	 */
	std::string
	getCxxFormula(const std::vector<std::string>&) const override;

	void checkCyclicDependency(std::vector<std::string>&) const override;
	std::shared_ptr<Expr>
	resolveDependencies(const std::vector<double>&) const override;
	std::shared_ptr<Expr>
	differentiate(const std::vector<double>::size_type,
		      const std::vector<double>&) const override;
	std::shared_ptr<Expr> clone(const std::vector<double>&) const override;
	void getParametersNames(std::set<std::string>&) const override;
	std::shared_ptr<Expr>
	createFunctionByChangingParametersIntoVariables(const std::vector<double>&,
							const std::vector<std::string>&,
							const std::map<std::string,
							std::vector<double>::size_type>&) const override;
	~ConditionalExpr() override;
      private:
	ConditionalExpr& operator=(const ConditionalExpr&) = delete;
	ConditionalExpr& operator=(ConditionalExpr&&) = delete;
	const std::shared_ptr<LogicalExpr> c;
	const std::shared_ptr<Expr> a;
	const std::shared_ptr<Expr> b;
      }; // end of struct BinaryOperation

    } // end of namespace parser

  } // end of namespace math

} // end of namespace tfel

#endif /* LIB_TFEL_CONDITIONALEXPR_HXX */
