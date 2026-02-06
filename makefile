# CC2 := C:\Strawberry\c\bin\g++.exe
# CC1 := C:/msys64/ucrt64/bin/g++.exe
CC := C:\mingw64\bin\g++.exe
INCLUDES := -I./Core -I./include -L./lib -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32 
CORE := .\Core\Materials\Material.cpp .\Core\Grid.cpp .\Core\Simulation.cpp .\Core\Active.cpp .\Core\Materials\MaterialBehaviors.cpp .\Core\Renderer.cpp

Main: Main.cpp
		$(CC) -Wall .\Main.cpp $(CORE) $(INCLUDES) -o Main.exe
		.\Main.exe
		del .\Main.exe

gdb : gdb # quit - q  step - s  run/restart? - r  backtrack - bt
		$(CC) -Wall .\Main.cpp $(CORE) $(INCLUDES) -o Main.exe
		gdb Main.exe


testTriangle: tests\testTriangle.cpp
		$(CC) -Wall .\tests\testTriangle.cpp $(INCLUDES) -o testTriangle.exe
		.\testTriangle.exe
		del .\testTriangle.exes