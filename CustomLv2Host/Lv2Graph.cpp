#include "Lv2Graph.h"

#include <cstdlib> //exit
#include <cstring> //memcpy

#include <iostream>
#include <exception>

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
  _audioBuffers.push_back( std::vector< short >( _audioBufferSize, 0 ) );
  _audioBuffers.push_back( std::vector< short >( _audioBufferSize, 0 ) );
  _audioBuffers.push_back( std::vector< short >( _audioBufferSize, 0 ) );
}

Lv2Graph::~Lv2Graph()
{
  delete _pWorld;
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

void Lv2Graph::processFrame( const short* bufferIn, short* bufferOut )
{
  size_t nbFrames = 2; // exception if nbFrames > Lv2Graph::audioBufferSize

  // if the graph is empty
  if ( _nodes.size() == 0 )
  {
    memcpy( bufferOut, bufferIn, nbFrames );
    return;
  }

  // copy input
  memcpy( &_audioBuffers.at(0)[0], bufferIn, nbFrames );
  // process nodes
  for ( unsigned int indexInstance = 0; indexInstance < _nodes.size(); ++indexInstance )
  {
    getNode( indexInstance ).process( nbFrames );
  }
  // copy output
  memcpy( bufferOut, &_audioBuffers.at(2)[0], nbFrames );
}

}