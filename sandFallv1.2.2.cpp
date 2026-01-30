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
#include <algorithm>

#include <Material.h>


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
static int mouseX, mouseY;
static bool nextFrame = false;
static bool canNextFrame = true;
static bool pawsed = true;
static bool canPaws = true;
static bool vSync = true;
static bool canChangevSync = true;


static double lastTime = glfwGetTime();
static int nbFrames = 0;

static int Seed = 0;
uint32_t rng = 0x12345678;

std::vector<int> active;
std::vector<uint8_t> activeMask(GRID_WIDTH * GRID_HEIGHT, 0);
std::vector<int> activeNext;
std::vector<uint8_t> activeNextMask(GRID_WIDTH * GRID_HEIGHT, 0);


static std::vector<Pixel> grid(GRID_HEIGHT * GRID_WIDTH, {
    (MaterialID)0, // can't use materialIdByName("Empty") or MAT_EMPTY as g_materials hasn't been populated yet, Empty is hard coded to be id: 0
    roomTemp,
    -1,
});
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

inline int idx(int x, int y) {
    return y * GRID_WIDTH + x;
}

inline int ix(int i) { return i % GRID_WIDTH; }
inline int iy(int i) { return i / GRID_WIDTH; }


inline bool randBit() {
    rng ^= rng << 13;
    rng ^= rng >> 17;
    rng ^= rng << 5;
    return rng & 1;
}

struct Color {
    unsigned char r, g, b, a;
};

Color materialToColor(MaterialID id) {
    // aarrggbb
    uint32_t c = mat(id).color;
    return {
        (unsigned char)((c >> 16) & 0xFF),  // r
        (unsigned char)((c >> 8)  & 0xFF),  // g
        (unsigned char)(c & 0xFF),          // b
        (unsigned char)((c >> 24) & 0xFF)   // a
    };
}

inline void addActive(std::vector<int> &active, std::vector<uint8_t> &activeMask, int x, int y) {
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;

    int i = y * GRID_WIDTH + x;
    if (!activeMask[i]) {
        activeMask[i] = 1;
        active.push_back(i);
    }
}


void handleMouse(GLFWwindow* window) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int gx = xpos / PIXEL_SIZE;
        int gy = ypos / PIXEL_SIZE;
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
            if (grid[gy * GRID_WIDTH + gx].material != DRAW){
                grid[gy * GRID_WIDTH + gx] = {
                    DRAW,                 // MaterialID
                    mat(DRAW).temperature,
                    mat(DRAW).lifeTime,
                };
                addActive(active, activeMask, gx, gy);
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
            grid[gy * GRID_WIDTH + gx] = {
                MAT_EMPTY,
                mat(MAT_EMPTY).temperature,
                mat(MAT_EMPTY).lifeTime,
            };
            for (int x = -1; x < 2; x++){
                for (int y = -1; y < 2; y++)
                {
                    if (!(x == 0 && y == 0)){
                        addActive(active, activeMask, gx + x, gy + y);
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

        if (DRAW < (MaterialID)numOfMaterials - 1)
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


void updateSub() {
    int x, y, xn, xp, yn, yp;
    bool moved;


    for (int i : active){

        activeMask[i] = 0;
        moved = false;
        x = ix(i);
        y = iy(i);

        if(y < 0 || y > GRID_HEIGHT - 2 || x > GRID_WIDTH - 1 || x < 0) {
            continue;
        }
        

        Pixel& p = grid[y * GRID_WIDTH + x];
        const MaterialDef& m = mat(p.material);


        // current iteration using isdenserThan causes unwanted behavere with Liquid FIX
        if (m.state == MaterialState::Powder) {                                                  // Powder
            if (isdenserThan(p, grid[(y + 1) * GRID_WIDTH + x])) {                                  // Down
                std::swap(grid[y * GRID_WIDTH + x], grid[(y + 1) * GRID_WIDTH + x]);
                moved = true;
            } else if (x > 0 && isdenserThan(p, grid[(y + 1) * GRID_WIDTH + (x - 1)])) {              // Down Left
                std::swap(grid[y * GRID_WIDTH + x], grid[(y + 1) * GRID_WIDTH + (x - 1)]);
                moved = true;
            } else if (x < GRID_WIDTH - 1 && isdenserThan(p, grid[(y + 1) * GRID_WIDTH + (x + 1)])) { // Down Right
                std::swap(grid[y * GRID_WIDTH + x], grid[(y + 1) * GRID_WIDTH + (x + 1)]);
                moved = true;
            }
        }

        if (m.state == MaterialState::Liquid) {                                                  // Liquid
            if (isdenserThan(p, grid[(y + 1) * GRID_WIDTH + x])) {                                  // Down
                std::swap(grid[y * GRID_WIDTH + x], grid[(y + 1) * GRID_WIDTH + x]);
                moved = true;
            } else if (x > 0 && isdenserThan(p, grid[(y + 1) * GRID_WIDTH + (x - 1)])) {              // Down Left
                std::swap(grid[y * GRID_WIDTH + x], grid[(y + 1) * GRID_WIDTH + (x - 1)]);
                moved = true;
            } else if (x < GRID_WIDTH - 1 && isdenserThan(p, grid[(y + 1) * GRID_WIDTH + (x + 1)])) { // Down Right
                std::swap(grid[y * GRID_WIDTH + x], grid[(y + 1) * GRID_WIDTH + (x + 1)]);
                moved = true;
            } else if (randBit()){                                                           // randomise
                if (x > 0 && grid[y * GRID_WIDTH + (x - 1)].material == MAT_EMPTY) {                     // Left
                    std::swap(grid[y * GRID_WIDTH + x], grid[y * GRID_WIDTH + (x - 1)]);
                    moved = true;
                } else if (x < GRID_WIDTH - 1 && grid[y * GRID_WIDTH + (x + 1)].material == MAT_EMPTY) { // Right
                    std::swap(grid[y * GRID_WIDTH + x], grid[y * GRID_WIDTH + (x + 1)]);
                    moved = true;
                }
            } else {
                if (x < GRID_WIDTH - 1 && grid[ y * GRID_WIDTH + (x + 1)].material == MAT_EMPTY) {        // Right
                    std::swap(grid[y * GRID_WIDTH + x], grid[y * GRID_WIDTH + (x + 1)]);
                    moved = true;
                } else if (x > 0 && grid[ y * GRID_WIDTH + (x - 1)].material == MAT_EMPTY) {              // Left
                    std::swap(grid[y * GRID_WIDTH + x], grid[y * GRID_WIDTH + (x - 1)]);
                    moved = true;
                }
            }
        }


        if (p.life > 0)
        { 
            moved = true;
            if (--p.life == 0)
            {
                p.material = MAT_EMPTY;
            }
        }

        if (moved)
        {
            xn = x - 1;
            xp = x + 1;
            yn = y - 1;
            yp = y + 1;
            
            addActive(activeNext, activeNextMask, xp, yp); //se
            addActive(activeNext, activeNextMask, x , yp); //s
            addActive(activeNext, activeNextMask, xn, yp); //sw
            addActive(activeNext, activeNextMask, xp, y ); // e
            addActive(activeNext, activeNextMask, x , y ); // c
            addActive(activeNext, activeNextMask, xn, y ); // w
            addActive(activeNext, activeNextMask, xp, yn); //ne
            addActive(activeNext, activeNextMask, x , yn); //n
            addActive(activeNext, activeNextMask, xn, yn); //nw

        }
    }
    active.clear();

    std::swap(active, activeNext);
    std::swap(activeMask, activeNextMask);
}

void update(){
    
    if (pawsed && !nextFrame) {
        return;
    }

    updateSub();

    // if (active.size() > 4000){
    //     std::cout << active.size() << '\n';
    //     if (active.size() > 1000000) {
    //         throw;
    //     }
    // }

    nextFrame = false;
}


// writen by chatGPT when stuff breaks Blame this first
GLuint createGridTexture(const std::vector<Pixel>& grid) {
    const int texWidth  = GRID_WIDTH;
    const int texHeight = GRID_HEIGHT;

    // std::vector<unsigned char> pixels(texWidth * texHeight * 4);

    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            Color c = materialToColor(grid[y * GRID_WIDTH + x].material);
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

// writen by chatGPT when stuff breaks Blame this first
void updateGridTexture(GLuint textureID, const std::vector<Pixel>& grid) {


    int x, y;
    for (int i : active){
        x = ix(i);
        y = iy(i);

        if(y < 0 || y >= GRID_HEIGHT || x >= GRID_WIDTH || x < 0) {
            continue;
        }

        Color c = materialToColor(grid[ y * GRID_WIDTH + x].material);
        pixels[i*4 + 0] = c.r;
        pixels[i*4 + 1] = c.g;
        pixels[i*4 + 2] = c.b;
        pixels[i*4 + 3] = c.a;
    }

    Color c = materialToColor(DRAW);
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

    active.reserve(50000);
    activeNext.reserve(50000);

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

        // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("shaders\\sandFallv.1.2.2\\sandfallv1.2.2.vert", "shaders\\sandFallv.1.2.2\\sandfallv1.2.2.frag"); 

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
        handleMouse(window);
        
        // renderBoxs();
        // renderUpdatesBoxs(set);
        updateGridTexture(texture, grid);
        renderTexture(ourShader);
        update();
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
