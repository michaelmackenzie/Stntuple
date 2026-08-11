//
#include <regex>
#include <format>
#include <iostream>

#include "messagefacility/MessageLogger/MessageLogger.h"

#include "canvas/Persistency/Provenance/EventID.h"

#include "Stntuple/mod/MFInterface.hh"

//-----------------------------------------------------------------------------
// Level:     0: debug
//            1: info
//            2: warning
//            3: error
//------------------------------------------------------------------------------
namespace stntuple {
  
void print_(art::EventID* EventID, int Level, const std::string& Message, const std::source_location& location) {

  struct split {
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
      std::vector<std::string> result;
      std::regex re(delimiter);
      std::sregex_token_iterator it(str.begin(), str.end(), re, -1);
      std::sregex_token_iterator end;
      while (it != end) {
        result.push_back(*it++);
      }
      return result;
    }
  } xx;
  
  std::string s;
  if (EventID) s = std::format("event: {}:{}:{} ",EventID->run(),EventID->subRun(),EventID->event());

  std::vector<std::string> ss   = xx.splitString(location.file_name()    ,"/");
  std::vector<std::string> func = xx.splitString(location.function_name(),":");

  if (Level == e_DEBUG) {
    std::cout << s << ss.back() << ":" << location.line() << ":" << func.back() << " : " << Message << std::endl;
  } 
  else if (Level == e_INFO) {
    std::cout << s << ss.back() << ":" << location.line() << ":" << func.back() << " : " << Message << std::endl;
  }
  else if (Level == e_WARNING) {                // warning
    std::cout << "WARNING: " << s << ss.back() << ":" << location.line() << " : " << Message << std::endl;
  }

  else if (Level == e_ERROR) {                // 
    std::cout << "ERROR: " << s << ss.back() << ":" << location.line() << " : " << Message << std:: endl;
  }

  else if (Level == e_SEVERE) {                // 
    std::cout << "SEVERE: " << s << ss.back() << ":" << location.line() << " : " << Message << std::endl;
  }
}
}
