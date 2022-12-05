/**
 * @file: main.cpp
 * @author: Hongda Lin (lin.3235@osu.com)
 * @brief: 
 * @version: 0.1
 * @date: 2022-11-10
 */

#define STB_IMAGE_IMPLEMENTATION
#include <stdio.h>

#include "Angel.h"
#include "stb_image.h"
typedef Angel::vec3 point3;
typedef Angel::vec3 color3;

#include <string>
#include <fstream>
#include <map>
#include <vector>
#include <GL/glew.h>
#include <GL/glut.h>

struct Edge
{
	point3 startPoint;
	point3 endPoint;
};

struct Memory
{
	Edge edge;
	GLfloat angle;
};

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);
GLuint lsystemShader; /* shader lsystemShader object id */
GLuint lsystemVAO; /* vertex array object id */
GLuint lsystemVBO; /* vertex buffer object id */

GLuint floorVAO; /* vertex array object id for the floor */
GLuint floorVBO; /* vertex buffer object id for floor */

GLuint skyboxShader; /* shader lsystemShader object id */
GLuint skyboxVAO; /* vertex array object id */
GLuint skyboxVBO; /* vertex buffer object id */
unsigned int cubemapTexture;

color3 color{ 1, 1, 1 }; // l-system color (white)
std::ifstream file{};
std::string axiom{}; /* save l-system axiom */
std::string tree{}; /* save l-system string */
std::vector<Edge> edges{}; /* save l-system edges */
std::vector<Memory> memories{}; /* save l-system states */
std::map<char, std::string> grammers{}; /* save l-system rules */
std::vector<point3> l_system_points{}; // holds all the points that construct the tree
std::vector<color3> l_system_colors{}; // holds the color for each line

Edge gl_poped_edge;
bool gl_poped = false;

// Projection transformation parameters
GLfloat fovy = 45.0; // Field-of-view in Y direction angle (in degrees)
GLfloat aspect; // Viewport aspect ratio
GLfloat zNear = 0.5, zFar = 2.0;
GLfloat width{1600}, height{ 800 };

GLfloat angle = 0.0; // rotation angle
GLfloat gl_angle = 0.0; // global rotation angle
GLfloat gl_len = 0.007f; //unit length
int generation{};

float A = 0; // L-system mv rotation
float X = 0; // L system mv translation X
float Z = 0;  // L system mv translation Z
const int floor_NumVertices = 6;       
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices

// Vertices of a unit floor centered at origin, sides aligned with axes
point3 vertices[4] = {
	point3(-0.5, -0.5, 0.5),
	point3(0.5, -0.5, 0.5),
	point3(-0.5, -0.5, -0.5),
	point3(0.5, -0.5, -0.5)
};

// RGBA colors
color3 vertex_colors[4] = {
	color3(0.7, 0.4, 0.1), // brown
	color3(0.7, 0.4, 0.1), // brown
	color3(0.7, 0.4, 0.1), // brown
	color3(0.7, 0.4, 0.1)  // brown
};

void floor()
{
	floor_colors[0] = vertex_colors[1];
	floor_points[0] = vertices[1];
	floor_colors[1] = vertex_colors[0];
	floor_points[1] = vertices[0];
	floor_colors[2] = vertex_colors[2];
	floor_points[2] = vertices[2];

	floor_colors[3] = vertex_colors[2];
	floor_points[3] = vertices[2];
	floor_colors[4] = vertex_colors[3];
	floor_points[4] = vertices[3];
	floor_colors[5] = vertex_colors[1];
	floor_points[5] = vertices[1];
}

const int skybox_NumVertices = 36;
float skyboxVertices[] = {
	// positions          
	-1.0f, 1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, -1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, -1.0f, 1.0f,
	-1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, -1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f, 1.0f,
	1.0f, -1.0f, 1.0f
};

std::vector<std::string> faces
{
	"skybox/right.jpg",
	"skybox/left.jpg",
	"skybox/top.jpg",
	"skybox/bottom.jpg",
	"skybox/front.jpg",
	"skybox/back.jpg"
};


void init();
void display();
void reshape(int w, int h);
void idle();
void keyboard(unsigned char key, int x, int y);
void onMouseClick(int button, int state, int x, int y);

void LSystemRules();
void LSystemString();
void LSystem(); // store all the line points in points

void createEdge();
void rotateLeft();
void rotateRight();
void push();
void pop();


int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Arguments are not provided!" << std::endl;
		return 1;
	}
	/* Read Generation, Angle, File from command line */
	generation = std::stoi(argv[2]);
	angle = std::stof(argv[4]);
	file = std::ifstream{ std::string(argv[5]) };
	if (!file)
	{
		std::cerr << "The rule file is not founded!" << std::endl;
		return 1;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutCreateWindow("Lab3");

	/* Call glewInit() and error checking */
	int err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}
	// Get info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(onMouseClick);

	init();
	glutMainLoop();
	return 0;
}

unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			// note: GL_TEXTURE_CUBE_MAP_POSITIVE_X + i => enum through
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			             0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

void init()
{
	// Load shaders and create a shader lsystemShader (to be used in display())
	lsystemShader = InitShader("vshader.glsl", "fshader.glsl");
	skyboxShader = InitShader("vshader2.glsl", "fshader2.glsl");

	// Initialize the l-system rules
	LSystemRules();
	// Initialize the l-system string
	LSystemString();
	// Initialize the vertex data for the gl_len-system
	LSystem();

	for (int i = 0; i < edges.size(); i++)
	{
		Edge edge = edges[i];
		l_system_points.push_back(edge.startPoint);
		l_system_colors.push_back(color);
		l_system_points.push_back(edge.endPoint);
		l_system_colors.push_back(color);
	}

	// Initialize the vertex data for the floor
	floor();
	// Step 1: Generate and bind the VAO for the floor
	glGenVertexArrays(1, &floorVAO);
	glBindVertexArray(floorVAO);
	// Step 2: Generate the VBO for the floor
	glGenBuffers(1, &floorVBO);
	// Step 3: Bind the VBO with the GL_ARRAY_BUFFER buffer type
	glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
	// Step 4: Copy the vertex data to the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);
	// Step 5: Connect the VBO to the vertex attributes in the shader
	// (get position, interpret the vertex data, and enable the vertex attributes)
	GLuint vPosition = glGetAttribLocation(lsystemShader, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	GLuint vColor = glGetAttribLocation(lsystemShader, "vColor");
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * floor_NumVertices));
	glEnableVertexAttribArray(vColor);
	// (optional) Step 6: unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Step 1: Generate and bind the VAO for the lines
	glGenVertexArrays(1, &lsystemVAO);
	glBindVertexArray(lsystemVAO);
	// Step 2: Generate the VBO for the lines
	glGenBuffers(1, &lsystemVBO);
	// Step 3: Bind the VBO with the GL_ARRAY_BUFFER buffer type
	glBindBuffer(GL_ARRAY_BUFFER, lsystemVBO);
	// Step 4: Copy the vertex data to the VBO
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point3) * l_system_points.size() + sizeof(point3) * l_system_colors.size(),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(point3) * l_system_points.size(), l_system_points.data());
	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * l_system_points.size(),
		sizeof(point3) * l_system_colors.size(),
		l_system_colors.data());
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * l_system_points.size()));
	glEnableVertexAttribArray(vColor);
	// (optional) Step 6: unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Step 1: Generate and bind the VAO for the skybox
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	cubemapTexture = loadCubemap(faces);

	glUseProgram(skyboxShader);
	glUniform1i(glGetUniformLocation(skyboxShader, "skybox"), 0);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);
	glPointSize(3.0);
	glClear(GL_COLOR_BUFFER_BIT);
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4 p = Perspective(fovy, aspect, zNear, zFar);
	/*---  Set up and pass on Model-View matrix to the shader ---*/
	vec4 at(0.0f, 0.0f, 0.0f, 1.0f);
	vec4 up(0.0f, 1.0f, 0.0f, 0.0f);
	vec4 eye(0.0, 0.0, 1.0f, 1.0); // negative Z (for skybox)

	glUseProgram(lsystemShader); 
	GLuint view = glGetUniformLocation(lsystemShader, "view");
	GLuint projection = glGetUniformLocation(lsystemShader, "projection");
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);
	/*----- Set up the Mode-View matrix for the floor -----*/
	mat4 model_view = LookAt(eye, at, up) * Translate(X, 0.0f, 0.0f + Z) * Rotate(0.0f + A, 0.0f, 2.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f); // rotated and translated
	glUniformMatrix4fv(view, 1, GL_TRUE, model_view);

	// draw the floor
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBindVertexArray(floorVAO);
	glDrawArrays(GL_TRIANGLES, 0, floor_NumVertices);

	// draw the l-system
	glBindVertexArray(lsystemVAO);
	glDrawArrays(GL_LINES, 0, l_system_points.size());

	// draw skybox as last 
	glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content
	glUseProgram(skyboxShader); 
	view = glGetUniformLocation(skyboxShader, "view");
	projection = glGetUniformLocation(skyboxShader, "projection");
	glUniformMatrix4fv(projection, 1, GL_TRUE, p);
	model_view = mat4WithUpperLeftMat3(upperLeftMat3(LookAt(eye, at, up) * Rotate(180.0f + A, 0.0f, 2.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f))); // remove translation from the view matrix
	glUniformMatrix4fv(view, 1, GL_TRUE, model_view);
	// bind both textures to the corresponding texture unit
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, skybox_NumVertices);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS); // set depth function back to default

	glutSwapBuffers();
}

void reshape(int w, int h)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}

void idle()
{
	// A += 0.03f;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 'W':
		Z += 0.1f;
		break;

	case 'a':
	case 'A':
		X += 0.1f;
		break;

	case 's':
	case 'S':
		Z -= 0.1f;
		break;

	case 'd':
	case 'D':
		X -= 0.1f;
		break;

	case ' ':
		X = 0.0f;
		Z = 0.0f;
		break;

	case 033: // Escape Key
	case 'q':
	case 'Q':
		exit(EXIT_SUCCESS);


	}
	glutPostRedisplay();
}

void onMouseClick(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		//store the x,y value where the click happened
		A += 11.0f;
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		//store the x,y value where the click happened
		A -= 11.0f;
	}
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		//store the x,y value where the click happened
		A = 0.0f;
	}
}


/**
 * @brief Initialize the axiom and rules for the L-System
 */
void LSystemRules()
{
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			if (line.length() == 1)
			{
				axiom = line;
			}
			else
			{
				char key = line[0];
				std::string value = line.substr(2);
				grammers.insert({ key, value });
			}
		}
		tree = axiom;
		file.close();
	}
}


/**
 * @brief Create L-system string
 */
void LSystemString()
{
	for (int i = 0; i < generation; i++)
	{
		std::string newTree;
		for (auto& symbol : tree)
		{
			if (grammers.count(symbol))
				newTree += grammers[symbol];
			else
				newTree += symbol;
		}
		tree = newTree;
	}
}


/**
 * @brief Create L-system
 */
void LSystem()
{
	for (auto& symbol : tree)
	{
		if (symbol == 'F')
		{
			createEdge();
		}
		else if (symbol == '+')
		{
			rotateLeft();
		}
		else if (symbol == '-')
		{
			rotateRight();
		}
		else if (symbol == '[')
		{
			push();
		}
		else if (symbol == ']')
		{
			pop();
		}
	}
}

void createEdge()
{
	if (edges.empty())
	{
		point3 startPoint{ 0, -0.5f, 0.0 };
		point3 endPoint{ startPoint.x, startPoint.y + gl_len, startPoint.z };
		Edge edge{ startPoint, endPoint };
		edges.push_back(edge);
		return;
	}
	Edge edge;
	if (gl_poped)
	{
		edge = gl_poped_edge;
		gl_poped = false;
	}
	else
	{
		edge = edges.back();
	}
	// get the last edge's end point
	point3 end = edge.endPoint;
	// step 1 : draw a straight line for the end point base on y axis, get the a temp end point
	point3 tempEnd{ end.x, end.y + gl_len, end.z };
	// step 2 : translate the temp end point to matrix 4
	mat4 tempEndMat4{ tempEnd.x, tempEnd.y, tempEnd, 1 };
	// calculate the translate matrix for end point to origin, since the end point will be the start point of new edge
	mat4 transMatToOrigin{ Translate(-end) };
	// calculate the rotation matrix for angle base on Z axis (RotateZ)
	mat4 rotMat{ Rotate(gl_angle, 0, 0, 1) };
	// calculate the scaling matrix
	// mat4 scalMat{Scale(gl_scale, gl_scale, 0)};
	// calculate the translate matrix for moving the point back
	mat4 transMatToStartPoint{ Translate(end) };
	// calculate the end point at origin scaled and rotated
	mat4 endPointMatAtOrigin = rotMat * transMatToOrigin * tempEndMat4;
	// calculate the end point translated back
	mat4 newEndMat = transMatToStartPoint * endPointMatAtOrigin;
	// get the new end point
	point3 newEndPoint{ newEndMat[0].x, newEndMat[1].y, 0.0 };
	// the old end point will be used for the new start point
	Edge newEdge = { end, newEndPoint };

	edges.push_back(newEdge);
}

void rotateLeft()
{
	gl_angle += angle;
}

void rotateRight()
{
	gl_angle -= angle;
}

void push()
{
	Memory memory = { edges.back(), gl_angle };
	memories.push_back(memory);
}

void pop()
{
	Memory memory = memories.back();
	gl_poped_edge = memory.edge;
	gl_angle = memory.angle;
	memories.pop_back();
	gl_poped = true;
}
