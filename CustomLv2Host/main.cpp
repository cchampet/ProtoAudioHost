#include <sndfile.hh>
#include <vector>

#include "Node.h"
#include "Lv2Graph.h"

#include <iostream>
#include "Debugger.h"

/**
* Proto which read test.wav, process 2 gain (of 5db and 10db), and write testVFX.wav
*/
int main(int argc, char** argv)
{
  // audio data - input / output / buffers
  SndfileHandle infile( "data/underwater.wav" );
  SndfileHandle outfile;
  
  const int format = infile.format();
  const int numChannels = infile.channels();
  const int samplerate = infile.samplerate();
  const float timeToRead = infile.frames() / (float)samplerate; //in seconds

  std::vector< short > audioIn( samplerate * numChannels * timeToRead, 0 );
  std::vector< short > audioOut( samplerate * numChannels * timeToRead, 0 );

  outfile = SndfileHandle( "data/underwaterVFX.wav" , SFM_WRITE, format , numChannels , samplerate );


  sound::Lv2Graph graph;
  // add nodes to the graph
  sound::Node& gain_1 = graph.addNode( "http://lv2plug.in/plugins/eg-amp", samplerate );
  sound::Node& gain_2 = graph.addNode( "http://lv2plug.in/plugins/eg-amp", samplerate ); //http://plugin.org.uk/swh-plugins/amp
  // connect ports
  graph.connect( gain_1, gain_2 );
  // update params
  gain_1.setParam( "gain", 5 );
  gain_2.setParam( "gain", 10 );


  while( 1 )
  {
    // read on disk
    size_t readedSamples = infile.read( &audioIn[0], samplerate );
    std::cout << "readedSamples : " << readedSamples << std::endl;
    if( ! readedSamples )
      break;

    // process graph
    for (size_t i = 0; i < readedSamples; ++i) 
    {
      graph.processFrame( &audioIn[ i ], &audioOut[ i ] );
    }

    // write on disk
    outfile.write( &audioOut[0], readedSamples );
  }

  return 0;
}
