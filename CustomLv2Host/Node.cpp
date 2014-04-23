#include "Node.h"

#include "Lv2Graph.h"

#include <cstdlib> //exit

#include <iostream>
#include "Debugger.h"

namespace sound
{

Node::Node( Lv2Graph* graph, const std::string pluginURIstr, int samplerate )
: pGraph(graph)
{
  Property pluginURI = pGraph->getWorld()->new_uri( &( pluginURIstr[0] ) );

  pInstance = Lilv::Instance::create( getPlugin( pluginURI ), samplerate, NULL );
  if( pInstance )
  {
    pInstance->activate( );

    // buffers
    initAudioBuffers( );
    connectControlInput( );
    connectControlOutput( );

    // test
    Debugger::print_plugin( pGraph->getWorld()->me, getPlugin( pluginURI ).me );
  }
  else
  {
    // exception - impossible to instanciate plugin
    // throw ...
    exit(1);
  }
}
/*
Node& Node::operator=(const Node& otherNode)
{
  if (this != &otherNode)
  {
    this->pGraph = otherNode.getGraph( );
    this->pInstance = otherNode.getInstance( );
    this->controlBuffers = otherNode.getControlBuffers( );
  }
  return *this;
}
*/
Node::~Node()
{
  pInstance->deactivate();
  //pInstance->free();
}

Lilv::Plugin Node::getPlugin( Property pluginURI ) const {
  return ( ( Lilv::Plugins )pGraph->getWorld()->get_all_plugins() ).get_by_uri( pluginURI );
}

// private function
void Node::initAudioBuffers( )
{
  Property pluginURI = pGraph->getWorld()->new_uri( &( pInstance->get_descriptor()->URI[0] ) );
  // control input / output
  controlBuffers.push_back( std::vector< int >( getPlugin( pluginURI ).get_num_ports( ), 0 ) );
  controlBuffers.push_back( std::vector< int >( getPlugin( pluginURI ).get_num_ports( ), 0 ) );
}

void Node::connectAudioInput( std::vector< short >& audioInputBuffer)
{
  Property audio = pGraph->getWorld()->new_uri( LILV_URI_AUDIO_PORT );
  Property input = pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT );
  Property pluginURI = pGraph->getWorld()->new_uri( &( pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( audio ) && port.is_a( input ) )
    {
      this->pInstance->connect_port( portIndex, &audioInputBuffer[0] );
      return;
    }
  }
  // exception - unregonized port
  exit(1);
}

void Node::connectAudioOutput( std::vector< short >& audioOutputBuffer)
{
  Property audio = pGraph->getWorld()->new_uri( LILV_URI_AUDIO_PORT );
  Property output = pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT );
  Property pluginURI = pGraph->getWorld()->new_uri( &( pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( audio ) && port.is_a( output ) )
    {
      this->pInstance->connect_port( portIndex, &audioOutputBuffer[0] );
      return;
    }
  }
  // exception - unregonized port
  exit(1);
}


void Node::connectControlInput( )
{
  Property control = pGraph->getWorld()->new_uri( LILV_URI_CONTROL_PORT );
  Property input = pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT );
  Property pluginURI = pGraph->getWorld()->new_uri( &( pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( control ) && port.is_a( input ) )
    {
      std::cout << "connectControlInput" << std::endl;
      this->pInstance->connect_port( portIndex, &controlBuffers.at( bufferControlInput ) );
      //set to default value

      return;
    }
  }
}

void Node::connectControlOutput( )
{
  Property control = pGraph->getWorld()->new_uri( LILV_URI_CONTROL_PORT );
  Property output = pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT );
  Property pluginURI = pGraph->getWorld()->new_uri( &( pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( control ) && port.is_a( output ) )
    {
      std::cout << "connectControlOutput" << std::endl;
      this->pInstance->connect_port( portIndex, &controlBuffers.at( bufferControlOutput ) );
      return;
    }
  }
}

void Node::setParam( const std::string& portSymbol, const unsigned int value)
{
  Property input = pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT );
  Property output = pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT );
  Property pluginURI = pGraph->getWorld()->new_uri( &( pInstance->get_descriptor()->URI[0] ) );

  Lilv::Port port = getPlugin( pluginURI ).get_port_by_symbol( pGraph->getWorld()->new_string(&portSymbol[0]) );

  if ( port.is_a( input ) )
  {
    std::cout << "port.get_index() : " << port.get_index() << " / update param to : " << value << std::endl;
    controlBuffers.at( bufferControlInput ).at( port.get_index() ) = value;
  }
  else if ( port.is_a( output ) )
  {
    controlBuffers.at( bufferControlOutput ).at( port.get_index() ) = value;
  }
  else
  {
    //exception
    exit(1);
  }
}

void Node::process(size_t sampleCount)
{
  pInstance->run( sampleCount );
}

}