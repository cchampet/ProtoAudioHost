#include <iostream>
#include <exception>
#include <stdlib.h>

#include "Lv2Graph.h"

// @todo : move this out of the function...
char** uris   = NULL;
size_t n_uris = 0;

static char*
copy_string(const char* str)
{
	const size_t len = strlen(str);
	char*        dup = (char*)malloc(len + 1);
	memcpy(dup, str, len + 1);
	return dup;
}

static LV2_URID
urid_map(LV2_URID_Map_Handle handle, const char* uri)
{
	for (size_t i = 0; i < n_uris; ++i) {
		if (!strcmp(uris[i], uri)) {
			return i + 1;
		}
	}

	uris = (char**)realloc(uris, ++n_uris * sizeof(char*));
	uris[n_uris - 1] = copy_string(uri);
	return n_uris;
}
//

namespace sound
{

Lv2Graph::Lv2Graph()
{
	_pWorld = new Lilv::World();
	_pWorld->load_all();
	
	_map = { NULL, urid_map };
	lv2_atom_forge_init( &_forge, &_map );
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
	// @todo : create a buffer in function connect(Node&, Node&)
	_audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
	_audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
	_audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
	_audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
	_audioBuffers.push_back( std::vector< float >( bufferSize, 0.f ) );
}

void Lv2Graph::setUp()
{
	for ( size_t indexInstance = 0; indexInstance < _nodes.size(); ++indexInstance )
	{
		if( ! getNode( indexInstance ).isConnected() )
			continue;
		
		// activate node
		_nodes[indexInstance]->getInstance()->activate();

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

void Lv2Graph::connect( Node& node1, Node& node2, size_t numAudioBuffer )
{
	node1.connectAudioOutput( _audioBuffers.at( numAudioBuffer ) );
	node2.connectAudioInput( _audioBuffers.at( numAudioBuffer ) );
}

void Lv2Graph::processFrame( const float* bufferIn, float* bufferOut )
{
	// if the graph is empty
	if ( _nodes.size() == 0 )
	{
		bufferOut[0] = bufferIn[0];
		return;
	}

	getAudioBuffer()[0][0] = bufferIn[0];

	// process nodes
	for ( size_t indexInstance = 0; indexInstance < _nodes.size(); ++indexInstance )
	{
		if( getNode( indexInstance ).isConnected() )
		{
			getNode( indexInstance ).process( 1 );
		}
	}

	bufferOut[0] = getAudioBuffer()[4][0];
}

}