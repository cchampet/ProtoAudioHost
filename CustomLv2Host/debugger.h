#include <lilv/lilv.h>

#include "lv2/lv2plug.in/ns/ext/port-groups/port-groups.h"
#include "lv2/lv2plug.in/ns/ext/presets/presets.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"

namespace lv2 {
namespace tools {

/**
* Tool to print some data on the terminal.
*/
struct Debugger
{

  static void print_port(LilvWorld* world,
                        const LilvPlugin* p,
                        uint32_t          index,
                        float*            mins,
                        float*            maxes,
                        float*            defaults);

  static void print_plugin(LilvWorld* world,
                          const LilvPlugin* p);

  static void list_plugins(const LilvPlugins* list);
  
};

}
}