/*! 
 * \file  generate_glossary.cxx
 * \brief
 * \author Helfer Thomas
 * \brief 14 mai 2009
 */

// g++ -fPIC -DPIC `tfel-config --includes --libs --utilities --glossary` generate_glossary.cxx -o generate_glossary

// g++ -fPIC -DPIC generate_glossary.cxx  -o generate_glossary -I/home/th202608/codes/tfel/trunk/install/include -L/home/th202608/codes/tfel/trunk/install/lib -lTFELUtilities  -lTFELGlossary && ./generate_glossary && mv Glossary.hxx ../../include/TFEL/Glossary/Glossary.hxx && mv Glossary.cxx ../../src/Glossary/Glossary.cxx && mv PythonGlossary.cxx ../../bindings/python/tfel/Glossary.cxx  &&  ./generate_glossary && pandoc -f markdown+tex_math_single_backslash --toc  glossary-pandoc.txt -o /tmp/glossary.docx

#include<iostream>
#include<stdexcept>
#include<algorithm>
#include<iterator>
#include<fstream>
#include<sstream>
#include<cstdlib>
#include<string>
#include<set>

#include"TFEL/Utilities/CxxTokenizer.hxx"
#include"TFEL/Utilities/StringAlgorithms.hxx"
#include"TFEL/Glossary/GlossaryEntry.hxx"

struct GlossaryTokenizer
  : protected tfel::utilities::CxxTokenizer,
    protected std::set<tfel::glossary::GlossaryEntry>
{

  using std::set<tfel::glossary::GlossaryEntry>::begin;

  using std::set<tfel::glossary::GlossaryEntry>::end;

  using std::set<tfel::glossary::GlossaryEntry>::const_iterator;

  using std::set<tfel::glossary::GlossaryEntry>::empty;

  using std::set<tfel::glossary::GlossaryEntry>::size;

  void execute(const std::string& f)
  {
    using namespace std;
    using namespace tfel::utilities;
    using namespace tfel::glossary;
    CxxTokenizer::TokensContainer::const_iterator p;
    CxxTokenizer::TokensContainer::const_iterator p2;
    CxxTokenizer::TokensContainer::const_iterator pe;
    set<string> keys;
    set<string> names;
    this->treatCharAsString(true);
    this->openFile(f);
    // stripping comment
    this->stripComments();
    p=CxxTokenizer::begin();
    pe = CxxTokenizer::end();
    while(p!=pe){
      string key;
      string name;
      string unit;
      string type;
      string short_description;
      vector<string> description;
      vector<string> notes;
      vector<string> aliases;
      key = CxxTokenizer::readString(p,pe);
      if(!this->isValidIdentifier(key,false)){
	string msg("GlossaryTokenizer : invalid glossary key '"+key+"'");
	throw(runtime_error(msg));
      }
      if(!keys.insert(key).second){
	string msg("GlossaryTokenizer::execute : "
		   "key '"+key+"' already used");
	throw(runtime_error(msg));
      }
      CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute","{",p,pe);
      CxxTokenizer::checkNotEndOfLine("GlossaryTokenizer::execute",p,pe);
      while(p->value!="}"){
	const string k = p->value;
	++p;
	CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute",":",p,pe);
	try{
	  if(k=="alias"){
	    if(!aliases.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "aliases already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    aliases.push_back(CxxTokenizer::readString(p,pe));
	  } else if(k=="aliases"){
	    if(!aliases.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "aliases already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    this->readBlock(aliases,p,pe);
	  } else if(k=="unit"){
	    if(!unit.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "unit already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    unit = CxxTokenizer::readString(p,pe);
	  } else if(k=="name"){
	    if(!name.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "name already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    name = CxxTokenizer::readString(p,pe);
	  } else if(k=="type"){
	    if(!type.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "type already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    type = CxxTokenizer::readString(p,pe);
	  } else if(k=="short_description"){
	    if(!short_description.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "short description already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    short_description = CxxTokenizer::readString(p,pe);
	  } else if(k=="description"){
	    if(!description.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "description already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    this->readBlock(description,p,pe);
	    CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute","}",p,pe);
	  } else if(k=="notes"){
	    if(!notes.empty()){
	      string msg("GlossaryTokenizer::execute : "
			 "notes already defined for key '"+key+"'");
	      throw(runtime_error(msg));
	    }
	    this->readBlock(notes,p,pe);
	    CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute","}",p,pe);
	  } else {
	    string msg("GlossaryTokenizer::execute : "
		       "unknown key '"+k+"'");
	    throw(runtime_error(msg));
	  }
	  CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute",";",p,pe);
	  CxxTokenizer::checkNotEndOfLine("GlossaryTokenizer::execute",p,pe);
	} catch(exception& ex){
	  string msg("GlossaryTokenizer::execute : ");
	  msg += "error while treating key '"+key+"' (";
	  msg += ex.what();
	  msg += ")";
	  throw(runtime_error(msg));
	}
      }
      CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute","}",p,pe);
      CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::execute",";",p,pe);
      // postprocessing
      if(short_description.empty()){
	string msg("GlossaryTokenizer::execute : "
		   "no short_description given for glossary entry '"+key+"'");
	throw(runtime_error(msg));
      }
      if(type.empty()){
	string msg("GlossaryTokenizer::execute : "
		   "no type given for glossary entry '"+key+"'");
	throw(runtime_error(msg));
      }
      if(name.empty()){
	name = key;
      }
      if(!names.insert(name).second){
	string msg("GlossaryTokenizer::execute : "
		   "name '"+name+"' already used. Error while defining key '"+key+"'");
	throw(runtime_error(msg));
      }
      for(vector<string>::const_iterator pa=aliases.begin();pa!=aliases.end();++pa){
	if(!names.insert(*pa).second){
	  string msg("GlossaryTokenizer::execute : "
		     "name '"+*pa+"' already used. Error while defining key '"+key+"'");
	  throw(runtime_error(msg));
	}
      }
      aliases.push_back(name);
      this->insert(GlossaryEntry(key,aliases,unit,type,
				 short_description,
				 description,
				 notes));
    }
  } // end of execute
  
private:
  
  void
  readBlock(std::vector<std::string>& b,
	    CxxTokenizer::TokensContainer::const_iterator& p,
	    const CxxTokenizer::TokensContainer::const_iterator pe)
  {
    using namespace std;
    using namespace tfel::utilities;
    CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::readBlock","{",p,pe);
    CxxTokenizer::checkNotEndOfLine("GlossaryTokenizer::readBlock",p,pe);
    while(p->value!="}"){
      CxxTokenizer::TokensContainer::const_iterator p2 = p;
      b.push_back(CxxTokenizer::readString(p,pe));
      CxxTokenizer::checkNotEndOfLine("GlossaryTokenizer::readBlock",p,pe);
      if(p->value!="}"){
	CxxTokenizer::readSpecifiedToken("GlossaryTokenizer::readBlock",",",p,pe);
	CxxTokenizer::checkNotEndOfLine("GlossaryTokenizer::readBlock",p,pe);
	if(p->flag!=Token::String){
	  string msg("GlossaryTokenizer::readBlock : "
		     "unpected a string, read '"+p->value+"'");
	  throw(runtime_error(msg));
	}
      }
    }
  }

}; // end of struct GlossaryTokenizer

std::string
serialize(const std::vector<std::string>& v,
	  const std::string& d)
{
  using namespace std;
  ostringstream out;
  if(v.empty()){
    out <<  "\"\" /* no '" << d << "' defined */";
  } else {
    vector<string>::const_iterator p;
    for(p=v.begin();p!=v.end();++p){
      if(p!=v.begin()){
	out << "\"@^separator^@\"\n";
      }
      out << "\"" << *p << "\"";
    }
  }
  return out.str();
}

void generateCxxOutput(const GlossaryTokenizer& tokenizer)
{
  using namespace std;
  using namespace tfel::utilities;
  GlossaryTokenizer::const_iterator p;
  ofstream header("Glossary.hxx");
  ofstream src("Glossary.cxx");
  if(!header){
    string msg("generate_glossary : ");
    msg += "can't open file 'Glossary.hxx'";
    throw(runtime_error(msg));    
  }
  if(!src){
    string msg("generate_glossary : ");
    msg += "can't open file 'Glossary.cxx'";
    throw(runtime_error(msg));    
  }
  // gathering all names
  vector<string> names;
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    const vector<string>& n = p->getNames();
    names.insert(names.end(),n.begin(),n.end());
  }
  // writting headers
  header << "/*!" << endl;
  header << " *\\file   Glossary.hxx" << endl;
  header << " *\\author Helfer Thomas" << endl;
  header << " *\\date   09/06/14" << endl;
  header << "*/" << endl;
  header << endl;
  header << "#ifndef __TFEL_GLOSSARY_GLOSSARY_HH__" << endl;
  header << "#define __TFEL_GLOSSARY_GLOSSARY_HH__" << endl;
  header << endl;
  header << "#include\"TFEL/Config/TFELConfig.hxx\"" << endl;
  header << "#include<set>"    << endl;
  header << "#include<vector>" << endl;
  header << "#include<string>" << endl;
  header << endl;
  header << "#include\"TFEL/Glossary/GlossaryEntry.hxx\"" << endl;
  header << endl;
  header << "namespace tfel" << endl;
  header << "{" << endl;
  header << "namespace glossary" << endl;
  header << "{" << endl;
  header << endl;
  header << "/*!\n"
	 << " * \\brief Structure in charge of handling the TFEL Glossary\n"
	 << " */" << endl;
  header << "struct TFELGLOSSARY_VISIBILITY_EXPORT Glossary" << endl;
  header << "{" << endl;
  header << endl;
  header << "/*!\n"
	 << " * \\return the uniq instance of this class (singleton pattern)\n"
	 << " */" << endl;
  header << "static Glossary&" << endl;
  header << "getGlossary();" << endl;
  header << endl;
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    header << "static const GlossaryEntry " << p->getKey() << ";" << endl;
  }
  header << endl;
  header << "/*!\n"
	 << " * \\return true if the glossary contains the given name\n"
	 << " * \\param[in] n : name\n"
	 << " */" << endl;
  header << "bool" << endl;
  header << "contains(const std::string&) const;" << endl;
  header << "/*!\n"
	 << " * \\return all the registred keys\n"
	 << " */" << endl;
  header << "const std::vector<std::string>&" << endl;
  header << "getKeys(void) const;" << endl;
  header << endl;
  header << "protected :" << endl;
  if(!names.empty()){
    header << "//! all glossary names (to initialise glossary entries" << endl;
    header << "static const char * names[" << names.size() << "];" << endl;
  }
  header << "/*!" << endl;
  header << " * \\brief insert a new entry" << endl;
  header << " */" << endl;
  header << "void" << endl;
  header << "insert(const GlossaryEntry&);" << endl;
  header << endl;
  header << "Glossary();" << endl;
  header << endl;
  header << "Glossary(const Glossary&);" << endl;
  header << endl;
  header << "Glossary&" << endl;
  header << "operator=(const Glossary&);" << endl;
  header << endl;
  header << "std::set<GlossaryEntry> entries;" << endl;
  header << endl;
  header << "std::vector<std::string> keys;" << endl;
  header << endl;
  header << "}; // end of struct Glossary" << endl;
  header << endl;
  header << "} // end of namespace glossary" << endl;
  header << endl;
  header << "} // end of namespace tfel" << endl;
  header << endl;
  header << "#endif /* __TFEL_GLOSSARY_GLOSSARY_HH__ */" << endl;
  /* writting src file */
  src << "/*!" << endl;
  src << " *\\file   Glossary.cxx" << endl;
  src << " *\\author Helfer Thomas" << endl;
  src << " *\\date   09/06/14" << endl;
  src << "*/" << endl;
  src << endl;
  src << "#include<stdexcept>" << endl;
  src << "#include<algorithm>" << endl;
  src << endl;
  src << "#include\"TFEL/Glossary/Glossary.hxx\"" << endl;
  src << endl;
  src << "namespace tfel" << endl;
  src << "{" << endl;
  src << "namespace glossary" << endl;
  src << "{" << endl;
  src << endl;
  if(!names.empty()){
    src << "const char* Glossary::names[" << names.size() << "] = {" << endl;
    for(vector<string>::const_iterator pn=names.begin();pn!=names.end();){
      src << '\"' << *pn << '\"';
      if(++pn!=names.end()){
	src << "," << endl;
      }
    }
    src <<  "};" << endl;
  }
  vector<string>::size_type pos = 0;
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    const string& d  = replace_all(serialize(p->getDescription(),"description"),"\\","\\\\");
    const string& n  = replace_all(serialize(p->getNotes(),"notes"),"\\","\\\\");
    const vector<string>& names = p->getNames();
    src << "const GlossaryEntry Glossary::" << p->getKey() << "("
	<< "\"" << replace_all(p->getKey(),"\\","\\\\")  << "\",";
    if(names.size()==1u){
      src << '\"' << names[0u] << "\"," << endl;
    } else {
      if(pos==0){
	src << "Glossary::names,Glossary::names+" << names.size() << "," << endl;
      } else {
	src << "Glossary::names+" << pos << ",Glossary::names+" << pos+names.size() << "," << endl;
      }
    }
    pos+= names.size();
    src<< "\"" << replace_all(p->getUnit(),"\\","\\\\") << "\","
	<< "\"" << replace_all(p->getType(),"\\","\\\\") << "\"," << endl
	<< "\"" << replace_all(p->getShortDescription(),"\\","\\\\") << "\"," << endl
	<< d << "," << endl
	<< n << ");\n" << endl;
  }
  src << "Glossary&" << endl;
  src << "Glossary::getGlossary(void)" << endl;
  src << "{" << endl;
  src << "static Glossary glossary;" << endl;
  src << "return glossary;" << endl;
  src << "} // end of Glossary::getGlossary" << endl;
  src << endl;
  src << "Glossary::Glossary()";
  src << "{" << endl;
  if(!tokenizer.empty()){
    src << "this->keys.reserve(" << tokenizer.size() << ");" << endl;
  }
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    src << "this->insert(Glossary::" << p->getKey() << ");" << endl;
  }
  src << "} // end of Glossary::Glossary" << endl;
  src << endl;
  src << "void" << endl;
  src << "Glossary::insert(const GlossaryEntry& e)" << endl;
  src << "{" << endl;
  src << "using namespace std;" << endl;
  src << "if(!this->entries.insert(e).second){" << endl;
  src << "string msg(\"Glossary::insert : \");" << endl;
  src << "msg += \"'\"+e.getKey()+\"' already declared\";" << endl;
  src << "throw(runtime_error(msg));" << endl;
  src << "}" << endl;
  src << "this->keys.push_back(e.getKey());" << endl;
  src << "} // end of Glossary::insert" << endl;
  src << endl;
  src << "bool" << endl;
  src << "Glossary::contains(const std::string& w) const" << endl;
  src << "{" << endl;
  src << "using namespace std;" << endl;
  src << "set<GlossaryEntry>::const_iterator p;" << endl;
  src << "for(p=this->entries.begin();p!=this->entries.end();++p){" << endl;
  src << "const vector<string>& n = p->getNames();" << endl;
  src << "if(find(n.begin(),n.end(),w)!=n.end()){" << endl;
  src << "return true;" << endl;
  src << "}" << endl;
  src << "}" << endl;
  src << "return false;" << endl;
  src << "} // end of Glossary::contains" << endl;
  src << endl;
  src << "const std::vector<std::string>&" << endl;
  src << "Glossary::getKeys(void) const" << endl;
  src << "{" << endl;
  src << "return this->keys;" << endl;
  src << "} // end of Glossary::contains" << endl;
  src << endl;
  src << "} // end of namespace glossary" << endl;
  src << endl;
  src << "} // end of namespace tfel" << endl;
}

void generateBoostPythonBindings(const GlossaryTokenizer& tokenizer)
{
  using namespace std;
  using namespace tfel::utilities;
  ofstream psrc("PythonGlossary.cxx");
  if(!psrc){
    string msg("generate_glossary : ");
    msg += "can't open file 'PythonGlossary.cxx'";
    throw(runtime_error(msg));    
  }
  GlossaryTokenizer::const_iterator p;
  // python bindings
  psrc << "/*!" << endl;
  psrc << " *\\file   PythonGlossary.cxx" << endl;
  psrc << " *\\author Helfer Thomas" << endl;
  psrc << " *\\date   09/06/14" << endl;
  psrc << "*/" << endl;
  psrc << endl;
  psrc << "#include<boost/python.hpp>" << endl;
  psrc << endl;
  psrc << "#include\"TFEL/Glossary/Glossary.hxx\"" << endl;
  psrc << endl;
  psrc << "void\ndeclareGlossary(void)" << endl;
  psrc << "{" << endl;
  psrc << "using namespace boost;" << endl;
  psrc << "using namespace boost::python;" << endl;
  psrc << "using namespace tfel::glossary;" << endl;
  psrc << "class_<Glossary,noncopyable>(\"Glossary\",no_init)" << endl;
  psrc << ".def(\"getGlossary\",Glossary::getGlossary," << endl
       << "return_value_policy<reference_existing_object>())" << endl
       << ".staticmethod(\"getGlossary\")" << endl
       << ".def(\"contains\",&Glossary::contains)" << endl;
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    psrc << ".def_readonly(\"" << p->getKey() << "\",&Glossary::" << p->getKey();
    const string& d = replace_all(p->getShortDescription(),"\\","\\\\");
    if(!d.empty()){
      psrc << ",\n\"" << d << "\"";
    }
    psrc << ")" << endl;
  }
  psrc << ";" << endl << endl;
  psrc << "}" << endl;
} // end of generateBoostPythonBindings

void generateXMLOutput(const GlossaryTokenizer& tokenizer)
{
  using namespace std;
  ofstream xml("glossary.xml");
  if(!xml){
    string msg("generate_glossary : ");
    msg += "can't open file 'glossary.xml'";
    throw(runtime_error(msg));    
  }
  GlossaryTokenizer::const_iterator p;
  xml << "<?xml version=\"1.0\"?>" << endl;
  xml << "<glossary>" << endl;
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    const vector<std::string>& n = p->getNames();
    xml << "<glossary_entry key=\"" << p->getKey() << "\">" << endl;
    for(vector<string>::const_iterator pn=n.begin();pn!=n.end();++pn){
      xml << "<name>" << *pn << "</name>" << endl;
    }
    xml << "<unit>" << p->getUnit() << "</unit>" << endl;
    xml << "<type>" << p->getType() << "</type>" << endl;
    xml << "<short_description>" << p->getShortDescription() << "</short_description>" << endl;
    xml << "<description>" << endl 
	<< "<![CDATA[" << endl;
    const vector<string>& d =  p->getDescription();
    for(vector<string>::const_iterator p2=d.begin();p2!=d.end();++p2){
      xml << *p2 << endl;
    }
    xml << "]]>"
	<< "\n</description>" << endl;
    xml << "</glossary_entry>" << endl;;
  }
  xml << "</glossary>" << endl;

}

void generatePandocOutput(const GlossaryTokenizer& tokenizer)
{
  using namespace std;
  ofstream doc("glossary-pandoc.txt");
  if(!doc){
    string msg("generatePandocOutput : ");
    msg += "can't open file 'glossary-pandoc.txt'";
    throw(runtime_error(msg));    
  }
  GlossaryTokenizer::const_iterator p;
  doc << "% TFEL Glossary description" << endl;
  doc << "% Helfer Thomas; Bernaud Stéphane" << endl;
  doc << endl;
  doc << "\\newcommand{\\dtot}{\\ensuremath{\\mathrm{d}}}" << endl;
  doc << "\\newcommand{\\paren}[1]{\\ensuremath{{\\displaystyle \\left(#1\\right)}}}" << endl;
  doc << "\\newcommand{\\average}[1]{\\ensuremath{{\\displaystyle \\left\\langle#1\\right\\rangle}}}" << endl;
  doc << "\\newcommand{\\Frac}[2]{\\ensuremath{{\\displaystyle \\frac{\\displaystyle #1}{\\displaystyle #2}}}}" << endl;
  doc << "\\newcommand{\\deriv}[2]{\\ensuremath{{\\displaystyle \\frac{\\displaystyle \\partial #1}{\\displaystyle \\partial #2}}}}" << endl;
  
  for(p=tokenizer.begin();p!=tokenizer.end();++p){
    const vector<string>& n = p->getNames();
    doc << endl;
    doc << "# L'entrée " << p->getKey() << endl << endl;
    if(!p->getShortDescription().empty()){
      doc << "Cette entrée décrit " << p->getShortDescription()
	  << "." << endl <<endl;
    }
    doc << "* noms  : ";
    for(vector<string>::const_iterator pn=n.begin();pn!=n.end();){
      doc << *pn;
      if(++pn!=n.end()){
	doc << ", ";
      }
    }
    doc << endl;
    doc << "* unité : ";
    if(!p->getUnit().empty()){
      doc << "\\(" <<p->getUnit() << "\\)" << endl;
    } else {
      doc << "sans unité" << endl;
    }
    if(p->getType()=="scalar"){
      doc << "* type : scalaire "<< endl;
    } else if(p->getType()=="vector"){
      doc << "* type : vecteur "<< endl;
    } else if(p->getType()=="tensor"){
      doc << "* type : tenseur symétrique" << endl;
    } else {
      doc << "* type : unsupported type" << endl;
    }
    const vector<string>& d =  p->getDescription();
    if(!d.empty()){
      doc << endl << "## Description" << endl << endl;
    }
    for(vector<string>::const_iterator p2=d.begin();p2!=d.end();++p2){
      doc << *p2 << endl;
    }
    const vector<string>& notes =  p->getNotes();
    if(!notes.empty()){
      doc << endl << "## Notes " << endl << endl;
    }
    for(vector<string>::const_iterator p2=notes.begin();p2!=notes.end();++p2){
      doc << *p2 << endl;
    }
    
  }
}

int main(void)
{
  using namespace std;
  GlossaryTokenizer tokenizer;
  tokenizer.execute("glossary.txt");
  generateCxxOutput(tokenizer);  
  generateBoostPythonBindings(tokenizer);  
  generateXMLOutput(tokenizer);  
  generatePandocOutput(tokenizer);  
} // end of main