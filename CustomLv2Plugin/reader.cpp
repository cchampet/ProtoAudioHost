// lv2-c++-tools : simplify Lv2 plugin creation
#include <lv2plugin.hpp>

// manage IO
#include "sox.h"

#include <string>


using namespace LV2;


class Reader : public Plugin<Reader> 
{
public:

	Reader(double rate)
		: Plugin<Reader>(1)
		, _inputFile( "" )
		, _outputStream( NULL )
	{
		// is safe here ?
		sox_init();
	}
	
	Reader(double rate, const std::string& inputFile)
		: Plugin<Reader>(1)
		, _inputFile( inputFile )
		, _outputStream( NULL )
	{
		// is safe here ?
		sox_init();
	}

	~Reader()
	{
		sox_quit();
	}

	void setInputFile(const std::string& inputFile)
	{
		_inputFile = inputFile;
	}

	void activate()
	{
		if( _outputStream )
			return;
		
		_outputStream = sox_open_read(&_inputFile[0], NULL, NULL, NULL);
	}
	
	void run(uint32_t nframes)
	{
		sox_read(_outputStream, &_buffer[0], nframes);

		// memcpy !!
		for (uint32_t i = 0; i < nframes; ++i)
			p(0)[i] = _buffer[i];
	}
	
	void deactivate()
	{
		sox_close(_outputStream);
		_outputStream = NULL;
	}
	
private :
	std::string 				_inputFile;
	std::vector<sox_sample_t> 	_buffer;

	sox_format_t* 				_outputStream;
};


static int _ = Reader::register_class("http://ll-plugins.nongnu.org/lv2/lv2pftci/reader");