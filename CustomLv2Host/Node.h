#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>
#include <map>

// Lilv wrapper C++
#include <lilv/lilvmm.hpp>

// atom
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"

namespace sound
{

class Lv2Graph;

class Node
{
public:
	
	typedef Lilv::Node Property;
	typedef std::pair< LV2_Atom*, std::vector< char > > Atom;

public:

	Node( Lv2Graph* graph, const std::string pluginURI, int sampleRate );
	~Node();

	void connectAudioInput( std::vector< float >& audioInputBuffer );
	void connectAudioOutput( std::vector< float >& audioOutputBuffer );

	void connectControls();
	void connectAtoms();

	void setParam( const std::string& portSymbol, const float value );
	void setParam( const std::string& portSymbol, const std::string& value );

	void process( size_t sampleCount );

	bool isConnected() const {return _isConnected;}

	Lv2Graph* getGraph() const { return _pGraph; }
	Lilv::Instance* getInstance() const { return _pInstance; }

	Lilv::Plugin getPlugin() const;
	size_t getNbAudioInput();
	size_t getNbAudioOutput();
	const Property getPluginURIProperty() const;
	const Property getAudioURIProperty() const;
	const Property getInputURIProperty() const;
	const Property getOutputURIProperty() const;
	const Property getControlURIProperty() const;
	const Property getSymbolProperty( const std::string& symbol ) const;
	const Property getAtomPortURIProperty() const;
	
	void printControlBufferMap();
	void printAtomBufferMap();

private:

	void createAudioBuffers();
	void createControlBuffers();
	void createAtomBuffers();

	Lv2Graph*       _pGraph;

	Lilv::Instance* _pInstance;

	std::map< size_t, float >	_controlBufferMap;
	std::map< size_t, Atom >	_atomBufferMap;
	
	//std::vector<Atom>  _atomBuffers;

	
	bool _isConnected;
};

}


#endif //NODE_H