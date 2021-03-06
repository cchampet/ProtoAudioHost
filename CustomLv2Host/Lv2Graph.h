#ifndef LV2_GRAPH_H
#define LV2_GRAPH_H

// Lilv wrapper C++
#include <lilv/lilvmm.hpp>

#include "Node.h"

namespace sound
{

class Lv2Graph
{
public:
	Lv2Graph();
	~Lv2Graph();

	void createAudioBuffer( const int bufferSize );

	void setUp();

	Node& addNode( const std::string pluginURI, int samplerate );

	void connect( std::vector< float >& bufferIn, Node& startedNode );
	void connect( Node& endedNode, std::vector< float >& bufferOut );
	void connect( Node& node1, Node& node2, size_t numAudioBuffer );

	void processFrame( const float* bufferIn, float* bufferOut );

	Lilv::World* getWorld() const {return _pWorld;}
	Node& getNode( size_t indexNode ) const { return *_nodes.at(indexNode); }
	std::vector< float >& getAudioBufferInput() { return _audioBuffers[0]; }
	std::vector< float >& getAudioBufferOutput() { return _audioBuffers[1]; }
	std::vector< std::vector< float > >& getAudioBuffer() {return _audioBuffers;}

private:

	/**
	* Contains all data of all lv2 plugins on the system.
	*/
	Lilv::World* _pWorld;

	/**
	* The list of nodes (intances) in the graph.
	*/
	std::vector<Node*> _nodes;

	/**
	* Audio buffers, to manage results of the nodes's process.
	*/
	std::vector< std::vector< float > > _audioBuffers;
};

}

#endif //LV2_GRAPH_H