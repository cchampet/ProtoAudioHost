#ifndef NODE_H
#define NODE_H

#include <string>
#include <vector>

// Lilv wrapper C++
#include <lilv/lilvmm.hpp>

namespace sound
{

class Lv2Graph;

class Node
{
  typedef Lilv::Node Property;

public:

  Node( Lv2Graph* graph, const std::string pluginURI, int sampleRate );
  ~Node( );
  
  void connectAudioInput( std::vector< float >& audioInputBuffer );
  void connectAudioOutput( std::vector< float >& audioOutputBuffer );

  void connectControls( );

  void setParam( const std::string& portSymbol, const float value );

  void process( size_t sampleCount );
  
  
  Lv2Graph* getGraph() const { return _pGraph; }
  Lilv::Instance* getInstance( ) const { return _pInstance; }
  
  Lilv::Plugin getPlugin( ) const;
  const Property getPluginURIProperty( ) const;
  const Property getAudioURIProperty( ) const;
  const Property getInputURIProperty( ) const;
  const Property getOutputURIProperty( ) const;
  const Property getControlURIProperty( ) const;
  const Property getSymbolProperty( const std::string& symbol ) const;
  
  void printControlBuffers( ) const;
  
private:

	void initControlBuffers( );

	
  Lv2Graph*       _pGraph;

  Lilv::Instance* _pInstance;
  
  std::vector< float > _controlBuffers;
};

}


#endif //NODE_H