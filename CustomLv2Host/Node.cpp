#include <exception>
#include <stdexcept> // runtime error
#include <math.h> // isnan
#include <iostream>

#include "Node.h"

#include "Lv2Graph.h"
#include "Debugger.h"


namespace sound
{

Node::Node( Lv2Graph* graph, const std::string pluginURIstr, int samplerate )
: _pGraph(graph)
, _isInputConnected(false)
, _isOutputConnected(false)
{
	Property pluginURI = _pGraph->getWorld()->new_uri( &( pluginURIstr[0] ) );
	Lilv::Plugin plugin = ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( pluginURI );

	if(!plugin)
		throw std::runtime_error("Can't find plugin "+pluginURIstr);
	
	// test
	Debugger::print_plugin( _pGraph->getWorld()->me, plugin.me );

	_pInstance = Lilv::Instance::create( plugin, samplerate, NULL );
	if( !_pInstance )
		throw std::runtime_error("Can't instantiate plugin "+pluginURIstr);

	_pInstance->activate( );

	initControlBuffers( );
	connectControls( );
}

Node::~Node()
{
	_pInstance->deactivate();
	//pInstance->free();
}

void Node::initControlBuffers()
{
	// @todo : not necessary to add a float for audio port
	for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
		_controlBuffers.push_back( 0.f );
}

void Node::connectAudioInput( std::vector< float >& audioInputBuffer)
{
	for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin( ).get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty( ) ) && port.is_a( getInputURIProperty( ) ) )
		{
			_pInstance->connect_port( port.get_index(), &audioInputBuffer[0] );
			_isInputConnected = true;
		}
	}
}

void Node::connectAudioOutput( std::vector< float >& audioOutputBuffer )
{
	for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin( ).get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty( ) ) && port.is_a( getOutputURIProperty( ) ) )
		{
			_pInstance->connect_port( port.get_index(), &audioOutputBuffer[0] );
			_isOutputConnected = true;
		}
	}
}

void Node::connectControls()
{
	size_t numPort = getPlugin( ).get_num_ports();
	for (unsigned int portIndex = 0; portIndex < numPort; ++portIndex)
	{
		// BUG : infinite loop with some plugins...
		//getPlugin( ).get_num_ports( );

		Lilv::Port port = getPlugin( ).get_port_by_index( portIndex );

		if( port.is_a( getControlURIProperty( ) ) )
		{
			// get default value of port
			float minValues[numPort];
			float maxValues[numPort];
			float defaultValues[numPort];
			getPlugin( ).get_port_ranges_float( &minValues[0], &maxValues[0], &defaultValues[0] );

			// add a buffer for this control
			if ( ! isnan( defaultValues[numPort] ))
			{
				_controlBuffers.at( portIndex ) = defaultValues[numPort];
			}
			else 
			{
				LilvNode* maxValue;
				LilvNode* minValue;
				LilvNode* defaultValue;
				lilv_port_get_range( getPlugin( ).me, port.me, 
						&defaultValue, 
						&minValue, 
						&maxValue );
				if ( defaultValue && Lilv::Node( defaultValue ).is_float() )
				{
					float value = Lilv::Node( defaultValue ).as_float();
					_controlBuffers.at( portIndex ) = value;
				}
			}

			// connect the port to the buffer
			_pInstance->connect_port( portIndex, &( _controlBuffers.at( portIndex ) ) );
		}
	}
}

void Node::setParam( const std::string& portSymbol, const float value)
{
	Lilv::Port port = getPlugin( ).get_port_by_symbol( _pGraph->getWorld()->new_string(&portSymbol[0]) );
	if( ! port.me )
	{
		throw std::runtime_error( portSymbol + " : param not found."  );
	}

	if ( port.is_a( getInputURIProperty( ) ) )
	{
		_controlBuffers.at( port.get_index() ) = value;
	}
	else // port.is_a( getOutputURIProperty( ) )
	{
		_controlBuffers.at( port.get_index() ) = value;
	}
}

void Node::process(size_t sampleCount)
{
	_pInstance->run( sampleCount );
}

void Node::printControlBuffers( ) const 
{
	for (unsigned int portIndex = 0; portIndex < _controlBuffers.size(); ++portIndex)
	  std::cout << "port #" << portIndex << " : " << _controlBuffers.at( portIndex ) << std::endl;
}

Lilv::Plugin Node::getPlugin( ) const 
{
	return ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( getPluginURIProperty( ) );
}

size_t Node::getNbAudioInput()
{
	size_t nbAudioInput = 0;
	for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin( ).get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty( ) ) && port.is_a( getInputURIProperty( ) ) )
		{
			++nbAudioInput;
		}
	}
	return nbAudioInput;
}

size_t Node::getNbAudioOutput()
{
	size_t nbAudioOutput = 0;
	for (unsigned int portIndex = 0; portIndex < getPlugin( ).get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin( ).get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty() ) && port.is_a( getOutputURIProperty( ) ) )
		{
			++nbAudioOutput;
		}
	}
	return nbAudioOutput;
}

const Node::Property Node::getPluginURIProperty() const 
{ 
	return _pGraph->getWorld()->new_uri( &( _pInstance->get_descriptor()->URI[0] ) ); 
}

const Node::Property Node::getAudioURIProperty() const 
{ 
	return _pGraph->getWorld()->new_uri( LILV_URI_AUDIO_PORT ); 
}

const Node::Property Node::getInputURIProperty() const 
{ 
	return _pGraph->getWorld()->new_uri( LILV_URI_INPUT_PORT ); 
}

const Node::Property Node::getOutputURIProperty() const 
{ 
	return _pGraph->getWorld()->new_uri( LILV_URI_OUTPUT_PORT ); 
}

const Node::Property Node::getControlURIProperty() const 
{ 
	return _pGraph->getWorld()->new_uri( LILV_URI_CONTROL_PORT ); 
}

const Node::Property Node::getSymbolProperty( const std::string& symbol ) const 
{ 
	return _pGraph->getWorld()->new_string( &symbol[0] ); 
}

}