#include <sndfile.hh>
#include <cstring> //memcpy
#include <string>
#include <math.h> 
#include <vector>

// Lilv wrapper C++
#include <lilv/lilvmm.hpp>
// Tools to print Lv2 data
#include "debugger.h"

#include <iostream>

/**
* Proto which read test.wav, process a gain of 5db, and write testVFX.wav
*/
int main(int argc, char** argv)
{
  // audio data - input / output / buffers
  SndfileHandle infile( "test.wav" );
  
  const int format = infile.format();
  const int numChannels = infile.channels();
  const int samplerate = infile.samplerate();
  const int timeToRead = 10; //in seconds

  std::vector< short > audioIn( samplerate * numChannels * timeToRead, 0 );
  std::vector< short > audioOut( samplerate * numChannels * timeToRead, 0 );
  
  SndfileHandle outfile( "testVFX.wav" , SFM_WRITE, format , numChannels , samplerate);

  std::vector< std::vector< short > > audioBuffers;
  std::vector< short > controlInputBuffer;
  std::vector< short > controlOutputBuffer;
  unsigned int audioBufferSize = 1024;
  unsigned int idInputAudioPort = 0;
  unsigned int idOutputAudioPort = 0;

  // load data of all lv2 plugins on the system
  Lilv::World* world = new Lilv::World();
  world->load_all();
  Lilv::Plugins plugins = ( Lilv::Plugins )world->get_all_plugins();

  // get plugin for the amplifier (gain)
  std::string pluginURI = "http://lv2plug.in/plugins/eg-amp";
  Lilv::Node plugin_uri = world->new_uri( &pluginURI[0] );
  Lilv::Plugin plugin = plugins.get_by_uri( plugin_uri );
  lv2::tools::Debugger::print_plugin( world->me, plugin.me );

  // create a node
  Lilv::Instance* instance = Lilv::Instance::create( plugin, samplerate, NULL );
  instance->activate();
  
  // connect ports
  unsigned int numPorts = plugin.get_num_ports();
  audioBuffers = std::vector< std::vector< short > >( numPorts );
  controlInputBuffer = std::vector< short >( numPorts, 0 );
  controlOutputBuffer = std::vector< short >( numPorts, 0 );

  // create some nodes that represent certain features: Input / Output, Audio, Control (other exist : MIDI, Event...)
  Lilv::Node audio    = world->new_uri( LILV_URI_AUDIO_PORT );
  Lilv::Node control  = world->new_uri( LILV_URI_CONTROL_PORT );
  Lilv::Node input    = world->new_uri( LILV_URI_INPUT_PORT );
  Lilv::Node output   = world->new_uri( LILV_URI_OUTPUT_PORT );

  for (unsigned int i = 0; i < numPorts; i++)
  {
    Lilv::Port port = plugin.get_port_by_index(i);

    if( port.is_a( audio ) && port.is_a( input ) )
    {
      audioBuffers[i] = std::vector< short >(audioBufferSize, 0);
      instance->connect_port( i, &audioBuffers[i][0]);
      idInputAudioPort = i;
    }
    else if( port.is_a( audio ) && port.is_a( output ) )
    {
      audioBuffers[i] = std::vector< short >(audioBufferSize, 0);
      instance->connect_port( i, &audioBuffers[i][0]);
      idOutputAudioPort = i;
    }
    
    else if( port.is_a( control ) && port.is_a( input ) )
    {
      instance->connect_port( i, &controlInputBuffer[i] );
      // set param of the gain
      unsigned int gain = 5;
      controlInputBuffer[i] = gain;
    }
    else if( port.is_a( control ) && port.is_a( output ) )
    {
      instance->connect_port( i, &controlOutputBuffer[i] );
    }
    else
    {
      // unregonized port
      return 1;
    }
  }

  // read audio on disk
  infile.read( &audioIn[0], samplerate * timeToRead );

  // process node
  int nframes = 128;
  for (int i = 0; i < ceil( samplerate * timeToRead / nframes ); i++) {
    // copy input
    std::copy(&audioIn[ i * nframes ], &audioIn[ i * nframes ] + nframes, &audioBuffers[idInputAudioPort][0]);
    instance->run(nframes);
    // copy output
    for (int x = 0; x < nframes; x++) {
      audioOut[ i * nframes + x ] = audioBuffers[idOutputAudioPort][x];
    }
    //memcpy( &audioOut[ i * nframes], audioBuffers[idOutputAudioPort], nframes );
  }

  // save audio to disk
  outfile.write( &audioOut[0], samplerate * timeToRead );

  return 0;
}


// class LadspaGraph
// {
// public:
//   LadspaGraph()
//   {}

//   Lilv::Instance* addNode( const std::string pluginURI )
//   {
//     Lilv::Node plugin_uri = world->new_uri( &pluginURI[0] );
//     Lilv::Plugin plugin = plugins.get_by_uri( plugin_uri );

//     Lilv::Instance* instance = Lilv::Instance::create( plugin, samplerate, NULL );
//     if( ! instance )
//       return NULL;
    
//     nodes.push_back( instance );
//     return instance;
//   }

//   void processFrame( const std::vector<char>& bufferIn, std::vector<char>& bufferOut )
//   {

//   }

// private:
//   std::vector<Lilv::Instance*> nodes;

// };


// int main(int argc, char** argv)
// {
//   SndfileHandle outfile( "testVFX.wav" , SFM_WRITE, format , numChannels , samplerate);
//   SndfileHandle infile( "test.wav" );

//   std::vector<char> bufferIn( buffer_size, 0 );
//   std::vector<char> bufferOut( buffer_size, 0 ];

//   LadspaGraph graph;
//   Lilv::Instance* gain = graph.addNode( "http://lv2plug.in/plugins/eg-amp" );
//   graph.addNode(  );
//   graph.addNode(  );

//   graph.connect( reader, gain );
//   graph.connect( gain, writer );

//   while( 1 )
//   {
//     size_t readedSamples = infile.read( bufferIn, bufferIn.size() ) // -> to bufferIn
//     if( ! readedSamples )
//       break;

//     graph.processFrame( bufferIn, bufferOut );

//     outfile.write( bufferIn, samplerate * 10 );
//   }

//   outfile.close();
// }
