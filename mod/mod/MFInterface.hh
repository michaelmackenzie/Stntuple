//-----------------------------------------------------------------------------
//  Aug 2026 P.Murat: interface to message facility
//-----------------------------------------------------------------------------
#ifndef StntupleUtilities_hh
#define StntupleUtilities_hh

#include <string>
#include <source_location>

namespace art {
  class EventID;
};

namespace stntuple {

  enum {
    e_DEBUG   = 0,
    e_INFO    = 1,
    e_WARNING = 2,
    e_ERROR   = 3,
    e_SEVERE  = 4,
  };
  
  // if EventID == nullptr, dont print the run:subrun:event
  
  void print_(art::EventID* EventID, int Level, const std::string& Message,
              const std::source_location& location  = std::source_location::current());

}
#endif
