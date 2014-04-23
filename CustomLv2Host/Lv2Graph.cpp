#include "Lv2Graph.h"

#include <cstdlib> //exit
#include <cstring> //memcpy

#include <iostream>
#include <exception>

namespace sound
{

Lv2Graph::Lv2Graph()
{
	nodes.reserve(200);
  pWorld = new Lilv::World();
  pWorld->load_all();

  initAudioBuffers();
}

// private function
void Lv2Graph::initAudioBuffers()
{
  audioBuffers.push_back( std::vector< short >( audioBufferSize, 0 ) );
  audioBuffers.push_back( std::vector< short >( audioBufferSize, 0 ) );
  audioBuffers.push_back( std::vector< short >( audioBufferSize, 0 ) );
}

Lv2Graph::~Lv2Graph()
{
  delete pWorld;
}

Node& Lv2Graph::addNode( const std::string pluginURI, int samplerate )
{
  Node* newNode = new Node( this, pluginURI, samplerate );
  nodes.push_back( newNode );
  return *nodes.back();
}

void Lv2Graph::connect( Node& node1, Node& node2 )
{
  node1.connectAudioInput( audioBuffers.at(0) );
  node1.connectAudioOutput( audioBuffers.at(1) );
  
  node2.connectAudioInput( audioBuffers.at(1) );
  node2.connectAudioOutput( audioBuffers.at(2) );
}

void Lv2Graph::processFrame( const short* bufferIn, short* bufferOut )
{
  size_t nbFrames = 2; // exception if nbFrames > Lv2Graph::audioBufferSize

  // if the graph is empty
  if ( nodes.size() == 0 )
  {
    memcpy( bufferOut, bufferIn, nbFrames );
    return;
  }

  // copy input
  memcpy( &audioBuffers.at(0)[0], bufferIn, nbFrames );
  // process nodes
  for ( unsigned int indexInstance = 0; indexInstance < nodes.size(); ++indexInstance )
  {
    getNode( indexInstance ).process( nbFrames );
  }
  // copy output
  memcpy( bufferOut, &audioBuffers.at(2)[0], nbFrames );
}

}