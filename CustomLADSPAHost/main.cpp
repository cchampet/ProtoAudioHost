#include <ladspamm-0/world.h>
#include <ladspamm-0/ladspamm.h>
#include <ladspamm-0/plugin_instance.h>

#include <iostream>
#include <cstdlib>
#include <vector>

#include <sndfile.hh>
#include <string>
#include <math.h> 

// Tools to print LADSPA data
#include "debugger.h"

int main(int argc, char** argv) 
{
	try
	{
		// load data of all lv2 plugins on the system
		ladspamm::world world;
		//ladspa::tools::Debugger::analyseworld(world);
		
		// get plugin for the amplifier (gain)
		ladspamm::library_ptr pLibrary;
		for (unsigned int lib_index = 0; lib_index < world.libraries.size(); ++lib_index)
		{
			if (world.libraries[lib_index]->the_dl->filename == "/usr/lib/ladspa/amp.so")
			{
				pLibrary = world.libraries[lib_index];
				break;
			}
		}
		std::cout << "Library: " << pLibrary->the_dl->filename << std::endl;

		ladspamm::plugin_ptr pPlugin;
		for(unsigned int plugin_index = 0; plugin_index < pLibrary->plugins.size(); ++plugin_index)
		{
			if (pLibrary->plugins[plugin_index]->label() == "amp_mono")
			{
				pPlugin = pLibrary->plugins[plugin_index];
				ladspa::tools::Debugger::analyseplugin(pPlugin);
				break;
			}
		}

		// audio data
		unsigned long samplerate = 8000;
		int* audioIn = new int[ samplerate * 10 ];
		int* audioOut = new int[ samplerate * 10 ];
		float** audioBuffers;
		float* controlInputBuffer;
		float* controlOutputBuffer;
		unsigned int audioBufferSize = 1024;
		unsigned int idInputAudioPort = 0;
		unsigned int idOutputAudioPort = 0;
		unsigned int idInputControlPort = 0;
		unsigned int idOutputControlPort = 0;

		// create a node
		ladspamm::plugin_instance instance(pPlugin, samplerate);
		instance.activate();

		// connect ports
		unsigned long numPorts = pPlugin->port_count();
		audioBuffers = new float*[numPorts];
		controlInputBuffer = new float[numPorts];
		controlOutputBuffer = new float[numPorts];
		for ( unsigned int i = 0; i < numPorts; i++ )
		{
			controlInputBuffer[i] = 0;
			controlOutputBuffer[i] = 0;
		}
		for (unsigned int port_index = 0; port_index < numPorts; port_index++)
		{
			if( pPlugin->port_is_audio(port_index) && pPlugin->port_is_input(port_index) )
			{
				audioBuffers[port_index] = new float[audioBufferSize];
				instance.connect_port( port_index, audioBuffers[port_index]);
				idInputAudioPort = port_index;
			}
			else if( pPlugin->port_is_audio(port_index) && pPlugin->port_is_output(port_index) )
			{
				audioBuffers[port_index] = new float[audioBufferSize];
				instance.connect_port( port_index, audioBuffers[port_index]);
				idOutputAudioPort = port_index;
			}

			else if( pPlugin->port_is_control(port_index) && pPlugin->port_is_input(port_index) )
			{
				instance.connect_port( port_index, &controlInputBuffer[port_index] );
				idInputControlPort = port_index;
				// set param of the gain
				unsigned int gain = 5;
				controlInputBuffer[port_index] = gain;
			}
			else if( pPlugin->port_is_control(port_index) && pPlugin->port_is_output(port_index) )
			{
				instance.connect_port( port_index, &controlOutputBuffer[port_index] );
				idOutputControlPort = port_index;
			}
			else
			{
				// unregonized port
				std::cout << "Unregonized port" << std::endl;
				return 1;
			}
		}

		// define input and output file
		const int format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
		unsigned int numChannels = 1;
		SndfileHandle outfile( "testVFX.wav", SFM_WRITE, format, numChannels, samplerate);
		SndfileHandle infile( "test.wav" );
		infile.read( audioIn, samplerate * 10 ); // read 10s of the input file

		// process node
		unsigned long nframes = 128;
		for (int i = 0; i < ceil( samplerate * 10 / 128 ); i++) {
			// copy input
			std::copy(&audioIn[ i * 128 ], &audioIn[ i * 128 ] + 128, audioBuffers[idInputAudioPort]);
			instance.run(nframes);
			// copy output
			for (int x = 0; x < 128; x++) {
				audioOut[ i * 128 + x ] = audioBuffers[idOutputAudioPort][x];
			}
		}

		// save audio to disk
		outfile.write( audioOut, samplerate * 10 );

		// cleanu
		delete[] controlInputBuffer;
		delete[] controlOutputBuffer;
		delete[] audioIn;
		delete[] audioOut;
		// for(int i = 0 ; i < numPorts; ++i)
		// {
		//   delete[] audioBuffers[i];
		// }
		// delete[] audioBuffers;
	}
	
	catch(const std::runtime_error &e)
	{
		std::cerr << e.what() << std::endl;
		return (EXIT_FAILURE);
	}
	
	return (EXIT_SUCCESS);
}


