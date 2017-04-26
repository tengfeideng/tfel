/*!
 * \file   ArgumentParser.hxx
 * \brief    
 * \author THOMAS HELFER
 * \date   09 juin 2016
 * \copyright Copyright (C) 2006-2014 CEA/DEN, EDF R&D. All rights 
 * reserved. 
 * This project is publicly released under either the GNU GPL Licence 
 * or the CECILL-A licence. A copy of thoses licences are delivered 
 * with the sources of TFEL. CEA or EDF may also distribute this 
 * project under specific licensing conditions. 
 */

#ifndef LIB_TFEL_UTILITIES_ARGUMENTPARSER_HXX
#define LIB_TFEL_UTILITIES_ARGUMENTPARSER_HXX

#include<map>
#include<vector>
#include<functional>
#include"TFEL/Config/TFELConfig.hxx"
#include"TFEL/Utilities/Argument.hxx"

namespace tfel
{

  namespace utilities
  {

    /*!
     * class used to parser command line arguments
     */
    struct  TFELUTILITIES_VISIBILITY_EXPORT ArgumentParser
    {
      /*!
       * \brief structure describing an action to be performed when a
       * argument is treated
       */
      struct TFELUTILITIES_VISIBILITY_EXPORT CallBack{
	//! default constructor (deleted)
	CallBack() = delete;
	/*!
	 * \brief constructor
	 * \param[out/in] d_: description
	 * \param[out/in] c_: action
	 * \param[in] b : true if the callback requires an option
	 */
	CallBack(const std::string&,
		 const std::function<void(void)>&,
		 const bool);
	//! move constructor
	CallBack(CallBack&&);
	//! copy constructor (deleted)
	CallBack(const CallBack&);
	//! move assignement
	CallBack& operator = (CallBack&&) = delete;
	//! assignement
	CallBack& operator = (const CallBack&) = delete;
	//! description
	const std::string d;
	//! action performed
	std::function<void(void)> c;
	//! flag, true if the callback has an option
	const bool hasOption = false;
      };
      //! default constructor
      ArgumentParser();
      /*!
       * \brief constructor
       * \param argc : number of arguments given at command line
       * \param argv : arguments list
       */
      ArgumentParser(const int,const char* const* const);
      /*!
       * \brief set the arguments to be parsed
       * \param argc : number of arguments given at command line
       * \param argv : arguments list
       */
      virtual void setArguments(const int,const char* const* const);
      /*!
       * \brief register a new callback
       * \param key         : command line argument name
       * \param f           : callback
       * \param description : description of the command line argument
       *                      (used for the --help) options
       * \param b           : This command line argument can have an option
       */
      virtual void registerCallBack(const std::string&,
				    const CallBack&);
      /*!
       * \brief register a new callback
       * \param key         : command line argument name
       * \param aliasName   : command line argument alias
       * \param f           : callback
       * \param description : description of the command line argument
       *                      (used for the --help) options
       * \param b           : This command line argument can have an option
       */
      virtual void registerCallBack(const std::string&,
				    const std::string&,
				    const CallBack&);
      //! \brief parse arguments using registred methods.
      virtual void parseArguments(void);
      //! destructor
      virtual ~ArgumentParser();
    protected:
      //! a simple alias
      using  CallBacksContainer = std::map<std::string,CallBack>;
      //! a simple alias
      using  AliasContainer = std::map<std::string,std::string> ;
      //! a simple alias
      using  ArgsContainer = std::vector<Argument>;
      /*!
       * \brief register a default callbacks
       *
       * The default callbacks are :
       * - '--help', '-h' which displays the list of the command line
       *    arguments (if the treatHelp method is not overriden if the
       *    derivate class)
       * - '--version', '-v' which displays the current code version
       *    using the getVersionDescription() method of the derivate
       *    class (if the 'treatVersion' method is not overriden if the
       *    derivate class)
       */
      virtual void registerDefaultCallBacks(void);
      /*!
       * \brief method called while parsing unregistred command line
       * arguments.
       *
       * \throw runtime_error
       */
      virtual void treatUnknownArgument(void);
      /*!
       * \brief method associated with the '--help' command line
       * argument.
       *
       * This method uses the getUsageDescription() method to get a
       * more helpfull message.
       *
       * This method stops the execution by calling the exit
       * method.
       */
      virtual void treatHelp(void);
      /*!
       * \brief method associated with the '--version' command line
       * argument
       */
      virtual void treatVersion(void);
      //! container of all the call-backs
      CallBacksContainer callBacksContainer;
      //! container of all the alias
      AliasContainer alias;
      //! container of all the command line arguments
      ArgsContainer  args;
      //! an iterator to the argument being treated
      ArgsContainer::iterator currentArgument;
      //! program name
      std::string programName;
    private:
      //! copy constructor
      ArgumentParser(const ArgumentParser&) = delete;
      //! move constructor
      ArgumentParser(ArgumentParser&&) = delete;
      //! standard assignement
      ArgumentParser& operator=(const ArgumentParser&) = delete;
      //! move assignement
      ArgumentParser& operator=(ArgumentParser&&) = delete;
      //! \brief replaces aliases by their usual names
      virtual void replaceAliases(void);
      //! \brief slip arguments and options
      virtual void stripArguments(void);
    protected:
      //! \return the version of the program being used
      virtual std::string getVersionDescription(void) const = 0;
      //! \return the usage of the program being used
      virtual std::string getUsageDescription(void) const = 0;
    };
  } // end of namespace utilities

} // end of namespace tfel


#endif /* LIB_TFEL_UTILITIES_ARGUMENTPARSER_HXX */
