#include <iostream>
#include <exception>

#include "Lv2Graph.h"

namespace sound
{

Lv2Graph::Lv2Graph()
{
  _pWorld = new Lilv::World();
  _pWorld->load_all();
}

Lv2Graph::~Lv2Graph()
{
  delete _pWorld;
  for ( size_t indexNode = 0; indexNode < _nodes.size(); ++indexNode )
  {
	  delete _nodes.at(indexNode);
  }
}

void Lv2Graph::createAudioBuffer( const int bufferSize )
{
  // AudioBuffers Input / Output
  _audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
  _audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
}

void Lv2Graph::setUp( )
{
  for ( unsigned int indexInstance = 0; indexInstance < _nodes.size(); ++indexInstance )
  {
    // As a special case, when sample_count == 0, the plugin should update
    // any output ports that represent a single instant in time (e.g. control
    // ports, but not audio ports). This is particularly useful for latent
    // plugins, which should update their latency output port so hosts can
    // pre-roll plugins to compute latency.
    getNode( indexInstance ).process( 0 );
  }
  //@todo : update buffer when latency
}

Node& Lv2Graph::addNode( const std::string pluginURI, int samplerate )
{
  Node* newNode = new Node( this, pluginURI, samplerate );
  _nodes.push_back( newNode );
  return *_nodes.back();
}

void Lv2Graph::connect( std::vector< float >& bufferIn, Node& startedNode )
{
	startedNode.connectAudioInput( bufferIn );
}

void Lv2Graph::connect( Node& endedNode, std::vector< float >& bufferOut )
{
	endedNode.connectAudioOutput( bufferOut );
}

void Lv2Graph::connect( Node& node1, Node& node2 )
{
  _audioBuffers.push_back( std::vector< float >( 1, 0.f ) );
  
  node1.connectAudioOutput( _audioBuffers.at( _audioBuffers.size() - 1 ) );
  node2.connectAudioInput( _audioBuffers.at( _audioBuffers.size() - 1 ) );
}

void Lv2Graph::processFrame( const float* bufferIn, float* bufferOut )
{
  // if the graph is empty
  if ( _nodes.size() == 0 )
  {
    bufferOut[0] = bufferIn[0];
    return;
  }
  
  getAudioBufferInput()[0] = bufferIn[0];
  
  // process nodes
  for ( size_t indexInstance = 0; indexInstance < _nodes.size(); ++indexInstance )
  {
	if( getNode( indexInstance ).isConnected() )
	  getNode( indexInstance ).process( 1 );
  }
  
  bufferOut[0] = getAudioBufferOutput()[0];
  
  // Print ControlBuffers (can see the value of latency after process nodes)
  // _nodes.at( 1 )->printControlBuffers( );
}

}