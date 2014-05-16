#include <exception>
#include <stdexcept> // runtime error
#include <math.h> // isnan
#include <iostream>

#include "Node.h"

#include "Lv2Graph.h"
#include "Debugger.h"

// atom
#include "lv2/lv2plug.in/ns/ext/atom/forge.h"

namespace sound
{

Node::Node( Lv2Graph* graph, const std::string pluginURIstr, int samplerate )
: _pGraph(graph)
, _pInstance(NULL)
, _isConnected(false)
{
	Property pluginURI = _pGraph->getWorld()->new_uri( &( pluginURIstr[0] ) );
	Lilv::Plugin plugin = ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( pluginURI );

	if(!plugin)
		throw std::runtime_error("Can't find plugin "+pluginURIstr);
	
	// test
	//Debugger::print_plugin( _pGraph->getWorld()->me, plugin.me );

	_pInstance = Lilv::Instance::create( plugin, samplerate, NULL );
	if( !_pInstance )
		throw std::runtime_error("Can't instantiate plugin "+pluginURIstr);
	
	createControlBuffers();
	createAtomBuffers();
	
	connectControls();
	connectAtoms();
}

Node::~Node()
{
	_pInstance->deactivate();
	//pInstance->free();
}

void Node::createControlBuffers()
{
	for(size_t portIndex = 0; portIndex < getPlugin().get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getControlURIProperty() ) )
		{
			_controlBufferMap.insert( std::pair< size_t, float>( portIndex, 0.f ) );
		}
	}
}

void Node::createAtomBuffers()
{
	for(size_t portIndex = 0; portIndex < getPlugin().get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getAtomPortURIProperty() ) )
		{
			_atomBufferMap[portIndex] = Atom( NULL, std::vector< char>() );
		}
	}
}

void Node::connectAudioInput( std::vector< float >& audioInputBuffer)
{
	for (unsigned int portIndex = 0; portIndex < getPlugin().get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty() ) && port.is_a( getInputURIProperty() ) )
		{
			_pInstance->connect_port( port.get_index(), &audioInputBuffer[0] );
			_isConnected = true;
		}
	}
}

void Node::connectAudioOutput( std::vector< float >& audioOutputBuffer )
{
	for (unsigned int portIndex = 0; portIndex < getPlugin().get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty() ) && port.is_a( getOutputURIProperty() ) )
		{
			_pInstance->connect_port( port.get_index(), &audioOutputBuffer[0] );
			_isConnected = true;
		}
	}
}

void Node::connectControls()
{
	size_t numPort = getPlugin().get_num_ports();
	for (unsigned int portIndex = 0; portIndex < numPort; ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getControlURIProperty() ) )
		{
			// get default value of port
			float minValues[numPort];
			float maxValues[numPort];
			float defaultValues[numPort];
			getPlugin().get_port_ranges_float( &minValues[0], &maxValues[0], &defaultValues[0] );

			if ( ! isnan( defaultValues[portIndex] ))
			{
				// put default value in buffer for this control port
				_controlBufferMap.at( portIndex ) = defaultValues[portIndex];
				// connect the control port to the buffer
				_pInstance->connect_port( portIndex, &( _controlBufferMap.at( portIndex ) ) );
			}
		}
	}
}

void Node::connectAtoms()
{
	size_t numPort = getPlugin().get_num_ports();
	for (unsigned int portIndex = 0; portIndex < numPort; ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );
		
		if( port.is_a( getAtomPortURIProperty() ) )
		{
			// connect the atom port to the buffer
			_pInstance->connect_port( portIndex, &( _atomBufferMap.at( portIndex ).first ) );
		}
	}
}

void Node::setParam( const std::string& portSymbol, const float value)
{
	Lilv::Port port = getPlugin().get_port_by_symbol( _pGraph->getWorld()->new_string(&portSymbol[0]) );
	if( ! port.me )
	{
		throw std::runtime_error( portSymbol + " : param not found."  );
	}

	if( port.is_a( getControlURIProperty() ) )
	{
		_controlBufferMap.at( port.get_index() ) = value;
	}
}

void Node::setParam( const std::string& portSymbol, const std::string& value )
{
	Lilv::Port port = getPlugin().get_port_by_symbol( _pGraph->getWorld()->new_string(&portSymbol[0]) );
	if( ! port.me )
	{
		throw std::runtime_error( portSymbol + " : param not found."  );
	}
	
	if( port.is_a( getAtomPortURIProperty() ) )
	{	
		size_t bufferSize = 1024; //seg fault with value.length() + 1
		std::vector< char >& buffer = _atomBufferMap[ port.get_index() ].second;
		buffer.resize( bufferSize, 0 );
		lv2_atom_forge_set_buffer( &_pGraph->_forge, (uint8_t*)&buffer[0], bufferSize );
		
		LV2_Atom* atom = lv2_atom_forge_deref( &_pGraph->_forge, lv2_atom_forge_string( &_pGraph->_forge, &value[0], value.length() ) );
		_atomBufferMap[ port.get_index() ] = Atom( atom, buffer );
		
		connectAtoms();
	}
}

void Node::process(size_t sampleCount)
{
	_pInstance->run( sampleCount );
}

Lilv::Plugin Node::getPlugin() const 
{
	return ( ( Lilv::Plugins )_pGraph->getWorld()->get_all_plugins() ).get_by_uri( getPluginURIProperty() );
}

void Node::printControlBufferMap() 
{
	for( std::map< size_t, float >::iterator it = _controlBufferMap.begin();
		it != _controlBufferMap.end();
		++it )
	{
		std::cout << "port control #" << (*it).first << " : " << (*it).second << std::endl;
	}
}

void Node::printAtomBufferMap() 
{
	for( std::map< size_t, Atom >::iterator it = _atomBufferMap.begin();
		it != _atomBufferMap.end();
		++it )
	{
		if( (*it).second.first == NULL )
		{
			std::cout << "port atom #" << (*it).first << " : NULL" << std::endl;
			continue;
		}
		std::string strAtom = (char*)LV2_ATOM_BODY( (*it).second.first );
		std::cout << "port atom #" << (*it).first << " : " << strAtom << std::endl;
	}
}

size_t Node::getNbAudioInput()
{
	size_t nbAudioInput = 0;
	for (unsigned int portIndex = 0; portIndex < getPlugin().get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty() ) && port.is_a( getInputURIProperty() ) )
		{
			++nbAudioInput;
		}
	}
	return nbAudioInput;
}

size_t Node::getNbAudioOutput()
{
	size_t nbAudioOutput = 0;
	for (unsigned int portIndex = 0; portIndex < getPlugin().get_num_ports(); ++portIndex)
	{
		Lilv::Port port = getPlugin().get_port_by_index( portIndex );

		if( port.is_a( getAudioURIProperty() ) && port.is_a( getOutputURIProperty() ) )
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

const Node::Property Node::getAtomPortURIProperty() const 
{ 
	return _pGraph->getWorld()->new_uri( LV2_ATOM__AtomPort ); 
}

const Node::Property Node::getSymbolProperty( const std::string& symbol ) const 
{ 
	return _pGraph->getWorld()->new_string( &symbol[0] ); 
}

}