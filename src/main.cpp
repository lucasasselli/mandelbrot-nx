#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include <unistd.h>

#include "device.h"

#define MAND_ITER  256

const GLfloat palette_data[] =
{
    0.0000, 0.0275, 0.3922,
    0.1255, 0.4196, 0.7961,
    0.9294, 1.0000, 1.0000,
    1.0000, 0.6667, 0.0000,
    0.0000, 0.0078, 0.0000,
    0.0000, 0.0275, 0.3922
};

GLfloat cx = 0.0f;
GLfloat cy = 0.0f;
GLfloat zoom = 1.0f;

static void errorCallback(int error, const char* description)
{
    printf("Error: %s\n", description);
    exit(EXIT_FAILURE);
}

static void frameBufferSizeCallback(GLFWwindow* window, int width, int height)
{
	if (!width || !height)
		return;

	glViewport(0, 0, width, height);
}

double oldTime = 0;

static void handleGamepad(GLFWwindow* window, const GLFWgamepadstate& gamepad)
{
	double curTime = glfwGetTime();
	float deltaTime = curTime - oldTime;
    oldTime = curTime;

    float k = deltaTime/0.1;

	const bool left_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS;
	const bool right_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;
	const bool up_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
	const bool down_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;

	const bool zoomin_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] == GLFW_PRESS;
	const bool zoomout_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER] == GLFW_PRESS;

	const bool exit_pressed  = gamepad.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS;

	// const float axis_x =  gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
	// const float axis_y = -gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
	// const float axis_magnitude = sqrtf(axis_x*axis_x + axis_y*axis_y);
	//const float axis_angle = atan2f(axis_y, axis_x);

    if (exit_pressed)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (up_pressed && cy < 1.0)
        cy += 0.1*zoom*k;

    if (down_pressed && cy > -1.0)
        cy -= 0.1*zoom*k;

    if (right_pressed && cx < 1.0)
        cx += 0.1*zoom*k;

    if (left_pressed && cx > -2.0)
        cx -= 0.1*zoom*k;

    if (zoomout_pressed && zoom < 1.0f)
        zoom *= (1.0f + k*0.5f);

    if (zoomin_pressed)
        zoom /= (1.0f + k*0.5f);
}

static GLuint setupShader(GLenum type, const char* path)
{
    GLint success;
    GLchar msg[512];

	FILE* f = fopen(path, "rb");
	if (!f)
	{
		printf("Could not open shader: %s\n", path);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	size_t shader_size = ftell(f);
	rewind(f);

	char* source = new char[shader_size+1];
	fread(source, 1, shader_size, f);
	source[shader_size] = 0;
	fclose(f);

    GLuint handle = glCreateShader(type);
    if (!handle)
    {
        printf("%u: cannot create shader", type);
        return 0;
    }
    glShaderSource(handle, 1, &source, nullptr);
	delete[] source;

    glCompileShader(handle);
    glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(handle, sizeof(msg), nullptr, msg);
        printf("%u: %s\n", type, msg);
        glDeleteShader(handle);
        return 0;
    }

    return handle;
}

static GLuint shaderProgram;
static GLuint VAO, VBO, EBO;
static GLuint texture;

static void sceneInit()
{
    GLint vsh = setupShader(GL_VERTEX_SHADER, RES("shaders/vertex_shader.glsl"));
    GLint fsh = setupShader(GL_FRAGMENT_SHADER, RES("shaders/fragment_shader.glsl"));

    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vsh);
    glAttachShader(shaderProgram, fsh);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char buf[512];
        glGetProgramInfoLog(shaderProgram, sizeof(buf), nullptr, buf);
        printf("Link error: %s", buf);
    }
    glDeleteShader(vsh);
    glDeleteShader(fsh);

    static const GLfloat vertices[] =
    {
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f
    };

    unsigned int edges[] = {
        0, 1, 2,
        1, 3, 2
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenTextures(1, &texture);

    // bind Vertex Array Object
    glBindVertexArray(VAO);

    // copy our vertices array in a vertex buffer for OpenGL to use
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // copy our index array in a element buffer for OpenGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(edges), edges, GL_STATIC_DRAW);

    // then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    // texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, texture);

    // texture sampling/filtering operation.
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage1D(
        GL_TEXTURE_1D,      // Specifies the target texture. Must be GL_TEXTURE_1D or GL_PROXY_TEXTURE_1D.
        0,                  // Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
        GL_RGBA32F,
        6,
        0,                  // border: This value must be 0.
        GL_RGB,
        GL_FLOAT,
        palette_data
    );

    assert(glGetError() == GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_1D, 0);
}

static void sceneRender()
{
    // glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, texture);

    GLuint paletteLoc = glGetUniformLocation(shaderProgram, "palette");
    GLuint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    GLuint zoomLoc = glGetUniformLocation(shaderProgram, "zoom");
    GLuint mandIterLoc = glGetUniformLocation(shaderProgram, "mand_iter");

    glUniform1i(paletteLoc, 0);
    glUniform1f(zoomLoc, zoom);
    glUniform2f(offsetLoc, cx, cy);
    glUniform1f(mandIterLoc, MAND_ITER);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

static void sceneExit()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProgram);
}

int main(void)
{
    deviceInit();
    printf("Program started!\n");

    // Init GLFW
    glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
    glfwSetErrorCallback(errorCallback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Mandelbrot", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, frameBufferSizeCallback);
    gladLoadGL();
    glfwSwapInterval(1);

    // Start rendering
    sceneInit();

	GLFWgamepadstate gamepad = {};

    while (!glfwWindowShouldClose(window))
    {
        sceneRender();

        // Read gamepad
		if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
		{
			// Gamepad not available, so let's fake it with keyboard
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]  = glfwGetKey(window, GLFW_KEY_A);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = glfwGetKey(window, GLFW_KEY_D);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]    = glfwGetKey(window, GLFW_KEY_W);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]  = glfwGetKey(window, GLFW_KEY_S);

			gamepad.buttons[GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER] = glfwGetKey(window, GLFW_KEY_E);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER]  = glfwGetKey(window, GLFW_KEY_Q);

			gamepad.buttons[GLFW_GAMEPAD_BUTTON_START] = glfwGetKey(window, GLFW_KEY_ESCAPE);
		}
        handleGamepad(window, gamepad);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Terminate graphics
    sceneExit();
    glfwDestroyWindow(window);
    glfwTerminate();

    // Terminate device
    printf("Program complete!\n");
    deviceStop();

    exit(EXIT_SUCCESS);
}