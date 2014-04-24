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
  
  void connectAudioInput( std::vector< short >& audioInputBuffer );
  void connectAudioOutput( std::vector< short >& audioOutputBuffer );

  void connectControlInput( );
  void connectControlOutput( );

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
  
private:

  void initAudioBuffers( );

  Lv2Graph*       _pGraph;

  Lilv::Instance* _pInstance;
  
  std::vector< std::vector< float > > _controlBuffers;

  static const size_t _bufferControlInput = 0;
  static const size_t _bufferControlOutput = 1;
};

}


#endif //NODE_H