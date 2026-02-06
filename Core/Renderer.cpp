#include "Renderer.h"
#include "Materials\Material.h"
#include "Colour.h"

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


Renderer::Renderer(int width, int height) : texWidth(width), texHeight(height), pixels(width * height * 4) {


    if (!glfwInit()) throw -1;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(texWidth, texHeight, "Falling Sand", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw -1;
    }


    DrawMaterial = 1;
    CanSwapDraw = true; 
    wasDrawingLastFrame = false;
    nextFrame = false;
    canNextFrame = true;
    pawsed = true;
    canPaws = true;
    vSync = true;
    canChangevSync = true;

}

void Renderer::prepWindow(Grid &grid) {

    lastTime = glfwGetTime();


    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // glfwSetMouseButtonCallback(window, mouse_button_callback);
    gladLoadGL(glfwGetProcAddress);

    // build and compile our shader program
    shader = new Shader("shaders\\vertexShader.vert", "shaders\\fragmentShader.frag");

    
    float u0 = (float)camera.x / grid.width;
    float v0 = (float)camera.y / grid.height;

    float u1 = (float)(camera.x + camera.width)  / grid.width;
    float v1 = (float)(camera.y + camera.height) / grid.height;


    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   u1, v0, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   u1, v1, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   u0, v1, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   u0, v0  // top left 
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

    texture = createGridTexture(grid);

    glOrtho(0, texWidth, texHeight, 0, -1, 1); // Simple 2D projection
}

void Renderer::handleMouse(Grid &grid) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int gx = xpos / PIXEL_SIZE;
        int gy = ypos / PIXEL_SIZE;
        if (gx >= 0 && gx < grid.width && gy >= 0 && gy < grid.height) {
            if (grid.at(gx, gy).material != DrawMaterial){
                grid.at(gx, gy) = {
                    DrawMaterial,                 // MaterialID
                    mat(DrawMaterial).temperature,
                    mat(DrawMaterial).lifeTime,
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
        if (gx >= 0 && gx < grid.width && gy >= 0 && gy < grid.height) {
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

void Renderer::processInput()
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS && CanSwapDraw) {
        CanSwapDraw = false;

        if (DrawMaterial < lastMaterial)
        {
            DrawMaterial++;
        } else {
            DrawMaterial = 1;
        }
        std::cout << mat(DrawMaterial).name << '\n';
    }

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
        CanSwapDraw = true;
    
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && CanSwapDraw) {
        CanSwapDraw = false;

        if (DrawMaterial > 1)
        {
            DrawMaterial--;
        } else {
            DrawMaterial = lastMaterial;
        }
        std::cout << mat(DrawMaterial).name << '\n';
    }

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

GLuint Renderer::createGridTexture(Grid &grid) {

    // std::vector<unsigned char> pixels(texWidth * texHeight * 4);

    for (int y = 0; y < grid.height; ++y) {
        for (int x = 0; x < grid.width; ++x) {
            Colour c = materialToColor(grid.at(x, y).material);
            int index = (y * grid.width + x) * 4;

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
        grid.width,
        grid.height,
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

void Renderer::updateGridTexture(Grid &grid) {

    int x, y;
    for (int i : grid.activeNow->active){
        x = grid.ix(i);
        y = grid.iy(i);

        if(y < 0 || y >= grid.height || x >= grid.width || x < 0) {
            continue;
        }

        Colour c = materialToColor(grid.at(x, y).material);
        pixels[i*4 + 0] = c.r;
        pixels[i*4 + 1] = c.g;
        pixels[i*4 + 2] = c.b;
        pixels[i*4 + 3] = c.a;
    }

    Colour c = materialToColor(DrawMaterial);
    pixels[0] = c.r;
    pixels[1] = c.g;
    pixels[2] = c.b;
    pixels[3] = c.a;

    glBindTexture(GL_TEXTURE_2D, texture);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0,
        0,
        0,
        grid.width,
        grid.height,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        pixels.data()
    );
}

void Renderer::updatePixels(Grid &grid) {
    int x, y;
    glBindTexture(GL_TEXTURE_2D, texture);

    for (int i : grid.activeNow->active)
    {
        int x = grid.ix(i);
        int y = grid.iy(i);

        if (x < 0 || x >= grid.width || y < 0 || y >= grid.height)
            continue;

        Colour c = materialToColor(grid.at(x, y).material);

        unsigned char pixel[4] = { c.r, c.g, c.b, c.a };

        // Upload exactly ONE pixel
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            x,
            y,
            1,
            1,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            pixel
        );
    }
}

void Renderer::renderTexture() {
    float u0 = (float)camera.x / GRID_WIDTH;
    float v0 = (float)camera.y / (HEIGHT / SCALE);

    float u1 = (float)(camera.x + camera.width)  / GRID_WIDTH;
    float v1 = (float)(camera.y + camera.height) / (HEIGHT / SCALE);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    float vertices[] = {
        // positions          // colors           // texture coords
         1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   u1, v0, // top right
         1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   u1, v1, // bottom right
        -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   u0, v1, // bottom left
        -1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   u0, v0  // top left 
    };

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);   

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // bind Texture
    glBindTexture(GL_TEXTURE_2D, texture);

    // render container
    (*shader).use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

bool Renderer::shouldWindowClose() {
    return glfwWindowShouldClose(window);
}

void Renderer::changeVSync() {
    if (canChangevSync) glfwSwapInterval(vSync);
}

void Renderer::fps() {
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

void Renderer::Render(Grid &grid) {
    fps();
    nextFrame = false;

    processInput();
    handleMouse(grid);
    
    updateGridTexture(grid);
    renderTexture();

    glfwSwapBuffers(window);
    glfwPollEvents();

    changeVSync();
}

void Renderer::Terminate() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
}