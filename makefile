CC2 := C:\Strawberry\c\bin\g++.exe
CC1 := C:/msys64/ucrt64/bin/g++.exe
CC := C:\mingw64\bin\g++.exe
INCLUDES := -I./Core -I./include -L./lib -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32 

sandfallv1.2.2: sandfallv1.2.2.cpp
		$(CC) -Wall .\sandfallv1.2.2.cpp $(INCLUDES) -o sandfallv1.2.2.exe
		.\sandfallv1.2.2.exe
		del .\sandfallv1.2.2.exe

testTriangle: tests\testTriangle.cpp
		$(CC) -Wall .\tests\testTriangle.cpp $(INCLUDES) -o testTriangle.exe
		.\testTriangle.exe
		del .\testTriangle.exe