#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// ==============================
// #define GLAD_GL_IMPLEMENTATION
// #include <glad/gl.h>
// ==============================
// ^ not needed with shader_s.h ^

#include <openGL/shader_s.h>

#include <glm-master/glm/glm.hpp>
#include <glm-master/glm/gtc/matrix_transform.hpp>
#include <glm-master/glm/gtc/type_ptr.hpp>

#include <vector>

#include <set>
#include <stdlib.h>
#include <iostream>

#include <Simulation.h>
#include <Material.h>
#include <Grid.h>
#include <Colour.h>


// const int WIDTH = 1920;
// const int HEIGHT = 1080;
const int WIDTH = 1600;
const int HEIGHT = 800;
const int SCALE = 4;
const int GRID_WIDTH = WIDTH / SCALE;
const int GRID_HEIGHT = HEIGHT / SCALE;
const int PIXEL_SIZE = WIDTH / GRID_WIDTH;


static MaterialID DRAW = 1;
static bool CanSwapDraw = true; 
static bool wasDrawingLastFrame = false;
static bool nextFrame = false;
static bool canNextFrame = true;
static bool pawsed = true;
static bool canPaws = true;
static bool vSync = true;
static bool canChangevSync = true;


Grid grid(GRID_WIDTH, GRID_HEIGHT);
Simulation sim(grid);

static double lastTime = glfwGetTime();
static int nbFrames = 0;

static int Seed = 0;

static std::vector<unsigned char> pixels(GRID_WIDTH * GRID_HEIGHT * 4);

static unsigned int texture;
static unsigned int VBO, VAO, EBO;

// void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
//     if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
//         double xpos, ypos;
//         glfwGetCursorPos(window, &xpos, &ypos);
//         int gx = xpos / PIXEL_SIZE;
//         int gy = ypos / PIXEL_SIZE;
//         if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
//             grid[gy][gx] = SAND;
//         }
//     }
// }

void handleMouse(GLFWwindow* window, Grid &grid) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int gx = xpos / PIXEL_SIZE;
        int gy = ypos / PIXEL_SIZE;
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
            if (grid.at(gx, gy).material != DRAW){
                grid.at(gx, gy) = {
                    DRAW,                 // MaterialID
                    mat(DRAW).temperature,
                    mat(DRAW).lifeTime,
                };
                grid.activeNow->addActive(gx, gy);
                wasDrawingLastFrame = true;
            }

        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
        wasDrawingLastFrame = false;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int gx = xpos / PIXEL_SIZE;
        int gy = ypos / PIXEL_SIZE;
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
            grid.at(gx, gy) = {
                MAT_EMPTY,
                mat(MAT_EMPTY).temperature,
                mat(MAT_EMPTY).lifeTime,
            };
            for (int x = -1; x < 2; x++){
                for (int y = -1; y < 2; y++)
                {
                    if (!(x == 0 && y == 0)){
                        grid.activeNow->addActive(gx + x, gy + y);
                    }
                }   
            }
            wasDrawingLastFrame = true;
        }
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
        wasDrawingLastFrame = false;
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && CanSwapDraw) {
        CanSwapDraw = false;

        if (DRAW < lastMaterial)
        {
            DRAW++;
        } else {
            DRAW = 1;
        }
        std::cout << mat(DRAW).name << '\n';
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
        CanSwapDraw = true;

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && canPaws) {
        if (pawsed) {
            pawsed = false;
        } else {
            pawsed = true;
        }
        canPaws = false;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        canPaws = true;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && canNextFrame) {
        nextFrame = true;
        canNextFrame = false;
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        canNextFrame = true;
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS && canChangevSync) {
        vSync = !vSync;
        canChangevSync = false;
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_RELEASE) {
        canChangevSync = true;
    }
    
}

GLuint createGridTexture(Grid &grid) {
    const int texWidth  = GRID_WIDTH;
    const int texHeight = GRID_HEIGHT;

    // std::vector<unsigned char> pixels(texWidth * texHeight * 4);

    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            Colour c = materialToColor((grid.at(x, y).material));
            int index = (y * texWidth + x) * 4;

            pixels[index + 0] = c.r;
            pixels[index + 1] = c.g;
            pixels[index + 2] = c.b;
            pixels[index + 3] = c.a;
        }
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA8,
        texWidth,
        texHeight,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );

    // Nearest-neighbor keeps the grid crisp when scaled
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void updateGridTexture(GLuint textureID, Grid &grid) {

    int x, y;
    for (int i : grid.activeNow->active){
        x = grid.ix(i);
        y = grid.iy(i);

        if(y < 0 || y >= GRID_HEIGHT || x >= GRID_WIDTH || x < 0) {
            continue;
        }

        Colour c = materialToColor(grid.at(x, y).material);
        pixels[i*4 + 0] = c.r;
        pixels[i*4 + 1] = c.g;
        pixels[i*4 + 2] = c.b;
        pixels[i*4 + 3] = c.a;
    }

    Colour c = materialToColor(DRAW);
    pixels[0] = c.r;
    pixels[1] = c.g;
    pixels[2] = c.b;
    pixels[3] = c.a;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        GRID_WIDTH,
        GRID_HEIGHT,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );
}

void renderTexture(Shader &ourShader) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind Texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // render container
    ourShader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void fps() {
    double currentTime = glfwGetTime();
    nbFrames++;
    if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1 sec ago
        // printf and reset timer
        printf("%f ms/frame\t %f fps\n", 1000.0/double(nbFrames), double(nbFrames));
        //printf("%f fps\n", double(nbFrames));
        nbFrames = 0;
        lastTime += 1.0;
    }
}

int main() {

    prepMaterials();
    if (!loadMaterialsFromJSON("materials.json")) return -1;

    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Falling Sand", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }


    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // glfwSetMouseButtonCallback(window, mouse_button_callback);
    gladLoadGL(glfwGetProcAddress);

    // build and compile our shader program
    // ------------------------------------
    Shader ourShader("shaders\\vertexShader.vert", "shaders\\fragmentShader.frag"); 

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);


    // load and create a texture 
    // -------------------------
    texture = createGridTexture(grid);

    glOrtho(0, WIDTH, HEIGHT, 0, -1, 1); // Simple 2D projection

    srand(Seed);

    while (!glfwWindowShouldClose(window)) {
        fps();
        processInput(window);
        handleMouse(window, grid);
        
        if (!(pawsed && !nextFrame)) {
            sim.step();
        }

        nextFrame = false;
        

        updateGridTexture(texture, grid);
        renderTexture(ourShader);

        // std::shuffle(active.begin(), active.end(), rng);

        glfwSwapBuffers(window);
        glfwPollEvents();

        if (!canChangevSync){
            glfwSwapInterval(vSync);
        }
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}
