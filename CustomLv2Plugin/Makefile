BUNDLE_PATH = plugins/
BUNDLE      = lv2pftci-reader.lv2
INSTALL_DIR = /homes/cnt/.lv2

INC_DIR = -I/datas/cnt/workspace/lv2/lv2-c++-tools-1.0.5/build/include/lv2-c++-tools -I/datas/cnt/workspace/sox/sox-install/include
LIB_DIR = -L/datas/cnt/workspace/lv2/lv2-c++-tools-1.0.5/build/lib -L/datas/cnt/workspace/sox/sox-install/lib64
LIB     = -lsox -llv2-plugin -lpthread


$(BUNDLE): manifest.ttl reader.so
	rm -rf $(BUNDLE_PATH)$(BUNDLE)
	mkdir $(BUNDLE_PATH)$(BUNDLE)
	cp manifest.ttl reader.ttl reader.so $(BUNDLE_PATH)$(BUNDLE)

reader.so: reader.cpp
	g++ -shared -fPIC -DPIC $(INC_DIR) reader.cpp -o reader.so $(LIB_DIR) $(LIB)

install: $(BUNDLE)
	mkdir -p $(INSTALL_DIR)
	rm -rf $(INSTALL_DIR)/$(BUNDLE)
	cp -R $(BUNDLE) $(INSTALL_DIR)

clean:
	rm -rf $(BUNDLE_PATH)$(BUNDLE) reader.so
