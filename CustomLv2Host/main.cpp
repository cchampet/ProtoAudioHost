#include <sndfile.hh>
#include <vector>
#include <exception>
#include <iostream>

#include "Node.h"
#include "Lv2Graph.h"


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
  const size_t nbFrames = infile.frames();
  const size_t nbSamples = nbFrames * numChannels;
  //const float timeToRead = nbFrames / (float)samplerate; //in seconds
  
  std::vector< short > audioIn;
  std::vector< short > audioOut;
  try
  {
    audioIn = std::vector< short > ( nbSamples, 0 );
    audioOut = std::vector< short > ( nbSamples, 0 );
  }
  catch(std::exception& e)
  {
    std::cout << "Exception when try to allocate audio buffers : " << e.what() << std::endl;
    return 1;
  }

  outfile = SndfileHandle( "../data/underwaterVFX.wav" , SFM_WRITE, format , numChannels , samplerate );

  sound::Lv2Graph graph;
  // add nodes to the graph
  sound::Node& gain = graph.addNode( "http://lv2plug.in/plugins/eg-amp", samplerate );
  sound::Node& limiter = graph.addNode("http://plugin.org.uk/swh-plugins/lookaheadLimiterConst", 96000);
  
  // connect ports
  graph.connect( gain, limiter );
  
  // update params
  gain.setParam( "gain", 0.f );
  limiter.setParam( "delay_s", 0.15f );
  
  // TEST
  sound::Node& reverb = graph.addNode("http://calf.sourceforge.net/plugins/Reverb", samplerate);
  // test - ui (need to load the binary to test the UI of a plugin...)
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
  
  size_t readedSamples = 0;
  while( 1 )
  {
    if ( readedSamples == nbSamples )
      break;
    
    // read on disk
    size_t currentReadedSamples = infile.read( &audioIn[readedSamples], samplerate );
    std::cout << "readedSamples : " << currentReadedSamples << std::endl;
    
	if( ! currentReadedSamples )
      break;
	
    // process graph
    for (size_t i = 0; i < currentReadedSamples; ++i) 
    {
      graph.processFrame( &audioIn[ readedSamples + i ], &audioOut[ readedSamples + i ] );
    }
	
    // write on disk
    outfile.write( &audioOut[readedSamples], currentReadedSamples );
    
    readedSamples += currentReadedSamples;
  }

  return 0;
}
