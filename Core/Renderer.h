#pragma once

#include <vector>
#include "Grid.h"

using GLuint = unsigned int;

class Shader;
struct GLFWwindow;

class Renderer {
public:
    Renderer(int width, int height);

    void prepWindow(Grid &grid);


    void handleMouse(Grid &grid);
    void processInput();

    GLuint createGridTexture(Grid &grid);
    void updateGridTexture(Grid &grid);
    void renderTexture();
    bool shouldWindowClose();
    void changeVSync();
    void fps();
    void Render(Grid &grid);
    inline bool Pawsed() {return pawsed && !nextFrame; };
    void Terminate();


    const int WIDTH = 1600;
    const int HEIGHT = 800;
    const int SCALE = 4;
    const int GRID_WIDTH = WIDTH / SCALE;
    const int GRID_HEIGHT = HEIGHT / SCALE;
    const int PIXEL_SIZE = WIDTH / GRID_WIDTH;

    double lastTime;
    int nbFrames = 0;

    int texWidth, texHeight;

    unsigned int texture;
    unsigned int VBO, VAO, EBO;

    std::vector<unsigned char> pixels;

    Shader* shader;
    GLFWwindow* window;


    // keyboard stuff
    MaterialID DrawMaterial;
    bool CanSwapDraw; 
    bool wasDrawingLastFrame;
    bool nextFrame;
    bool canNextFrame;
    bool pawsed;     // don't change
    bool canPaws;
    bool vSync;
    bool canChangevSync;
};