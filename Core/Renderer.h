#pragma once

#include <vector>
#include "Grid.h"

using GLuint = unsigned int;

class Shader;
struct GLFWwindow;

struct Camera {
    int x;      // top-left grid cell
    int y;
    int width;  // how many grid cells visible
    int height;
};


class Renderer {
public:
    Renderer(int width, int height);

    void prepWindow(Grid &grid);


    void handleMouse(Grid &grid);
    void processInput();

    GLuint createGridTexture(Grid &grid);
    void updateGridTexture(Grid &grid);
    void updatePixels(Grid &grid);
    void renderTexture();
    bool shouldWindowClose();
    void changeVSync();
    void fps();
    void Render(Grid &grid);
    inline bool Pawsed() {return pawsed && !nextFrame; };
    void Terminate();


    //todo fix?
    const int WIDTH = 1600;
    const int HEIGHT = 800;
    const int SCALE = 4;
    const int GRID_WIDTH = WIDTH / SCALE;
    const int PIXEL_SIZE = WIDTH / GRID_WIDTH;

    Camera camera {
        0, 0,
        GRID_WIDTH,                // visible cells horizontally
        HEIGHT / SCALE             // visible cells vertically
    };

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