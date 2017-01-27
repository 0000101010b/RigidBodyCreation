/*
* Program name: Assignment 2 (Real Time Rendering) Transmittance and normal mapping
* Written by: Ben Thompson
* Date:9/11/2016
* 

CONTROLS:

FIRST PERSON CAMERA MOVEMENT FROM HTTP://LEARNOPENGL.COM (SEE CAMERA INTERNAL HEADER FILE) 
W A D S AND MOUSE MOVEMENT

shader,Model and mesh adapted from HTTP://LEARNOPENGL.COM 
*/
#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <algorithm>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// Other Libraries
//Texture Library 
#include <SOIL.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Internal header files
#include "Shader.h"
#include "Camera.h"
#include "Teapot.h"
#include "Model.h"
#include "Particle_Generator.h"
#include "RigidBody_Generator.h"

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void do_movement();
GLuint loadTexture(GLchar* path);

// Window dimensions
const GLuint WIDTH = 800, HEIGHT =600;

// Camera
Camera  camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool    keys[1024];


// Deltatime
GLfloat deltaTime = 0.0f;	// Time between current frame and last frame
GLfloat lastFrame = 0.0f;  	// Time of last frame

//particles
ParticleGenerator *Particles;
RigidBodyGenerator *rigidBody;

// The MAIN function, from here we start the application and run the game loop
int main()
{
	#pragma region Init
	// Init GLFW
	glfwInit();
	// Set all the required options for GLFW
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	// Create a GLFWwindow object that we can use for GLFW's functions
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Particles and collision", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// GLFW Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
	glewExperimental = GL_TRUE;

	// Initialize GLEW to setup the OpenGL Function pointers
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, WIDTH, HEIGHT);

	// OpenGL options
	glEnable(GL_DEPTH_TEST);
#pragma endregion

	#pragma region Load Shaders
	//Build and compile shader programs
	Shader particle("shaders/lab1/particle.vs", "shaders/lab1/particle.frag");
	#pragma endregion

	#pragma region Load Models
	// Load models
	Model ball("models/Ball/sphere1.obj");
	//load particles
	
	Particles = new ParticleGenerator(particle, 1000,ball);
	//load rigidbody
	rigidBody = new RigidBodyGenerator(particle, ball);
	
	//collision plane
	Particles->plane.point=glm::vec3(0.0f, -10.0f, 0.0f);
	Particles->plane.normal=glm::vec3(0.0f, 1.0f, 0.0f);
	
	//Set world physics
	Particles->SetWorldPhysics(glm::vec3(0, -9.8, 0), 0.10f);
	
	//load Plane
	#pragma region collisionPlane
	GLfloat planeVertices[] = {
		// Positions          // Normals         // Texture Coords
		8.0f, -0.5f,  8.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
		-8.0f, -0.5f,  8.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
		-8.0f, -0.5f, -8.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,

		8.0f, -0.5f,  8.0f,  0.0f, 1.0f, 0.0f,  5.0f, 0.0f,
		-8.0f, -0.5f, -8.0f,  0.0f, 1.0f, 0.0f,  0.0f, 5.0f,
		8.0f, -0.5f, -8.0f,  0.0f, 1.0f, 0.0f,  5.0f, 5.0f
	};
	// Setup plane VAO
	GLuint planeVAO, planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), &planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
	glBindVertexArray(0);

	#pragma endregion
	
	//Particles = new ParticleGenerator(particle, );
	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		#pragma region Time and Events
		// Calculate deltatime of current frame
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Check for events
		glfwPollEvents();
		do_movement();
		Particles->camera = camera;
		rigidBody->camera = camera;
		#pragma endregion

		// Clear the colorbuffer
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		glm::mat4 projection = glm::perspective(camera.Zoom, (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		/*
		#pragma region Draw_Plane

		//local view Position
		GLint viewPos_plane = glGetUniformLocation(particle.Program, "viewPos");
		glUniform3f(viewPos_plane, camera.Position.x, camera.Position.y, camera.Position.z);
		// Create camera transformations
		view = camera.GetViewMatrix();
		projection = glm::perspective(camera.Zoom, (GLfloat)800 / (GLfloat)600, 0.1f, 100.0f);
		// Get the uniform matrixes
		GLint modelLoc = glGetUniformLocation(particle.Program, "model");
		GLint viewLoc = glGetUniformLocation(particle.Program, "view");
		GLint projLoc = glGetUniformLocation(particle.Program, "projection");

		// Pass the matrices for view and projection
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glm::mat4 particleModel;
		particleModel = glm::scale(particleModel, glm::vec3(5.0f, 1.0f, 5.0f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(particleModel));

		glm::vec3 offset = glm::vec3(0.0f,-10.0f,0.0f);
		glUniform3f(glGetUniformLocation(particle.Program, "offset"), (GLfloat)offset.x, (GLfloat)offset.y, (GLfloat)offset.z);

		GLfloat scale = 1.0f;
		glUniform1f(glGetUniformLocation(particle.Program, "scale"), scale);

		glm::vec4 color = glm::vec4(.5f,0.2f,0.5f,1.0f);
		glUniform4f(glGetUniformLocation(particle.Program, "color"), (GLfloat)color.r, (GLfloat)color.g, (GLfloat)color.b, (GLfloat)color.a);
		
		glm::vec3 lightPos = glm::vec3(0.0f, 20.0f, 0.0f);
		glUniform3f(glGetUniformLocation(particle.Program, "lightPos"), (GLfloat)lightPos.x, (GLfloat)lightPos.y, (GLfloat)lightPos.z);
		glBindVertexArray(planeVAO);
		//glBindTexture(GL_TEXTURE_2D, floorTexture);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
		#pragma endregion
		*/
		#pragma region Particles
		//Particles->Draw();
		//Particles->Update(deltaTime, glm::vec3(0.0f), 10, glm::vec3(0.0f, 1.0f, 0.0f));
		rigidBody->Update(deltaTime);
		rigidBody->Draw();

#pragma endregion

		// Swap the screen buffers
		glfwSwapBuffers(window);
	}

	// Terminate GLFW, clearing any resources allocated by GLFW.
	glfwTerminate();
	return 0;
}



// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
			keys[key] = false;
	}
}

void do_movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);
}


bool firstMouse = true;
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to left

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}


// This function loads a texture from file. Note: texture loading functions like these are usually 
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio). 
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar* path)
{
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height;
	unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	SOIL_free_image_data(image);
	return textureID;
}
