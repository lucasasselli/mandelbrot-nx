#include <glad/glad.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <cassert>
#include <unistd.h>

#include "device.h"

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

static void handleGamepad(GLFWwindow* window, const GLFWgamepadstate& gamepad)
{
	// double curTime = glfwGetTime();
	// float deltaTime = curTime - s_updTime;
	// s_updTime = curTime;

	const bool left_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT] == GLFW_PRESS;
	const bool right_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] == GLFW_PRESS;
	const bool up_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP] == GLFW_PRESS;
	const bool down_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN] == GLFW_PRESS;

	const bool zoomin_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_TRIANGLE] == GLFW_PRESS;
	const bool zoomout_pressed = gamepad.buttons[GLFW_GAMEPAD_BUTTON_CROSS] == GLFW_PRESS;

	const bool exit_pressed  = gamepad.buttons[GLFW_GAMEPAD_BUTTON_START] == GLFW_PRESS;

	// const float axis_x =  gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
	// const float axis_y = -gamepad.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
	// const float axis_magnitude = sqrtf(axis_x*axis_x + axis_y*axis_y);
	//const float axis_angle = atan2f(axis_y, axis_x);

    if (exit_pressed)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    if (up_pressed)
        cy += 0.1*zoom;

    if (down_pressed)
        cy -= 0.1*zoom;

    if (right_pressed)
        cx += 0.1*zoom;

    if (left_pressed)
        cx -= 0.1*zoom;

    if (zoomout_pressed)
        zoom*=2.0;

    if (zoomin_pressed)
        zoom*=0.5;
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
        // printf("Link error: %s", buf);
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
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_1D, texture);

    GLuint samplerLoc = glGetUniformLocation(shaderProgram, "palette");
    GLuint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    GLuint zoomLoc = glGetUniformLocation(shaderProgram, "zoom");

    glUniform1i(samplerLoc, 0);
    glUniform1f(zoomLoc, zoom);
    glUniform2f(offsetLoc, cx, cy);

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Mandelbrot", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);

    sceneInit();

	GLFWgamepadstate gamepad = {};

    while (!glfwWindowShouldClose(window))
    {
        sceneRender();

        // Read gamepad
		if (!glfwGetGamepadState(GLFW_JOYSTICK_1, &gamepad))
		{
			// Gamepad not available, so let's fake it with keyboard
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]  = glfwGetKey(window, GLFW_KEY_LEFT);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] = glfwGetKey(window, GLFW_KEY_RIGHT);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_UP]    = glfwGetKey(window, GLFW_KEY_UP);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]  = glfwGetKey(window, GLFW_KEY_DOWN);
			gamepad.buttons[GLFW_GAMEPAD_BUTTON_START]      = glfwGetKey(window, GLFW_KEY_ESCAPE);
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