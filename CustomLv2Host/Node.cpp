#include "Node.h"

#include "Lv2Graph.h"

#include <cstdlib> //exit

#include <iostream>
#include "Debugger.h"

namespace sound
{

Node::Node( Lv2Graph* graph, const std::string pluginURIstr, int samplerate )
: _pGraph(graph)
{
  Property pluginURI = _pGraph->getWorld()->new_uri( &( pluginURIstr[0] ) );

  _pInstance = Lilv::Instance::create( getPlugin( pluginURI ), samplerate, NULL );
  if( _pInstance )
  {
    _pInstance->activate( );

    // buffers
    initAudioBuffers( );
    connectControlInput( );
    connectControlOutput( );

    // test
    Debugger::print_plugin( _pGraph->getWorld()->me, getPlugin( pluginURI ).me );
  }
  else
  {
    // exception - impossible to instanciate plugin
    // throw ...
    exit(1);
  }
}

Node::~Node()
{
  _pInstance->deactivate();
  //pInstance->free();
}

Lilv::Plugin Node::getPlugin( Property pluginURI ) const {
  return ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( pluginURI );
}

// private function
void Node::initAudioBuffers( )
{
  Property pluginURI = _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) );
  // control input / output
  _controlBuffers.push_back( std::vector< int >( getPlugin( pluginURI ).get_num_ports( ), 0 ) );
  _controlBuffers.push_back( std::vector< int >( getPlugin( pluginURI ).get_num_ports( ), 0 ) );
}

void Node::connectAudioInput( std::vector< short >& audioInputBuffer)
{
  Property audio = _pGraph->getWorld()->new_uri( LILV_URI_AUDIO_PORT );
  Property input = _pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT );
  Property pluginURI = _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( audio ) && port.is_a( input ) )
    {
      this->_pInstance->connect_port( portIndex, &audioInputBuffer[0] );
      return;
    }
  }
  // exception - unregonized port
  exit(1);
}

void Node::connectAudioOutput( std::vector< short >& audioOutputBuffer)
{
  Property audio = _pGraph->getWorld()->new_uri( LILV_URI_AUDIO_PORT );
  Property output = _pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT );
  Property pluginURI = _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( audio ) && port.is_a( output ) )
    {
      this->_pInstance->connect_port( portIndex, &audioOutputBuffer[0] );
      return;
    }
  }
  // exception - unregonized port
  exit(1);
}


void Node::connectControlInput( )
{
  Property control = _pGraph->getWorld()->new_uri( LILV_URI_CONTROL_PORT );
  Property input = _pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT );
  Property pluginURI = _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( control ) && port.is_a( input ) )
    {
      this->_pInstance->connect_port( portIndex, &_controlBuffers.at( _bufferControlInput ).at( port.get_index() ) );
      //set to default value

      return;
    }
  }
}

void Node::connectControlOutput( )
{
  Property control = _pGraph->getWorld()->new_uri( LILV_URI_CONTROL_PORT );
  Property output = _pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT );
  Property pluginURI = _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) );

  for (unsigned int portIndex = 0; portIndex < getPlugin( pluginURI ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( pluginURI ).get_port_by_index(portIndex);

    if( port.is_a( control ) && port.is_a( output ) )
    {
      this->_pInstance->connect_port( portIndex, &_controlBuffers.at( _bufferControlOutput ).at( port.get_index() ) );
	  //set to default value
	  
      return;
    }
  }
}

void Node::setParam( const std::string& portSymbol, const unsigned int value)
{
  Property input = _pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT );
  Property output = _pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT );
  Property pluginURI = _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) );

  Lilv::Port port = getPlugin( pluginURI ).get_port_by_symbol( _pGraph->getWorld()->new_string(&portSymbol[0]) );

  if ( port.is_a( input ) )
  {
    _controlBuffers.at( _bufferControlInput ).at( port.get_index() ) = value;
  }
  else if ( port.is_a( output ) )
  {
    _controlBuffers.at( _bufferControlOutput ).at( port.get_index() ) = value;
  }
  else
  {
    //exception
    exit(1);
  }
}

void Node::process(size_t sampleCount)
{
  _pInstance->run( sampleCount );
}

}