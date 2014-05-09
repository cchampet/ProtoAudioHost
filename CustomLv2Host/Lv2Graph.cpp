#include <iostream>
#include <exception>

#include "Lv2Graph.h"

namespace sound
{

Lv2Graph::Lv2Graph()
{
  _pWorld = new Lilv::World();
  _pWorld->load_all();

  initAudioBuffers();
}

// private function
void Lv2Graph::initAudioBuffers()
{
  _audioBuffers.push_back( std::vector< float >( _audioBufferSize, 0 ) );
  _audioBuffers.push_back( std::vector< float >( _audioBufferSize, 0 ) );
  _audioBuffers.push_back( std::vector< float >( _audioBufferSize, 0 ) );
}

Lv2Graph::~Lv2Graph()
{
  delete _pWorld;
  for ( size_t indexNode = 0; indexNode < _nodes.size(); ++indexNode )
  {
	  delete _nodes.at(indexNode);
  }
}

Node& Lv2Graph::addNode( const std::string pluginURI, int samplerate )
{
  Node* newNode = new Node( this, pluginURI, samplerate );
  _nodes.push_back( newNode );
  return *_nodes.back();
}

void Lv2Graph::connect( Node& node1, Node& node2 )
{
  node1.connectAudioInput( _audioBuffers.at(0) );
  node1.connectAudioOutput( _audioBuffers.at(1) );
  
  node2.connectAudioInput( _audioBuffers.at(1) );
  node2.connectAudioOutput( _audioBuffers.at(2) );
}

void Lv2Graph::processFrame( const float* bufferIn, float* bufferOut )
{
  // if the graph is empty
  if ( _nodes.size() == 0 )
  {
    bufferOut[0] = bufferIn[0];
    return;
  }

  // copy input
  _audioBuffers.at(0)[0] = bufferIn[0];
  
  // process nodes
  for ( unsigned int indexInstance = 0; indexInstance < _nodes.size(); ++indexInstance )
  {
    getNode( indexInstance ).process( 1 );
  }
  
  // Print ControlBuffers (can see the value of latency after process nodes)
  // _nodes.at( 1 )->printControlBuffers( );
  
  // copy output
  bufferOut[0] = _audioBuffers.at(2)[0];
}

}