#include <ladspamm-0/world.h>
#include <ladspamm-0/plugin.h>

namespace ladspa {
namespace tools {

struct Debugger 
{

	static void analyseworld(ladspamm::world world);
	static void analyseplugin(ladspamm::plugin_ptr plugin);

};

}
}