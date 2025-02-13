#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
//#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include"cast_ray.h"
#include"LookAt.h"
#include<algorithm>
#include <iostream>
#pragma comment (lib, "glew32.lib")  

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"out vec3 ourColor;\n"
"out vec2 TexCoord;\n"
"void main()\n"
"{\n"
"gl_Position = vec4(aPos, 1.0);\n"
"ourColor = aColor;\n"
"TexCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
"}\0";

const char* fragmentShaderSource = "#version 330 core\n"
"out vec4 FragColor;\n"
"in vec3 ourColor;\n"
"in vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main()\n"
"{\n"
"   FragColor = texture(texture1, TexCoord);\n"
"}\n\0";


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Display RGB Array", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // // GLEW: load all OpenGL function pointers
    glewInit();

    // build and compile the shaders
    // ------------------------------------
    // vertex shader
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
        // positions          // colors           // texture coords
         0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
         0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
        -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
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
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Create the image (RGB Array) to be displayed
    const int width=8, height=8;
    //width = 8; height = 8; // keep it in powers of 2!
    unsigned char image[width * height * 3];
    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            int idx = (i * width + j) * 3;
            image[idx] = (unsigned char)(255 * i * j / height / width); //((i+j) % 2) * 255;
            image[idx + 1] = 0;
            image[idx + 2] = 0;
        }
    }

/*    sh1.color.diffuse_color = Vec3(0.1f, 0.4f, 0.3f);
    sh1.color.specular_exponent = 100.f;
    sh1.color.albedo = Vec4(0.9f, 0.1f, 0.0f, 0.0f);
    sh1.color.refractive_index = 0.2f;// 1.5f;
    sh1.color.difintensity = directional_light.intensity;

    sh2.color.diffuse_color = Vec3(0.7f, 0.1f, 0.4f);
    sh2.color.specular_exponent = 1400.f;
    sh2.color.albedo = Vec4(0.1f, 10.0f, 1.6f, 0.0f);
    sh2.color.refractive_index = 0.1f;// 1.f;
    sh2.color.difintensity = directional_light.intensity;

    tehd.color.diffuse_color = Vec3(0.1f, 0.75f, 0.9f);
    tehd.color.specular_exponent = 10.f;
    tehd.color.albedo = Vec4(0.4f, 0.5f, 0.1f, 0.8f);
    tehd.color.refractive_index = 0.05f;//1.0f;
    tehd.color.difintensity = directional_light.intensity;

    vector<Sphere> shseq;
    shseq.push_back(sh1);
    shseq.push_back(sh2);

    const int screenWidth = 1024;
    const int screenHeight = 768;
    const float fov = M_PI / 3.;
    std::vector<Vec3> framebuffer(screenWidth * screenHeight);
    unsigned char image[screenWidth * screenHeight * 3];*/

    /*int choice;
    cout << "please select a mode: 0--from origin��0 0 0��  1--from another direction:" << endl;
    cin >> choice;*/

/*#pragma omp parallel for
    for (size_t j = 0; j < screenHeight; j++) { // actual rendering loop
        for (size_t i = 0; i < screenWidth; i++) {
            float dir_x = (i + 0.5) - (float)screenWidth / 2.f; /// ((float)screenWidth / 2.f);// *eye.dx + eye.origin[0];
            float dir_y = -(j + 0.5) + (float)screenHeight / 2.f; /// ((float)screenHeight / 2.f);// *eye.dy + eye.origin[1];    // this flips the image at the same time
            float dir_z = -(float)screenHeight / (2.f * tan(fov / 2.));// *eye.dz + eye.origin[2];
            Vec3 node = Vec3(dir_x, dir_y, dir_z) / (dir_z * -1);
            Vec3 screen = eye.viewscreen(node) + eye.origin;*/
            //cout << screen.x << " " << screen.y << " " << screen.z << "  ";
            /*
            if (i == screenWidth - 1 && j == screenHeight - 1)
                cout << screen.x << " " << screen.y << " " << screen.z << "  " << node.x << " " << node.y << " " << node.z << endl;
            if (i == 0 && j == 0)
                cout << screen.x << " " << screen.y << " " << screen.z << "  " << node.x << " " << node.y << " " << node.z << endl;
            if (!choice)
                framebuffer[i + j * screenWidth] = Cast_Ray_Parallel(node, FORWARD, spheres, tehd);
            else
                framebuffer[i + j * screenWidth] = Cast_Ray_Parallel(screen, eye.direction, spheres, tehd);*/
                //framebuffer[i + j * screenWidth] = Cast_Ray_Shading(screen, eye.direction, spheres, tehd, lights);
/*            framebuffer[i + j * screenWidth] = Cast_Ray_Glazing(screen, eye.direction, shseq, tehd, rays);

            Vec3 c = framebuffer[i + j * screenWidth];
            float max = std::max(c[0], std::max(c[1], c[2]));
            if (max > 1) c = c * (1. / max);
            image[(i + j * screenWidth) * 3] = (char)(255 * c[0]);
            image[(i + j * screenWidth) * 3 + 1] = (char)(255 * c[1]);
            image[(i + j * screenWidth) * 3 + 2] = (char)(255 * c[2]);

        }
    }*/

    unsigned char* data = &image[0];
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }



    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // bind Texture
        glBindTexture(GL_TEXTURE_2D, texture);

        // render container
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}