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

#define AUDIO_BUFFER_AUDIO_INPUT    0
#define AUDIO_BUFFER_AUDIO_OUTPUT   1
#define AUDIO_BUFFER_CONTROL_INPUT  2
#define AUDIO_BUFFER_CONTROL_OUTPUT 3
// #define AUDIO_BUFFER_EVENT_INPUT    4
// #define AUDIO_BUFFER_EVENT_OUTPUT   5
// MIDI ?

class Lv2Graph
{
public:
  Lv2Graph()
  {
    pWorld = new Lilv::World();
    pWorld->load_all();

    initAudioBuffers();
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

    // create some nodes that represent certain features: Input / Output, Audio, Control (other exist : MIDI, Event...)
    Lilv::Node audio    = pWorld->new_uri( LILV_URI_AUDIO_PORT );
    Lilv::Node control  = pWorld->new_uri( LILV_URI_CONTROL_PORT );
    Lilv::Node input    = pWorld->new_uri( LILV_URI_INPUT_PORT );
    Lilv::Node output   = pWorld->new_uri( LILV_URI_OUTPUT_PORT );

    unsigned int numPorts = plugins.at(indexPlugin).get_num_ports();
    for (unsigned int i = 0; i < numPorts; i++)
    {
      Lilv::Port port = plugins.at(indexPlugin).get_port_by_index(i);

      if( port.is_a( audio ) && port.is_a( input ) )
      {
        node1->connect_port( i, &audioBuffersIn[ AUDIO_BUFFER_AUDIO_INPUT ][0] );
      }
      else if( port.is_a( audio ) && port.is_a( output ) )
      {
        node1->connect_port( i, &audioBuffersIn[ AUDIO_BUFFER_AUDIO_OUTPUT ][0] );
      }
      
      else if( port.is_a( control ) && port.is_a( input ) )
      {
        node1->connect_port( i, &audioBuffersIn[ AUDIO_BUFFER_CONTROL_INPUT ][0] );
      }
      else if( port.is_a( control ) && port.is_a( output ) )
      {
        node1->connect_port( i, &audioBuffersIn[ AUDIO_BUFFER_CONTROL_OUTPUT ][0] );
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

    audioBuffersIn[ AUDIO_BUFFER_CONTROL_INPUT ][ port.get_index() ] = value;
  }

  void processGraph( const short* bufferIn, short* bufferOut )
  {
    // if the graph is empty
    if ( nodes.size() == 0 )
    {
      memcpy( bufferOut, bufferIn, Lv2Graph::audioBufferSize );
      return;
    }

    // copy input
    std::copy( bufferIn, bufferIn + Lv2Graph::audioBufferSize, &audioBuffersIn[ AUDIO_BUFFER_AUDIO_INPUT ][0] );
    // process nodes
    for ( unsigned int indexInstance = 0; indexInstance < nodes.size(); ++indexInstance )
    {
      nodes.at(indexInstance)->run( Lv2Graph::audioBufferSize );
    }
    // copy output
    memcpy( bufferOut, &audioBuffersIn[ AUDIO_BUFFER_AUDIO_OUTPUT ][0], Lv2Graph::audioBufferSize );
  }

  /**
  * Size of audio buffers.
  */
  static const unsigned int audioBufferSize = 128;

private:
  void initAudioBuffers()
  {
    audioBuffersIn = std::vector< std::vector< short > >( 4 );
    audioBuffersOut = std::vector< std::vector< short > >( 4 );

    audioBuffersIn[ AUDIO_BUFFER_AUDIO_INPUT ] = std::vector< short >( audioBufferSize, 0 );
    audioBuffersIn[ AUDIO_BUFFER_AUDIO_OUTPUT ] = std::vector< short >( audioBufferSize, 0 );
    audioBuffersIn[ AUDIO_BUFFER_CONTROL_INPUT ] = std::vector< short >( audioBufferSize, 0 );
    audioBuffersIn[ AUDIO_BUFFER_CONTROL_OUTPUT ] = std::vector< short >( audioBufferSize, 0 );
  }

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
  */
  std::vector< std::vector< short > > audioBuffersIn;
  std::vector< std::vector< short > > audioBuffersOut;
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
    for (size_t i = 0; i < readedSamples; ++i) 
    {
      graph.processGraph( &audioIn[ i ], &audioOut[ i ] );
    }

    // write on disk
    outfile.write( &audioOut[0], readedSamples );
  }

  return 0;
}
