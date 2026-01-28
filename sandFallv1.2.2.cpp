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

#include <CellData.h>
#include <Material.h>

class CellDataLocal : CellData{
public:
    int x;
    int y;

    CellDataLocal(int p_x, int p_y)
    {
        x = p_x;
        y = p_y;
    }

    bool operator< (const CellDataLocal& other) const
    {
        if (y != other.y)
            return y > other.y;
        return x > other.x;
    }
};


// const int WIDTH = 1920;
// const int HEIGHT = 1080;
const int WIDTH = 1600;
const int HEIGHT = 800;
const int SCALE = 4;
const int GRID_WIDTH = WIDTH / SCALE;
const int GRID_HEIGHT = HEIGHT / SCALE;
const int PIXEL_SIZE = WIDTH / GRID_WIDTH;


MaterialID DRAW = 1;
bool CanSwapDraw = true; 
bool wasDrawingLastFrame = false;
int mouseX, mouseY;
bool nextFrame = false;
bool canNextFrame = true;
bool pawsed = true;
bool canPaws = true;
bool vSync = true;
bool canChangevSync = true;


double lastTime = glfwGetTime();
int nbFrames = 0;

int Seed = 0;

std::set<CellDataLocal> set;
std::vector<std::vector<Pixel>> grid(GRID_HEIGHT+2, std::vector<Pixel>(GRID_WIDTH, {
    (MaterialID)0, // can't use materialIdByName("Empty") or MAT_EMPTY as g_materials hasn't been populated yet, Empty is hard coded to be id: 0
    20.0f,
    -1,
    false
}));
std::vector<unsigned char> pixels(GRID_WIDTH * GRID_HEIGHT * 4);

unsigned int texture;
unsigned int VBO, VAO, EBO;

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


void addPixel(std::set<CellDataLocal> &s, int x, int y){
    s.insert({x, y});
}

void handleMouse(GLFWwindow* window) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int gx = xpos / PIXEL_SIZE;
        int gy = ypos / PIXEL_SIZE;
        if (gx >= 0 && gx < GRID_WIDTH && gy >= 0 && gy < GRID_HEIGHT) {
            if (grid[gy][gx].material != DRAW){
                grid[gy][gx] = {
                    DRAW,                 // MaterialID
                    mat(DRAW).temperature,
                    mat(DRAW).lifeTime,
                    false
                };
                addPixel(set, gx, gy);
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
            grid[gy][gx] = {
                MAT_EMPTY,
                mat(MAT_EMPTY).temperature,
                mat(MAT_EMPTY).lifeTime,
                false
            };
            for (int x = -1; x < 2; x++){
                for (int y = -1; y < 2; y++)
                {
                    if (!(x == 0 && y == 0)){
                        addPixel(set, gx + x, gy + y);
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


// Base
// void updateSub(std::set<CellDataLocal> &setCurrent) {
//     for (int y = GRID_HEIGHT - 2; y >= 0; --y) {
//         for (int x = 0; x < GRID_WIDTH; ++x) {
//             if (grid[y][x] == SAND) {                                           // Sand
//                 if (grid[y + 1][x] == WATER || grid[y + 1][x] == EMPTY) {                                  // Down
//                     std::swap(grid[y][x], grid[y + 1][x]);
//                 } else if (x > 0 && (grid[y + 1][x - 1] == WATER || grid[y + 1][x - 1] == EMPTY)) {              // Down Left
//                     std::swap(grid[y][x], grid[y + 1][x - 1]);
//                 } else if (x < GRID_WIDTH - 1 && (grid[y + 1][x + 1] == WATER || grid[y + 1][x + 1] == EMPTY)) { // Down Right
//                     std::swap(grid[y][x], grid[y + 1][x + 1]);
//                 } else {
//                     continue;
//                 }
//             }
//             if (grid[y][x] == WATER) {                                          // Water
//                 if (grid[y + 1][x] == EMPTY) {                                  // Down
//                     std::swap(grid[y][x], grid[y + 1][x]);
//                 } else if (x > 0 && grid[y + 1][x - 1] == EMPTY) {              // Down Left
//                     std::swap(grid[y][x], grid[y + 1][x - 1]);
//                 } else if (x < GRID_WIDTH - 1 && grid[y + 1][x + 1] == EMPTY) { // Down Right
//                     std::swap(grid[y][x], grid[y + 1][x + 1]);
//                 } else if (rand()%2 == 0){
//                     if (x > 0 && grid[y][x - 1] == EMPTY) {                     // Left
//                         std::swap(grid[y][x], grid[y][x - 1]);
//                     } else if (x < GRID_WIDTH - 1 && grid[y][x + 1] == EMPTY) { // Right
//                         std::swap(grid[y][x], grid[y][x + 1]);
//                     }
//                 } else {
//                     if (x < GRID_WIDTH - 1 && grid[y][x + 1] == EMPTY) {        // Right
//                         std::swap(grid[y][x], grid[y][x + 1]);
//                     } else if (x > 0 && grid[y][x - 1] == EMPTY) {              // Left
//                         std::swap(grid[y][x], grid[y][x - 1]);
//                     }
//                 }
//             }
//         }
//     }
// }


// First Try
// void updateSub(std::vector<CellDataLocal> stackCurrent, std::vector<CellDataLocal> stackNext) {
//     int x, y;
//     CellType type;
//     if (stackCurrent.empty()){
//         std::cout<< "null";
//     }
//     for (CellDataLocal cell = stackCurrent.back(); !stackCurrent.empty(); cell = stackCurrent.back()) {
// 
//         if (stackCurrent.size() > 4000){
//             return;
//         }
//         std::cout << stackCurrent.size() << ","; // << cell.x << "," << cell.y;
//         x = cell.x;
//         y = cell.y;
// 
// 
//         type = grid[y][x];        
//         // stackCurrent.pop_back();
//         if (type == EMPTY || type == WALL){
//             std::cout << "v";
//             stackCurrent.pop_back();
//             continue;
//         }
// 
//         if (grid[y][x] == SAND) { 
//             std::cout <<  "Snad";                                       // Sand
//             stackCurrent.pop_back();
//             if (grid[y + 1][x] == EMPTY) {                                  // Down
//                 std::swap(grid[y][x], grid[y + 1][x]);
//             } else if (x > 0 && grid[y + 1][x - 1] == EMPTY) {              // Down :eft
//                 std::swap(grid[y][x], grid[y + 1][x - 1]);
//             } else if (x < GRID_WIDTH - 1 && grid[y + 1][x + 1] == EMPTY) { // Down Right
//                 std::swap(grid[y][x], grid[y + 1][x + 1]);
//             } else {
//                 continue;
//             }
//         }
// 
//         // if (grid[y][x] == WATER) {                                          // Water
//         //     if (grid[y + 1][x] == EMPTY) {                                  // Down
//         //         std::swap(grid[y][x], grid[y + 1][x]);
//         //     } else if (x > 0 && grid[y + 1][x - 1] == EMPTY) {              // Down Left
//         //         std::swap(grid[y][x], grid[y + 1][x - 1]);
//         //     } else if (x < GRID_WIDTH - 1 && grid[y + 1][x + 1] == EMPTY) { // Down Right
//         //         std::swap(grid[y][x], grid[y + 1][x + 1]);
//         //     } else if (rand()%2 == 0){
//         //         if (x > 0 && grid[y][x - 1] == EMPTY) {                     // Left
//         //             std::swap(grid[y][x], grid[y][x - 1]);
//         //         } else if (x < GRID_WIDTH - 1 && grid[y][x + 1] == EMPTY) { // Right
//         //             std::swap(grid[y][x], grid[y][x + 1]);
//         //         }
//         //     } else {
//         //         if (x < GRID_WIDTH - 1 && grid[y][x + 1] == EMPTY) {        // Right
//         //             std::swap(grid[y][x], grid[y][x + 1]);
//         //         } else if (x > 0 && grid[y][x - 1] == EMPTY) {              // Left
//         //             std::swap(grid[y][x], grid[y][x - 1]);
//         //         }
//         //     }
//         // }
//         // stackCurrent.pop_back();
// 
//         stackNext.push_back({x - 1, y - 1}); //nw
//         stackNext.push_back({x - 1, y    }); // w
//         stackNext.push_back({x - 1, y + 1}); //sw
//         stackNext.push_back({x    , y + 1}); //s
//         stackNext.push_back({x + 1, y + 1}); //se
//         stackNext.push_back({x + 1, y    }); // e
//         stackNext.push_back({x + 1, y - 1}); //ne
//         stackNext.push_back({x    , y - 1}); //n
//         std::cout << stackNext.size();
//     }
//     stackCurrent.clear();
//     std::cout << "c" << stackFlag << " ";
// }


void updateSub(std::set<CellDataLocal> &setCurrent) {
    int x, y, xn, xp, yn, yp;
    bool moved;
    std::set<CellDataLocal> setNext;
    for (CellDataLocal cell : setCurrent){
        moved = false;
        x = cell.x;
        y = cell.y;
        if(y < 0 || y > GRID_HEIGHT - 2 || x > GRID_WIDTH - 1 || x < 0) {
            continue;
        }
        

        Pixel& p = grid[y][x];
        const MaterialDef& m = mat(p.material);


        // current iteration using isdenserThan causes unwanted behavere with Liquid FIX
        if (m.state == MaterialState::Powder) {                                                  // Powder
            if (isdenserThan(p, grid[y + 1][x])) {                                  // Down
                std::swap(grid[y][x], grid[y + 1][x]);
                moved = true;
            } else if (x > 0 && isdenserThan(p, grid[y + 1][x - 1])) {              // Down Left
                std::swap(grid[y][x], grid[y + 1][x - 1]);
                moved = true;
            } else if (x < GRID_WIDTH - 1 && isdenserThan(p, grid[y + 1][x + 1])) { // Down Right
                std::swap(grid[y][x], grid[y + 1][x + 1]);
                moved = true;
            }
        }

        if (m.state == MaterialState::Liquid) {                                                  // Liquid
            if (grid[y + 1][x].material == MAT_EMPTY) {                                  // Down
                std::swap(grid[y][x], grid[y + 1][x]);
                moved = true;
            } else if (x > 0 && grid[y + 1][x - 1].material == MAT_EMPTY) {              // Down Left
                std::swap(grid[y][x], grid[y + 1][x - 1]);
                moved = true;
            } else if (x < GRID_WIDTH - 1 && grid[y + 1][x + 1].material == MAT_EMPTY) { // Down Right
                std::swap(grid[y][x], grid[y + 1][x + 1]);
                moved = true;
            } else if (rand()%2 == 0){                                                           // randomise
                if (x > 0 && grid[y][x - 1].material == MAT_EMPTY) {                     // Left
                    std::swap(grid[y][x], grid[y][x - 1]);
                    moved = true;
                } else if (x < GRID_WIDTH - 1 && grid[y][x + 1].material == MAT_EMPTY) { // Right
                    std::swap(grid[y][x], grid[y][x + 1]);
                    moved = true;
                }
            } else {
                if (x < GRID_WIDTH - 1 && grid[y][x + 1].material == MAT_EMPTY) {        // Right
                    std::swap(grid[y][x], grid[y][x + 1]);
                    moved = true;
                } else if (x > 0 && grid[y][x - 1].material == MAT_EMPTY) {              // Left
                    std::swap(grid[y][x], grid[y][x - 1]);
                    moved = true;
                }
            }
        }

        // Pixel& here = grid[y][x];
        // Pixel& below = grid[y+1][x];

        // const MaterialDef& here = mat(grid[y][x].material);
        // const MaterialDef& below = mat(grid[y+1][x].material);

        // if (((here.state != MaterialState::Solid) && (below.state != MaterialState::Solid)) && (below.density < here.density)) {
        //     std::swap(grid[y][x], grid[y+1][x]);
        //     moved = true;
        // }

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
            // xn = abs(x - 1);
            // xp = (x + 1) % GRID_WIDTH;
            // yn = abs(y - 1);
            // yp = (y + 1) % GRID_HEIGHT;
            // addPixel(setNext, x , y ); // c
            addPixel(setNext, xn, y ); // w
            addPixel(setNext, xn, yp); //sw
            addPixel(setNext, x , yp); //s
            addPixel(setNext, xp, yp); //se
            addPixel(setNext, xp, y ); // e
            addPixel(setNext, xp, yn); //ne
            addPixel(setNext, x , yn); //n
            addPixel(setNext, xn, yn); //nw
        }
    }
    setCurrent.clear();
    std::swap(setCurrent, setNext);
}

void update(){
    
    if (pawsed && !nextFrame) {
        return;
    }

    updateSub(set);

    if (set.size() > 4000){
        std::cout << set.size() << '\n';
        if (set.size() > 1000000) {
            throw;
        }
    }

    nextFrame = false;
}


// writen by chatGPT when stuff breaks Blame this first
GLuint createGridTexture(const std::vector<std::vector<Pixel>>& grid) {
    const int texWidth  = GRID_WIDTH;
    const int texHeight = GRID_HEIGHT;

    // std::vector<unsigned char> pixels(texWidth * texHeight * 4);

    for (int y = 0; y < texHeight; ++y) {
        for (int x = 0; x < texWidth; ++x) {
            Color c = materialToColor(grid[y][x].material);
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
void updateGridTexture(GLuint textureID, const std::vector<std::vector<Pixel>>& grid, std::set<CellDataLocal> &setCurrent) {

    // std::vector<unsigned char> pixels(GRID_WIDTH * GRID_HEIGHT * 4);

    int x, y;
    for (CellDataLocal cell : setCurrent){
        x = cell.x;
        y = cell.y;

        if(y < 0 || y > GRID_HEIGHT - 1 || x > GRID_WIDTH - 1 || x < 0) {
            continue;
        }

        Color c = materialToColor(grid[y][x].material);
        int i = (y * GRID_WIDTH + x) * 4;
        pixels[i + 0] = c.r;
        pixels[i + 1] = c.g;
        pixels[i + 2] = c.b;
        pixels[i + 3] = c.a;
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
        //printf("%f ms/frame\n", 1000.0/double(nbFrames));
        printf("%f fps\n", double(nbFrames));
        nbFrames = 0;
        lastTime += 1.0;
    }
}

int main() {

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
        updateGridTexture(texture, grid, set);
        renderTexture(ourShader);
        update();

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
