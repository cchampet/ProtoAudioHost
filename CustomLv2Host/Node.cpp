#include "Node.h"
#include "Lv2Graph.h"

#include <exception>

#include <iostream>
#include "Debugger.h"

namespace sound
{

Node::Node( Lv2Graph* graph, const std::string pluginURIstr, int samplerate )
: _pGraph(graph)
{
  Property pluginURI = _pGraph->getWorld()->new_uri( &( pluginURIstr[0] ) );
  Lilv::Plugin plugin = ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( pluginURI );

  _pInstance = Lilv::Instance::create( plugin, samplerate, NULL );
  if( !_pInstance )
  {
    throw std::bad_alloc( );
  }
  
  _pInstance->activate( );

  // buffers
  initAudioBuffers( );
  connectControlInput( );
  connectControlOutput( );

  // test
  Debugger::print_plugin( _pGraph->getWorld()->me, plugin.me );
}

Node::~Node()
{
  _pInstance->deactivate();
  //pInstance->free();
}

// private function
void Node::initAudioBuffers( )
{
  // control input / output
  _controlBuffers.push_back( std::vector< float >( getPlugin( ).get_num_ports( ), 0 ) );
  _controlBuffers.push_back( std::vector< float >( getPlugin( ).get_num_ports( ), 0 ) );
}

void Node::connectAudioInput( std::vector< short >& audioInputBuffer)
{
  Lilv::Port port = getPlugin( ).get_port_by_symbol( getSymbolProperty( "in" ) );
  
  if( port.is_a( getAudioURIProperty( ) ) && port.is_a( getInputURIProperty( ) ) )
  {
    _pInstance->connect_port( port.get_index(), &audioInputBuffer[0] );
  }
}

void Node::connectAudioOutput( std::vector< short >& audioOutputBuffer)
{
  Lilv::Port port = getPlugin( ).get_port_by_symbol( getSymbolProperty( "out" ) );
  
  if( port.is_a( getAudioURIProperty( ) ) && port.is_a( getOutputURIProperty( ) ) )
  {
    _pInstance->connect_port( port.get_index(), &audioOutputBuffer[0] );
  }
}


void Node::connectControlInput( )
{
  for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( ).get_port_by_index(portIndex);

    if( port.is_a( getControlURIProperty( ) ) && port.is_a( getInputURIProperty( ) ) )
    {
      _pInstance->connect_port( portIndex, &( _controlBuffers.at( _bufferControlInput ).at( port.get_index() ) ) );
      //set to default value

      return;
    }
  }
}

void Node::connectControlOutput( )
{
  for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
  {
    Lilv::Port port = getPlugin( ).get_port_by_index(portIndex);

    if( port.is_a( getControlURIProperty( ) ) && port.is_a( getOutputURIProperty( ) ) )
    {
      _pInstance->connect_port( portIndex, &( _controlBuffers.at( _bufferControlOutput ).at( port.get_index() ) ) );
	  //set to default value
	  
      return;
    }
  }
}

void Node::setParam( const std::string& portSymbol, const float value)
{
  Lilv::Port port = getPlugin( ).get_port_by_symbol( _pGraph->getWorld()->new_string(&portSymbol[0]) );

  if ( port.is_a( getInputURIProperty( ) ) )
  {
    _controlBuffers.at( _bufferControlInput ).at( port.get_index() ) = value;
  }
  else // port.is_a( getOutputURIProperty( ) )
  {
    _controlBuffers.at( _bufferControlOutput ).at( port.get_index() ) = value;
  }
}

void Node::process(size_t sampleCount)
{
  _pInstance->run( sampleCount );
}

Lilv::Plugin Node::getPlugin( ) const {
  return ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( getPluginURIProperty( ) );
}

const Node::Property Node::getPluginURIProperty( ) const 
{ 
  return _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) ); 
}

const Node::Property Node::getAudioURIProperty( ) const 
{ 
  return _pGraph->getWorld()->new_uri( LILV_URI_AUDIO_PORT ); 
}

const Node::Property Node::getInputURIProperty( ) const 
{ 
  return _pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT ); 
}

const Node::Property Node::getOutputURIProperty( ) const 
{ 
  return _pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT ); }

const Node::Property Node::getControlURIProperty( ) const 
{ 
  return _pGraph->getWorld()->new_uri( LILV_URI_CONTROL_PORT ); 
}

const Node::Property Node::getSymbolProperty( const std::string& symbol ) const 
{ 
  return _pGraph->getWorld()->new_string( &symbol[0] ); 
}

}