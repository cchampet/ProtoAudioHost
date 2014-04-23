BIN = customLv2Host

INC_DIR = -I/datas/cnt/workspace/lv2/lilv-0.18.0/lilv-install/include/lilv-0

LIB_DIR = -L/datas/cnt/workspace/lv2/lilv-0.18.0/lilv-install/lib/
LIB     = -lsndfile -llilv-0

.PHONY: all docs clean

customLv2Host: Debugger.o Lv2Graph.o Node.o main.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o $(BIN) main.cpp Debugger.o Lv2Graph.o Node.o $(LIB) $(LIB_DIR)

Debugger.o: Debugger.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o Debugger.o -c Debugger.cpp $(LIB_DIR)
Lv2Graph.o: Lv2Graph.cpp Node.o
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o Lv2Graph.o -c Lv2Graph.cpp $(LIB_DIR)
Node.o: Node.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o Node.o -c Node.cpp $(LIB_DIR)

docs:
	doxygen

clean:
	rm *.o
	rm underwaterVFX.wav
	rm -f customLv2Host

