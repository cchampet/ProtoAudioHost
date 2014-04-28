BIN     = customLv2SearchPlugins

INC_DIR = -I/datas/cnt/workspace/lv2/lilv-0.18.0/lilv-install/include/lilv-0

LIB_DIR = -L/datas/cnt/workspace/lv2/lilv-0.18.0/lilv-install/lib
LIB     = -lsndfile -llilv-0

.PHONY: all docs clean

customLv2SearchPlugins: main.cpp
	g++ $(INC_DIR) -ansi -Wall -g -O0 -o $(BIN) main.cpp $(LIB) $(LIB_DIR)

docs:
	doxygen

clean:
	rm -f $(BIN)
