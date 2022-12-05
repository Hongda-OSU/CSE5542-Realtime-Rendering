#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <vector>
#include <xlocmon>

#include "Angel.h"
#include <GL/glew.h>
#include <GL/glut.h>

typedef Angel::vec3 point3;
typedef Angel::vec3 color3;

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
GLuint program; /* shader program object id */
GLuint vao_line; /* vertex array object id */
GLuint vbo_line; /* vertex buffer object id */

// Projection transformation parameters
GLfloat fovy = 45.0; // Field-of-view in Y direction angle (in degrees)
GLfloat aspect; // Viewport aspect ratio
GLfloat zNear = 0.5, zFar = 2.0;

GLfloat gl_angle = 0.0; // global an angle for rotate a line (direction up)
GLfloat angle = 0.0; // rotation angle in degrees

vec4 init_eye(0.0, 0.0, 1.0, 1.0); // initial viewer position (z control the far to see the tree)
vec4 eye = init_eye; // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int floorFlag = 1; // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'

color3 color{1, 1, 1}; // color white

int generation{};
GLfloat width{ 512 }, height{512};
GLfloat gl_len{}, gl_scale{1}, scale_f{0.7};

Edge gl_poped_edge;
bool gl_poped = false;

std::string fileName{};
std::string axiom{};
std::string tree{};
std::vector<std::string> trees{};
std::vector<Edge> edges{};
std::vector<Memory> memories{};
std::map<char, std::string> grammers{}; // map the l system grammars

int Index = 0; // keep track all the points
std::vector<point3> l_system_points{}; // holds all the points that construct the tree
std::vector<color3> l_system_colors{}; // holds the color for each line



void init();
void display();
void reshape(int w, int h);
void idle();
void keyboard(unsigned char key, int x, int y);

void initGrammars();
void initLSystem(); // generate the gl_len-system string
void generateLSystem(); // store all the line points in points

void createEdge();
void rotateLeft();
void rotateRight();
void push();
void pop();

int main(int argc, char** argv)
{
	if (argc > 1)
	{
		generation = std::stoi(argv[2]);
		angle = std::stof(argv[4]);
		fileName = std::string(argv[5]);
	}
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(1200, 200);
	glutCreateWindow("Lab2 L-System");

	// initialize glew
	glewExperimental = GL_TRUE;
	GLenum glewInitResult = glewInit();
	if (GLEW_OK != glewInitResult)
	{
		std::cerr << "Error initializing glew." << std::endl;
		return 1;
	}

	init();
	// All the callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	//glutMouseFunc(mouse);
	glutMainLoop();
	return 0;
}

void mapLSystem()
{
	for (int i = 0; i < edges.size(); i++)
	{
		Edge edge = edges[i];
		l_system_points.push_back(edge.startPoint);
		l_system_colors.push_back(color);
		Index++;
		l_system_points.push_back(edge.endPoint);
		l_system_colors.push_back(color);
		Index++;
	}
}

void init()
{
	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader.glsl", "fshader.glsl");

	// read file for variable
	initGrammars();
	// Initialize the gl_len-system string
	initLSystem();
	// Initialize the vertex data for the gl_len-system
	generateLSystem();
	mapLSystem();

	// Step 1: Generate and bind the VAO for the lines
	glGenVertexArrays(1, &vao_line);
	glBindVertexArray(vao_line);
	// Step 2: Generate the VBO for the lines
	glGenBuffers(1, &vbo_line);
	// Step 3: Bind the VBO with the GL_ARRAY_BUFFER buffer type
	glBindBuffer(GL_ARRAY_BUFFER, vbo_line);
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
	// Step 5: Connect the VBO to the vertex attributes in the shader 
	// (get position, interpret the vertex data, and enable the vertex attributes)
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * l_system_points.size()));
	glEnableVertexAttribArray(vColor);
	// (optional) Step 6: unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(2.0);
}

/**
 * @brief draw the object that is associated with the vertex array object
 * and has "num_vertices" vertices. The primitive type is specified by type
 * @param buffer
 * @param num_vertices
 */
void drawLSystem(GLuint buffer, int num_vertices)
{
	// Bind the VAO we initialized before 
	glBindVertexArray(buffer);
	/* Draw a sequence of lines from the vertex buffer
	   (using the attributes specified in each enabled vertex attribute array) */
	glDrawArrays(GL_LINES, 0, num_vertices);
}

void display()
{
	GLuint model_view; // model-view matrix uniform shader variable location
	GLuint projection; // projection matrix uniform shader variable location

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(program); // Use the shader program

	model_view = glGetUniformLocation(program, "model_view");
	projection = glGetUniformLocation(program, "projection");

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4 p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

	/*---  Set up and pass on Model-View matrix to the shader ---*/
	// eye is a global variable of vec4 set to init_eye and updated by keyboard()
	vec4 at(0.0, 0.0, 0.0, 1.0);
	vec4 up(0.0, 1.0, 0.0, 0.0);

	/*----- Set up the Mode-View matrix for the floor -----*/
	// The set-up below gives a new scene (scene 2), using Correct LookAt() function
	mat4 mv = LookAt(eye, at, up) * Translate(0.0f, 0.0f, 0.0f) * Scale(1.0f, 1.0f, 1.0f);

	glUniformMatrix4fv(model_view, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major

	// draw the gl_len-system
	drawLSystem(vao_line, l_system_points.size());

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
	angle += 0.3f;
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 033: // Escape Key
	case 'q':
	case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'X':
		eye[0] += 0.1;
		break;
	case 'x':
		eye[0] -= 0.1;
		break;
	case 'Y':
		eye[1] += 0.1;
		break;
	case 'y':
		eye[1] -= 0.1;
		break;
	case 'Z':
		eye[2] += 0.1;
		break;
	case 'z':
		eye[2] -= 0.1;
		break;

	case 'a':
	case 'A': // Toggle between animation and non-animation
		animationFlag = 1 - animationFlag;
		if (animationFlag == 1)
			glutIdleFunc(idle);
		else
			glutIdleFunc(NULL);
		break;
	case 'f':
	case 'F': // Toggle between filled and wireframe floor
		floorFlag = 1 - floorFlag;
		break;

	case ' ': // reset to initial viewer/eye position
		eye = init_eye;
		break;
	}
	glutPostRedisplay();
}

void initGrammars()
{
	std::ifstream file(fileName);
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
				grammers.insert({key, value});
			}
		}
		tree = axiom;
		trees.push_back(axiom);
		file.close();
	}
}

void initLSystem()
{
	for (int i = 0; i < generation; i++)
	{
		gl_scale *= scale_f;
		std::string newTree;
		for (auto& symbol : tree)
		{
			if (grammers.count(symbol))
				newTree += grammers[symbol];
			else
				newTree += symbol;
		}
		tree = newTree;
		trees.push_back(newTree);
	}
	if (generation >= 6)
	{
		gl_len = 0.08f;
	}
	else if (generation >= 4 && generation < 6)
	{
		gl_len = 0.12f;
	}
	else if (generation < 4)
	{
		gl_len = 0.25f;
	}
}

void generateLSystem()
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
		point3 startPoint{0, -0.8f, 0.0};
		point3 endPoint{startPoint.x, startPoint.y + gl_len * gl_scale, startPoint.z};
		Edge edge{startPoint, endPoint};
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
	point3 tempEnd{end.x, end.y + gl_len, end.z};
	// step 2 : translate the temp end point to matrix 4
	mat4 tempEndMat4{tempEnd.x, tempEnd.y, tempEnd, 1};
	// calculate the translate matrix for end point to origin, since the end point will be the start point of new edge
	mat4 transMatToOrigin{Translate(-end)};
	// calculate the rotation matrix for angle base on Z axis
	mat4 rotMat{Rotate(gl_angle, 0, 0, 1)};
	// calculate the scaling matrix
	mat4 scalMat{Scale(gl_scale, gl_scale, 0)};
	// calculate the translate matrix for moving the point back
	mat4 transMatToStartPoint{Translate(end)};
	// calculate the end point at origin scaled and rotated 
	mat4 endPointMatAtOrigin = rotMat * scalMat * transMatToOrigin * tempEndMat4;
	// calculate the end point translated back
	mat4 newMatEnd = transMatToStartPoint * endPointMatAtOrigin;
	// get the new end point
	point3 newEndPoint{ newMatEnd[0].x, newMatEnd[1].y, 0.0 };
	// the old end point will be used for the new start point
	Edge newEdge = {end, newEndPoint};

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
	Memory memory = {edges.back(), gl_angle};
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
