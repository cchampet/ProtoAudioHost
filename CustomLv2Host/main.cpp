#include <sndfile.hh>
#include <vector>
#include <exception>
#include <iostream>

#include "Node.h"
#include "Lv2Graph.h"

#include "lv2/lv2plug.in/ns/ext/presets/presets.h"

/**
* Proto which read test.wav, process a gain of 5db and a limiter which produce delay of 0.15s, and write testVFX.wav
*/
int main(int argc, char** argv)
{
  SndfileHandle infile;
  SndfileHandle outfile;
  try
  {
    infile = SndfileHandle( "../data/underwater.wav" );
  }
  catch(std::exception& e)
  {
    std::cout << "Exception when try to read the file :  " << e.what() << std::endl;
    return 1;
  }
  
  const int format = infile.format();
  const int numChannels = infile.channels();
  const int samplerate = infile.samplerate();
  // const size_t nbFrames = infile.frames();
  // const size_t nbSamples = nbFrames * numChannels;
  //const float timeToRead = nbFrames / (float)samplerate; //in seconds

  outfile = SndfileHandle( "../data/underwaterVFX.wav" , SFM_WRITE, format , numChannels , samplerate );

  // create graph
  sound::Lv2Graph graph;
  graph.createAudioBuffer( samplerate );
  
  // add nodes to the graph
  sound::Node& gain = graph.addNode( "http://lv2plug.in/plugins/eg-amp", samplerate );
  sound::Node& limiter = graph.addNode("http://plugin.org.uk/swh-plugins/lookaheadLimiterConst", samplerate);
  sound::Node& reverb = graph.addNode("http://plugin.org.uk/swh-plugins/gverb", samplerate);
  
  // connect ports
  graph.connect( graph.getAudioBufferInput(), reverb );
  //graph.connect( gain, reverb );
  graph.connect( reverb, graph.getAudioBufferOutput() );
  
  // update params
  gain.setParam( "gain", 0.f );
  limiter.setParam( "delay_s", 0.15f );
  limiter.setParam( "limit", -10.f );
  reverb.setParam( "revtime", 2.f );
  
  // setup
  graph.setUp();
  
  while( 1 )
  {
    // read on disk
    size_t currentReadedSamples = infile.read( &graph.getAudioBufferInput()[0], samplerate );
    std::cout << "readedSamples : " << currentReadedSamples << std::endl;
    
	if( ! currentReadedSamples )
      break;
	
    // process graph
    for (size_t i = 0; i < currentReadedSamples; ++i) 
    {
      graph.processFrame( &graph.getAudioBufferInput()[i], &graph.getAudioBufferOutput()[i] );
    }
	
    // write on disk
    outfile.write( &graph.getAudioBufferOutput()[0], currentReadedSamples );
  }

  return 0;
}


// TEST
//sound::Node& reverb = graph.addNode("http://calf.sourceforge.net/plugins/Reverb", samplerate);
// test - presets
/*
LilvNode* label_pred = lilv_new_uri(graph.getWorld( )->me, LILV_NS_RDFS "label");
LilvNode* preset_class = lilv_new_uri(graph.getWorld( )->me, LV2_PRESETS__Preset);

LilvNodes* presets = lilv_plugin_get_related( reverb.getPlugin( ), preset_class );
LILV_FOREACH(nodes, i, presets) 
{
  const LilvNode* preset = lilv_nodes_get(presets, i);
  lilv_world_load_resource(graph.getWorld( )->me, preset);
  LilvNodes* titles = lilv_world_find_nodes(graph.getWorld( )->me, preset, label_pred, NULL);
  if (titles) 
  {
	const LilvNode* title = lilv_nodes_get_first(titles);
	LilvNode* port_value = lilv_port_get( reverb.getPlugin( ), reverb.getPlugin( ).get_port_by_index( 7 ).me, 
			lilv_new_uri(graph.getWorld( )->me, "http://lv2plug.in/ns/ext/port-props#hasStrictBounds") );
	std::cout << "Preset : " << lilv_node_as_string(title) << std::endl;
	std::cout << "Value on port #7 : " << lilv_node_as_string(port_value) << std::endl;

	LilvNodes* extendedData = lilv_plugin_get_extension_data( reverb.getPlugin( ) );
	LILV_FOREACH(nodes, j, extendedData) 
	{
	  const LilvNode* data = lilv_nodes_get(extendedData, j);
	  std::cout << "Extended data : " << lilv_node_as_string(data) << std::endl;
	}
  }
}
lilv_nodes_free(presets);
*/

// test - scale point
/*
LilvScalePoints * scalePoints = lilv_port_get_scale_points ( reverb.getPlugin( ), reverb.getPlugin( ).get_port_by_index( 9 ).me );
LilvNode* portName = lilv_port_get_name ( reverb.getPlugin( ), reverb.getPlugin( ).get_port_by_index( 9 ).me );
std::cout << "portName : " << lilv_node_as_string(portName) << std::endl;

LILV_FOREACH(scale_points, i, scalePoints) 
{
  const LilvScalePoint* scalePoint = lilv_scale_points_get(scalePoints, i);
  std::cout << lilv_node_as_string(lilv_scale_point_get_label(scalePoint)) << " " << 
		  lilv_node_as_string(lilv_scale_point_get_value(scalePoint)) << std::endl;
}
*/

// test - ui (need to load the binary to test the UI of a plugin...)
/*
LilvUIs* reverb_uis = lilv_plugin_get_uis ( reverb.getPlugin( ) );
std::cout << 	lilv_uis_size(reverb_uis) << std::endl;
LILV_FOREACH(uis, i, reverb_uis) 
{
  const LilvUI* reverb_ui = lilv_uis_get(reverb_uis, i);
  const LilvNode * node = lilv_ui_get_uri (reverb_ui);
  const LilvNode * buddle_uri = lilv_ui_get_binary_uri(reverb_ui);

  std::cout << lilv_node_as_string(node) << std::endl;
  std::cout << lilv_node_as_string(buddle_uri) << std::endl;
}
*/

// test - plugin class
/*
const LilvPluginClasses* pluginClasses = lilv_world_get_plugin_classes ( graph.getWorld( )->me );
LILV_FOREACH(plugin_classes, i, pluginClasses) 
{
  const LilvPluginClass *	pluginClass = lilv_plugin_classes_get (pluginClasses, i);
  const LilvNode * nodeUri = lilv_plugin_class_get_uri(pluginClass);
  const LilvNode * nodeLabel = lilv_plugin_class_get_label(pluginClass);
  std::cout << lilv_node_as_string(nodeUri) << "  /  " << 
		  lilv_node_as_string(nodeLabel) << std::endl;
}
*/