run: main
	LD_LIBRARY_PATH="." ./main
	
main: src/main.cpp src/hello.cpp src/fileexplorer.cpp src/plotter.cpp src/texteditor.cpp imgui.so implot.so
	clang++ -I./imgui -I./implot -I. -ggdb -std=c++20 -lglfw -lGL -lGLEW imgui.so implot.so src/main.cpp -o main

imgui.so: imgui/*.cpp imgui/*.h
	clang++ -shared -Iimgui -ggdb -std=c++20 \
	 imgui/imgui.cpp \
	 imgui/imgui_draw.cpp \
	 imgui/imgui_widgets.cpp \
	 imgui/imgui_tables.cpp \
	 imgui/imgui_demo.cpp \
	 imgui/backends/imgui_impl_glfw.cpp \
	 imgui/backends/imgui_impl_opengl3.cpp \
	 -o imgui.so

implot.so: implot/*.cpp implot/*.h
	clang++ -shared -Iimplot -Iimgui -ggdb -std=c++20 \
	 implot/implot.cpp \
	 implot/implot_items.cpp \
	 -o implot.so
	
