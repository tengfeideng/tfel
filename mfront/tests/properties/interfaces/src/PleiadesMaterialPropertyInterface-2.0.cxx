/*
 * \brief PleiadesMaterialPropertyInterface
 *
 * \file PleiadesMaterialPropertyInterface.cpp
 *
 *
 * \author: sb152252
 * \date 26 oct. 2017
 *
 * Copyright © CEA 2017
 */

#include <fstream>
#include <algorithm>
#include <stdexcept>

#include "TFEL/Raise.hxx"
#include "TFEL/System/System.hxx"
#include "MFront/DSLUtilities.hxx"
#include "MFront/MFrontHeader.hxx"
#include "MFront/FileDescription.hxx"
#include "MFront/TargetsDescription.hxx"
#include "MFront/MaterialPropertyDescription.hxx"
#include "MFront/MaterialPropertyInterfaceProxy.hxx"
#include "MFront/PleiadesMaterialPropertyInterface-2.0.hxx"

namespace mfront {

PleiadesMaterialPropertyInterface::PleiadesMaterialPropertyInterface() = default;
  
std::string PleiadesMaterialPropertyInterface::getName(void) {
  return "pleiades-2.0";
  // TODO: make a PleiadesMaterialPropertyInterface.cxx.in and use @PACKAGE_VERSION@
}

std::pair<bool, PleiadesMaterialPropertyInterface::const_iterator>
PleiadesMaterialPropertyInterface::treatKeyword(const std::string& k, const std::vector<std::string>& i,
                                         const_iterator current, const const_iterator) {
  tfel::raise_if(std::find(i.begin(), i.end(), "pleiades-2.0") != i.end(),
                 "PleiadesMaterialPropertyInterface::treatKeyword: "
                 "unsupported key '" +
                     k + "'");
  return {false, current};
}  // end of treatKeyword

void PleiadesMaterialPropertyInterface::getTargetsDescription(
    TargetsDescription& d, const MaterialPropertyDescription& mpd) const {
  const auto lib = "libPleiades" + getMaterialLawLibraryNameBase(mpd);
  const auto name = mpd.material.empty() ? mpd.className : mpd.material + "_" + mpd.className;
  const auto hn = "include/Pleiades/Metier/MaterialProperty/" + name + "-pleiades.hh";
  d[lib].ldflags.push_back("-lm");
  d[lib].cppflags.push_back("`pleiades-config --includes`\n");
  d[lib].sources.push_back(name + "-pleiades.cpp");
  d[lib].epts.push_back(name);
  d.headers.push_back(hn.substr(8));
}  // end of PleiadesMaterialPropertyInterface::getTargetsDescription

void PleiadesMaterialPropertyInterface::writeOutputFiles(const MaterialPropertyDescription& mpd,
                                                  const FileDescription& fd) const {
  this->writeHeaderFile(mpd, fd);
  this->writeSrcFile(mpd, fd);
}  // end of PleiadesMaterialPropertyInterface::writeOutputFiles

void PleiadesMaterialPropertyInterface::writeHeaderFile(const MaterialPropertyDescription& mpd,
                                                 const FileDescription& fd) const {
  using namespace tfel::system;
  const auto name = (mpd.material.empty()) ? mpd.className : mpd.material + "_" + mpd.className;
  systemCall::mkdir("include/Pleiades");
  systemCall::mkdir("include/Pleiades/Metier");
  systemCall::mkdir("include/Pleiades/Metier/MaterialProperty");

  const auto fn = "include/Pleiades/Metier/MaterialProperty/" + name + "-pleiades.hh";
  std::ofstream os(fn);

  tfel::raise_if(!os,
                 "MaterialLawParser::writeOutputFiles: "
                 "unable to open '" +
                     fn + "' for writing output file.");
  os.exceptions(std::ios::badbit | std::ios::failbit);

  os << "/*!\n"
     << "* \\file   " << fn << '\n' << "* \\brief  "
     << "this file declares the " << name << " MaterialLaw.\n"
     << "*         File generated by " << MFrontHeader::getVersionName() << " "
     << "version " << MFrontHeader::getVersionNumber() << '\n';
  if (!fd.authorName.empty()) {
    os << "* \\author " << fd.authorName << '\n';
  }
  if (!fd.date.empty()) {
    os << "* \\date   " << fd.date << '\n';
  }
  if (!fd.description.empty()) {
    os << fd.description << '\n';
  }
  os << " */\n\n"
     << "#ifndef _PLEIADES_" << makeUpperCase(name) << "_HH\n"
     << "#define _PLEIADES_" << makeUpperCase(name) << "_HH\n\n";
  //  writeExportDirectives(os);

  os << "#include <cmath>\n";
  if (!mpd.includes.empty()) {
    os << mpd.includes << '\n';
  }
  os << "#include \"Pleiades/Metier/MaterialProperty/MaterialPropertyBase.hh\"\n\n"
     << "namespace Pleiades\n{\n\n"
     << "class " << name << '\n' << ": public MaterialPropertyBase\n"
     << "{\n\n"
     << "public:\n" << name << "();\n"
     << "void declare();\n "
     << "void compute();\n "
     << "double operator()(";
  if (!mpd.inputs.empty()) {
    for (auto p4 = mpd.inputs.begin(); p4 != mpd.inputs.end();) {
      os << "const double& " << p4->name;
      if ((++p4) != mpd.inputs.end()) {
        os << ",";
      }
    }
  } else {
    os << "void";
  }
  os << ") const;\n\n";
  // Disable copy constructor and assignement operator
  os << "private:\n\n"
     << "//! Copy constructor (disabled)\n" << name << "(const " << name << "&);\n\n"
     << "//! Assignement operator (disabled)\n" << name << "&\n"
     << "operator=(const " << name << "&);\n\n"
     << "}; // end of class " << name << "\n\n"
     << "} // end of namespace Pleiades\n\n"
     << "#endif /* _PLEIADES_" << makeUpperCase(name) << "_HH */\n";
  os.close();
}  // end of PleiadesMaterialPropertyInterface::writeHeaderFile()

void PleiadesMaterialPropertyInterface::writeSrcFile(const MaterialPropertyDescription& mpd,
                                              const FileDescription& fd) const {
  auto throw_if = [](const bool b, const std::string& m) {
    tfel::raise_if(b, "PleiadesMaterialPropertyInterface::writeSrcFile: " + m);
  };
  const auto name = (mpd.material.empty()) ? mpd.className : mpd.material + "_" + mpd.className;
  const auto fn = "src/" + name + "-pleiades.cpp";
  std::ofstream os(fn);
  throw_if(!os, "unable to open '" + fn + "' for writing output file.");
  os.exceptions(std::ios::badbit | std::ios::failbit);

  os << "/*!\n"
     << "* \\file   " << fn << '\n' << "* \\brief  "
     << "this file implements the " << name << " MaterialLaw.\n"
     << "*         File generated by " << MFrontHeader::getVersionName() << " "
     << "version " << MFrontHeader::getVersionNumber() << '\n';
  if (!fd.authorName.empty()) {
    os << "* \\author " << fd.authorName << '\n';
  }

  if (!fd.date.empty()) {
    os << "* \\date   " << fd.date << '\n';
  }

  os << " */\n\n"
     << "#include \"Pleiades/Metier/MaterialProperty/" << name << "-pleiades.hh\"\n\n"
     << "#include <string>\n"
     << "#include <cmath>\n\n"
     << "#include \"Pleiades/Examplars/ClassProxy.hh\"\n"
     << "#include \"Pleiades/Metier/Field/FieldApply.hh\"\n"
     << "#include \"Pleiades/Exceptions/pexceptions.hh\"\n"
     << "#include \"Pleiades/Metier/Glossary/Glossary.hh\"\n"
     << "#include \"Pleiades/Utilitaires/tools.hh\"\n"
     << "namespace Pleiades {\n\n" << name << "::" << name << "(){}\n\n";

  // declare method
  os << "void " << name << "::declare() {\n";

  std::string oname;

  if (mpd.output.hasGlossaryName()) {
    oname = "Glossary::" + mpd.output.getExternalName();
  } else if (mpd.output.hasEntryName()) {
    oname = "\"" + mpd.output.getExternalName() + "\"";
  } else {
    os << "ARCH_WARNING(\"Glossary name not defined for output field: " << mpd.output.name << " in "
       << name << "\");\n";
    oname = "\"" + mpd.output.name + "\"";
  }

  os << "declareField<double>(\"OutputField\", OUT);\n"
     << "setFieldName(\"OutputField\", " << oname << ");\n";

  for (const auto& i : mpd.inputs) {
    const auto iname = [&i] {
      if (i.hasGlossaryName()) {
        return "Glossary::" + i.getExternalName();
      } else if (i.hasEntryName()) {
        return "\"" + i.getExternalName() + "\"";
      }

      return "\"" + i.name + "\"";
    }();
    os << "declareField<double>(\"" << i.name << "\", "
       << "IN"
       << ");\n"
       << "setFieldName(\"" << i.name << "\", " << iname << ");\n";
  }

  os << "}\n\n";

  // Compute
  os << "void " << name << "::compute() {\n"
     << "  apply(*this, getOutputField<double>(\"OutputField\")";
  for (const auto& i : mpd.inputs) {
    os << ", getField<double>(\"" << i.name << "\")";
  }
  os << ");\n"
     << "} // end of " << name << "::compute\n\n";
  // Law
  os << "double " << name << "::operator()(";
  if (!mpd.inputs.empty()) {
    for (auto pi = mpd.inputs.begin(); pi != mpd.inputs.end();) {
      os << "const double& " << pi->name;
      if ((++pi) != mpd.inputs.end()) {
        os << ",";
      }
    }
  } else {
    os << "void";
  }
  os << ") const {\n"
     << "using namespace std;\n";

  // material laws
  writeMaterialLaws(os, mpd.materialLaws);
  // static variables
  writeStaticVariables(os, mpd.staticVars, fd.fileName);
  if (!mpd.parameters.empty()) {
    for (const auto& p : mpd.parameters) {
      throw_if(!p.hasAttribute(VariableDescription::defaultValue),
               "internal error (can't find value of "
               "parameter " +
                   p.name + ")");
      os << "static constexpr double " << p.name << " = "
         << p.getAttribute<double>(VariableDescription::defaultValue) << ";\n";
    }
  }

  os << "double " << mpd.output.name << ";\n";
  if ((hasPhysicalBounds(mpd.inputs)) || (hasBounds(mpd.inputs))) {
    os << "#ifndef NO_PLEIADES_BOUNDS_CHECK\n";
  }

  auto get_ename = [](const VariableDescription& v) {
    if (v.hasGlossaryName()) {
      return "Glossary::" + v.getExternalName();
    } else if (v.hasEntryName()) {
      return "\"" + v.getExternalName() + "\"";
    }
    return "\"" + v.name + "\"";
  };

  if (hasPhysicalBounds(mpd.inputs)) {
    os << "// treating physical bounds\n";
    for (const auto& i : mpd.inputs) {
      if (!i.hasPhysicalBounds()) {
        continue;
      }
      const auto& b = i.getPhysicalBounds();
      const auto fname = get_ename(i);
      if (b.boundsType == VariableBoundsDescription::LOWER) {
        os << "if(" << i.name << " < " << b.lowerBound << "){\n"
           << "string msg (\"" << name << "::compute : \");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is below its physical lower bound \";\n"
           << "msg += \"(\" + toString(" << b.lowerBound << ") + \")\";\n"
           << "PLEIADES_THROW(msg);\n"
           << "}\n";
      } else if (b.boundsType == VariableBoundsDescription::UPPER) {
        os << "if(" << i.name << " > " << b.upperBound << "){\n"
           << "string msg (\"" << name << "::compute : \");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is over its physical upper bound \";\n"
           << "msg += \"(\" + toString(" << b.upperBound << ") + \")\";\n"
           << "PLEIADES_THROW(msg);\n"
           << "}\n";
      } else {
        os << "if((" << i.name << " < " << b.lowerBound << ")||"
           << "(" << i.name << " > " << b.upperBound << ")){\n"
           << "if(" << i.name << " < " << b.lowerBound << "){\n"
           << "string msg (\"" << name << "::compute : \");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is below its physical lower bound \";\n"
           << "msg += \"[\" + toString(" << b.lowerBound << ") + \"; \" + toString(" << b.upperBound
           << ") + \"].\";\n"
           << "PLEIADES_THROW(msg);\n"
           << "} else {\n"
           << "string msg (\"" << name << "::compute : \");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is over its physical upper bound \";\n"
           << "msg += \"[\" + toString(" << b.lowerBound << ") + \"; \" + toString(" << b.upperBound
           << ") + \"].\";\n"
           << "PLEIADES_THROW(msg);\n"
           << "}\n"
           << "}\n";
      }
    }
  }

  if (hasBounds(mpd.inputs)) {
    os << "// treating standard bounds\n";
    for (const auto& i : mpd.inputs) {
      if (!i.hasBounds()) {
        continue;
      }
      const auto fname = get_ename(i);
      const auto& b = i.getBounds();
      if (b.boundsType == VariableBoundsDescription::LOWER) {
        os << "if(" << i.name << " < " << b.lowerBound << "){\n"
           << "string msg(\"" << name << "::compute : value of\");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is below its lower bound \";\n"
           << "msg += \"(\" + toString(" << b.lowerBound << ") + \")\";\n"
           << "treatOutOfBounds(msg);\n"
           << "}\n";
      } else if (b.boundsType == VariableBoundsDescription::UPPER) {
        os << "if(" << i.name << " > " << b.upperBound << "){\n"
           << "string msg(\"" << name << "::compute : value of\");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is over its upper bound \";\n"
           << "msg += \"(\" + toString(" << b.upperBound << ") + \")\";\n"
           << "treatOutOfBounds(msg);\n"
           << "}\n";
      } else {
        os << "if((" << i.name << " < " << b.lowerBound << ")||"
           << "(" << i.name << " > " << b.upperBound << ")){\n"
           << "string msg(\"" << name << "::compute : value of\");\n"
           << "msg += " << fname << ";\n"
           << "msg += \" is out of bounds \";\n"
           << "msg += \"[\" + toString(" << b.lowerBound << ") + \"; \" + toString(" << b.upperBound
           << ") + \"].\";\n"
           << "treatOutOfBounds(msg);\n"
           << "}\n";
      }
    }
  }
  if ((hasPhysicalBounds(mpd.inputs)) || (hasBounds(mpd.inputs))) {
    os << "#endif /* NO_PLEIADES_BOUNDS_CHECK */\n";
  }
  os << mpd.f.body << "return " << mpd.output.name << ";\n"
     << "} // end of " << name << "::law\n\n"
     << "GENERATE_PROXY(IMaterialProperty," << name << ");\n"
     << "} // end of namespace Pleiades\n";
  os.close();
}  // end of PleiadesMaterialPropertyInterface::writeSrcFile()

PleiadesMaterialPropertyInterface::~PleiadesMaterialPropertyInterface() = default;
  
MaterialPropertyInterfaceProxy<PleiadesMaterialPropertyInterface> pleiadesLawProxy;
}  // end of namespace mfront
