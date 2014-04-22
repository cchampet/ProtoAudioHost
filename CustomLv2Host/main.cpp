#include <sndfile.hh>
#include <cstring> //memcpy
#include <string>
#include <math.h> 
#include <vector>
#include <algorithm> //find

// Lilv wrapper C++
#include <lilv/lilvmm.hpp>
// Tools to print Lv2 data
#include "debugger.h"

// #define AUDIO_BUFFER_AUDIO_INPUT    0
// #define AUDIO_BUFFER_AUDIO_OUTPUT   1
// #define AUDIO_BUFFER_CONTROL_INPUT  2
// #define AUDIO_BUFFER_CONTROL_OUTPUT 3

class Lv2Graph
{
public:
  Lv2Graph()
  {
    pWorld = new Lilv::World();
    pWorld->load_all();
  }

  ~Lv2Graph()
  {
    delete pWorld;
  }

  Lilv::Instance* addNode( const std::string pluginURI, int samplerate )
  {
    Lilv::Node plugin_uri = pWorld->new_uri( &pluginURI[0] );
    Lilv::Plugin plugin = ( ( Lilv::Plugins )pWorld->get_all_plugins() ).get_by_uri( plugin_uri );
        
    Lilv::Instance* instance = Lilv::Instance::create( plugin, samplerate, NULL );
    if( ! instance )
      return NULL;
    
    plugins.push_back( plugin );
    nodes.push_back( instance );

    instance->activate();

    lv2::tools::Debugger::print_plugin( pWorld->me, plugin.me );
    
    return instance;
  }

  void connect( Lilv::Instance* node1 )
  {
    std::vector<Lilv::Instance*>::iterator it = std::find(nodes.begin(), nodes.end(), node1);
    if (it == nodes.end())
    {
      exit(1);
    } 
    int indexPlugin = std::distance(nodes.begin(), it);

    unsigned int numPorts = plugins.at(indexPlugin).get_num_ports();
    audioBuffers = std::vector< std::vector< short > >( numPorts );
    controlInputBuffer = std::vector< short >( numPorts, 0 );
    controlOutputBuffer = std::vector< short >( numPorts, 0 );

    // create some nodes that represent certain features: Input / Output, Audio, Control (other exist : MIDI, Event...)
    Lilv::Node audio    = pWorld->new_uri( LILV_URI_AUDIO_PORT );
    Lilv::Node control  = pWorld->new_uri( LILV_URI_CONTROL_PORT );
    Lilv::Node input    = pWorld->new_uri( LILV_URI_INPUT_PORT );
    Lilv::Node output   = pWorld->new_uri( LILV_URI_OUTPUT_PORT );

    for (unsigned int i = 0; i < numPorts; i++)
    {
      Lilv::Port port = plugins.at(indexPlugin).get_port_by_index(i);

      if( port.is_a( audio ) && port.is_a( input ) )
      {
        audioBuffers[i] = std::vector< short >(audioBufferSize, 0);
        node1->connect_port( i, &audioBuffers[i][0]);
        idInputAudioPort = i;
      }
      else if( port.is_a( audio ) && port.is_a( output ) )
      {
        audioBuffers[i] = std::vector< short >(audioBufferSize, 0);
        node1->connect_port( i, &audioBuffers[i][0]);
        idOutputAudioPort = i;
      }
      
      else if( port.is_a( control ) && port.is_a( input ) )
      {
        node1->connect_port( i, &controlInputBuffer[i] );
      }
      else if( port.is_a( control ) && port.is_a( output ) )
      {
        node1->connect_port( i, &controlOutputBuffer[i] );
      }
      else
      {
        // unregonized port
        exit(1);
      }
    }
  }

  void setParam(Lilv::Instance* node, const std::string& portSymbol, int value)
  {
    std::vector<Lilv::Instance*>::iterator it = std::find( nodes.begin(), nodes.end(), node );
    if ( it == nodes.end() )
    {
      exit(1);
    } 
    int indexPlugin = std::distance( nodes.begin(), it );
    Lilv::Port port = plugins.at( indexPlugin ).get_port_by_symbol( pWorld->new_string(&portSymbol[0]) );

    controlInputBuffer[ port.get_index() ] = value;
  }

  void processGraph( const short* bufferIn, short* bufferOut, int nbFrames )
  {
    // int idInputAudioPort = 
    // int idOutputAudioPort = 
    // copy input
    std::copy( bufferIn, bufferIn + nbFrames, &audioBuffers[idInputAudioPort][0] );
    // process nodes
    for ( unsigned int indexInstance = 0; indexInstance < nodes.size(); ++indexInstance )
    {
      nodes.at(indexInstance)->run( nbFrames );
    }
    // copy output
    memcpy( bufferOut, &audioBuffers[idOutputAudioPort][0], nbFrames );
  }

private:
  /**
  * Contains all data of all lv2 plugins on the system.
  */
  Lilv::World* pWorld;

  /**
  * The list of nodes (intances) in the graph.
  */
  std::vector<Lilv::Instance*> nodes;

  /**
  * The list of plugins in the graph.
  */
  std::vector<Lilv::Plugin> plugins;  

  /**
  * Audio buffers
  * + TMP data
  */
  std::vector< std::vector< short > > audioBuffers;
  static const unsigned int audioBufferSize = 1024;
  std::vector< short > controlInputBuffer;
  std::vector< short > controlOutputBuffer;
  
  unsigned int idInputAudioPort;
  unsigned int idOutputAudioPort;

};

/**
* Proto which read test.wav, process a gain of 5db, and write testVFX.wav
*/
int main(int argc, char** argv)
{
  // audio data - input / output / buffers
  SndfileHandle infile( "test.wav" );
  SndfileHandle outfile;
  
  const int format = infile.format();
  const int numChannels = infile.channels();
  const int samplerate = infile.samplerate();
  const float timeToRead = infile.frames() / (float)samplerate; //in seconds

  std::vector< short > audioIn( samplerate * numChannels * timeToRead, 0 );
  std::vector< short > audioOut( samplerate * numChannels * timeToRead, 0 );

  outfile = SndfileHandle( "testVFX.wav" , SFM_WRITE, format , numChannels , samplerate);

  Lv2Graph graph;

  // add nodes to the graph
  Lilv::Instance* gain = graph.addNode( "http://lv2plug.in/plugins/eg-amp", samplerate );
  // connect ports
  graph.connect(gain);
  // update params
  graph.setParam(gain, "gain", 5);

  while( 1 )
  {
    // read on disk
    size_t readedSamples = infile.read( &audioIn[0], samplerate );
    if( ! readedSamples )
      break;

    // process graph
    int nbFrames = 128;
    for (size_t i = 0; i < readedSamples; ++i) 
    {
      graph.processGraph(&audioIn[ i ], &audioOut[ i ], nbFrames);
    }

    // write on disk
    outfile.write( &audioOut[0], readedSamples );
  }

  return 0;
}
