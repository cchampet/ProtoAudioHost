// SoX : manage IO
#include "sox.h"

// lv2-c++-tools : simplify Lv2 plugin creation
#include <lv2plugin.hpp>

// Lv2 ext
#include "lv2/lv2plug.in/ns/ext/atom/atom.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"

#include <string>
#include <iostream>

#define PLUGIN_READER_URI "http://ll-plugins.nongnu.org/lv2/lv2pftci/reader"

using namespace LV2;

class Reader : public Plugin< Reader > 
{
public:

	Reader(double samplerate)
		: Plugin< Reader >( 3 )
		, _inputStream( NULL )
		, _nChannels( 0 )
		, _samplerate( samplerate )
	{
		// is safe here ?
		sox_init();
		
		// tmp
		_inputFile = "/datas/cnt/workspace/ProtoAudio/CustomLv2Host/data/underwater.wav";
	}

	~Reader()
	{
		sox_quit();
	}

	void activate()
	{
		if( _inputStream )
			return;
		
		// get input file path from atom port 0
		const LV2_Atom* atom = p< LV2_Atom >( 0 );
		std::string sbody = (char*)LV2_ATOM_BODY( atom );
		std::cout << "Plugin - inputPath : " << sbody << std::endl;
		
		// get number of channels form control port 2
		_nChannels = *p(2);
		std::cout << "Plugin - nbChannels : " << _nChannels << std::endl;
		
		// open the input file to set the input stream
		_inputStream = sox_open_read( &_inputFile[0], NULL, NULL, NULL );
	}
	
	void run( uint32_t nframes )
	{
		// support run with 0 frame
		if( nframes == 0 )
			return;
		
		// allocate buffer
		if( _buffer.capacity() < nframes )
			_buffer.resize( nframes );
		
		if( sox_read( _inputStream, &_buffer[0], nframes ) != SOX_EOF )
		{
			//memcpy( &p(1)[0], &_buffer[0], 1 );
			for( uint32_t i = 0; i < nframes; ++i)
			{
				p(1)[i] = _buffer[i];
			}
		}
	}
	
	void deactivate()
	{
		sox_close( _inputStream );
		_inputStream = NULL;
	}
	
private :
	/**
	 * Path of the input file, from port 0 (Atom port).
	 */
	std::string 				_inputFile;
	
	/**
	 * Number of channels of the input file, from port 2 (Control port).
	 */
	float						_nChannels;
	
	/**
	 * Buffer when reading the input file.
	 */
	std::vector<sox_sample_t> 	_buffer;

	/**
	 * Input stream to read.
	 */
	sox_format_t* 				_inputStream;
	
	/**
	 * Samplerate of the audio input.
	 */
	double						_samplerate;
};


static int _ = Reader::register_class( PLUGIN_READER_URI );