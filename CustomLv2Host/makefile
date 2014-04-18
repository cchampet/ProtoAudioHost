BIN = CustomLv2Host

INC_DIR = -I/datas/cnt/workspace/lv2/lilv-0.18.0/lilv-install/include/lilv-0

LIB_DIR = 
LIB     = -lsndfile
LIB_DYN = /datas/cnt/workspace/lv2/lilv-0.18.0/lilv-install/lib/liblilv-0.so

.PHONY: all docs clean

CustomLv2Host: debugger.o main.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o $(BIN) main.cpp debugger.o $(LIB) $(LIB_DIR) $(LIB_DYN)

debugger.o: debugger.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o debugger.o -c debugger.cpp $(LIB_DIR) $(LIB_DYN)

docs:
	doxygen

clean:
	rm *.o
	rm testVFX.wav
	rm -f CustomLv2Host

