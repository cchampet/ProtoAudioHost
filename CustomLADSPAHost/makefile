BIN = CustomLADSPAHost

INC_DIR = -I/datas/libs/boost_1_55_0_py26/include -I/datas/cnt/workspace/ladspa/ladspamm/ladspamm-install/include

LIB_DIR = 
LIB     = -ldl -lsndfile
LIB_DYN = /datas/libs/boost_1_55_0_py26/lib/libboost_filesystem.so /datas/libs/boost_1_55_0_py26/lib/libboost_system.so

.PHONY: all clean

CustomLADSPAHost: debugger.o main.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o $(BIN) main.cpp debugger.o $(LIB) $(LIB_DYN)

debugger.o: debugger.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o debugger.o -c debugger.cpp $(LIB)

docs:
	doxygen

clean:
	rm *.o
	rm testVFX.wav
	rm -f CustomLADSPAHost

