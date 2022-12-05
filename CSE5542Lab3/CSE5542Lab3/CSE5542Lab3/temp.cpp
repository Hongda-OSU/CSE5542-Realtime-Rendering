/**
 * @file: main.cpp
 * @author: Hongda Lin (lin.3235@osu.com)
 * @brief: L-system
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
GLuint program; /* shader program object id */
GLuint vao_line; /* vertex array object id */
GLuint vbo_line; /* vertex buffer object id */
GLuint vao_floor; /* vertex array object id for the floor */
GLuint vbo_floor; /* vertex buffer object id for floor */
GLuint vao_skybox; /* vertex array object id */
GLuint vbo_skybox; /* vertex buffer object id */
GLuint textureId; /* texture object id */
unsigned int cubemapTexture;

color3 color{ 1, 1, 1 }; // l-system color
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
GLfloat width{ 512 }, height{ 512 };

GLfloat angle = 0.0; // rotation angle
GLfloat gl_angle = 0.0; // global rotation angle
GLfloat gl_len = 0.007f; //unit length
int generation{};

vec4 init_eye(0.0, 0.0, 1.0, 1.0); // initial viewer position (z control the far to see the tree)
vec4 eye = init_eye; // current viewer position

int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by key 'a' or 'A'
int floorFlag = 1; // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
const int floor_NumVertices = 6;        //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point3 floor_points[floor_NumVertices]; // positions for all vertices
color3 floor_colors[floor_NumVertices]; // colors for all vertices

// Vertices of a unit floor centered at origin, sides aligned with axes
point3 vertices[4] = {
	point3(-0.5, -0.5, 0.5),
	point3(0.5, -0.5, 0.5),
	point3(-0.5, -0.5, -0.5),
	point3(0.5, -0.5, -0.5) };
// RGBA colors
color3 vertex_colors[4] = {
	color3(0.7, 0.4, 0.1), // brown
	color3(0.7, 0.4, 0.1), // brown
	color3(0.7, 0.4, 0.1), // brown
	color3(0.7, 0.4, 0.1)  // brown
};

const int skybox_NumVertices = 36;
point3 skyboxVertices[36] = {
	// positions
	point3(-1.0f, 1.0f, -1.0f),
	point3(-1.0f, -1.0f, -1.0f),
	point3(1.0f, -1.0f, -1.0f),
	point3(1.0f, -1.0f, -1.0f),
	point3(1.0f, 1.0f, -1.0f),
	point3(-1.0f, 1.0f, -1.0f),

	point3(-1.0f, -1.0f, 1.0f),
	point3(-1.0f, -1.0f, -1.0f),
	point3(-1.0f, 1.0f, -1.0f),
	point3(-1.0f, 1.0f, -1.0f),
	point3(-1.0f, 1.0f, 1.0f),
	point3(-1.0f, -1.0f, 1.0f),

	point3(1.0f, -1.0f, -1.0f),
	point3(1.0f, -1.0f, 1.0f),
	point3(1.0f, 1.0f, 1.0f),
	point3(1.0f, 1.0f, 1.0f),
	point3(1.0f, 1.0f, -1.0f),
	point3(1.0f, -1.0f, -1.0f),

	point3(-1.0f, -1.0f, 1.0f),
	point3(-1.0f, 1.0f, 1.0f),
	point3(1.0f, 1.0f, 1.0f),
	point3(1.0f, 1.0f, 1.0f),
	point3(1.0f, -1.0f, 1.0f),
	point3(-1.0f, -1.0f, 1.0f),

	point3(-1.0f, 1.0f, -1.0f),
	point3(1.0f, 1.0f, -1.0f),
	point3(1.0f, 1.0f, 1.0f),
	point3(1.0f, 1.0f, 1.0f),
	point3(-1.0f, 1.0f, 1.0f),
	point3(-1.0f, 1.0f, -1.0f),

	point3(-1.0f, -1.0f, -1.0f),
	point3(-1.0f, -1.0f, 1.0f),
	point3(1.0f, -1.0f, -1.0f),
	point3(1.0f, -1.0f, -1.0f),
	point3(-1.0f, -1.0f, 1.0f),
	point3(1.0f, -1.0f, 1.0f)
};

vec3 skyboxTextCoord[36] = {
	// positions
	vec3(-1.0f, 1.0f, -1.0f),
	vec3(-1.0f, -1.0f, -1.0f),
	vec3(1.0f, -1.0f, -1.0f),
	vec3(1.0f, -1.0f, -1.0f),
	vec3(1.0f, 1.0f, -1.0f),
	vec3(-1.0f, 1.0f, -1.0f),

	vec3(-1.0f, -1.0f, 1.0f),
	vec3(-1.0f, -1.0f, -1.0f),
	vec3(-1.0f, 1.0f, -1.0f),
	vec3(-1.0f, 1.0f, -1.0f),
	vec3(-1.0f, 1.0f, 1.0f),
	vec3(-1.0f, -1.0f, 1.0f),

	vec3(1.0f, -1.0f, -1.0f),
	vec3(1.0f, -1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, -1.0f),
	vec3(1.0f, -1.0f, -1.0f),

	vec3(-1.0f, -1.0f, 1.0f),
	vec3(-1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, -1.0f, 1.0f),
	vec3(-1.0f, -1.0f, 1.0f),

	vec3(-1.0f, 1.0f, -1.0f),
	vec3(1.0f, 1.0f, -1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(1.0f, 1.0f, 1.0f),
	vec3(-1.0f, 1.0f, 1.0f),
	vec3(-1.0f, 1.0f, -1.0f),

	vec3(-1.0f, -1.0f, -1.0f),
	vec3(-1.0f, -1.0f, 1.0f),
	vec3(1.0f, -1.0f, -1.0f),
	vec3(1.0f, -1.0f, -1.0f),
	vec3(-1.0f, -1.0f, 1.0f),
	vec3(1.0f, -1.0f, 1.0f)
};


//-------------------------------
// generate 2 triangles: 6 vertices and 6 colors
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
	//glutMouseFunc(mouse);

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
	//stbi_set_flip_vertically_on_load(true);
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
	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader.glsl", "fshader.glsl");
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
	glGenVertexArrays(1, &vao_floor);
	glBindVertexArray(vao_floor);
	// Step 2: Generate the VBO for the floor
	glGenBuffers(1, &vbo_floor);
	// Step 3: Bind the VBO with the GL_ARRAY_BUFFER buffer type
	glBindBuffer(GL_ARRAY_BUFFER, vbo_floor);
	// Step 4: Copy the vertex data to the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors), floor_colors);
	// Step 5: Connect the VBO to the vertex attributes in the shader 
	// (get position, interpret the vertex data, and enable the vertex attributes)
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * floor_NumVertices));
	glEnableVertexAttribArray(vColor);
	// (optional) Step 6: unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

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
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vColor, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * l_system_points.size()));
	glEnableVertexAttribArray(vColor);
	// (optional) Step 6: unbind VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Step 1: Generate and bind the VAO for the skybox
	glGenVertexArrays(1, &vao_skybox);
	glBindVertexArray(vao_skybox);
	// Step 2: Generate the VBO for the skybox
	glGenBuffers(1, &vbo_skybox);
	// Step 3: Bind the VBO with the GL_ARRAY_BUFFER buffer type
	glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox);
	// Step 4: Copy the vertex data to the VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices) + sizeof(skyboxVertices),
		NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(skyboxVertices), skyboxVertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), sizeof(skyboxVertices), skyboxVertices);
	// Step 5: Connect the VBO to the vertex attributes in the shader 
   // (get position, interpret the vertex data, and enable the vertex attributes)
	glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(vPosition);
	GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(skyboxVertices)));
	glEnableVertexAttribArray(vTexCoord);

	// binding to GL_TEXTURE_CUBE_MAP
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
	cubemapTexture = loadCubemap(faces);
	glUseProgram(program);
	glUniform1i(glGetUniformLocation(program, "textureId"), 0);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glLineWidth(1.0);
	glPointSize(3.0);
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

/**
 * @brief draw the object that is associated with the vertex array object
 * and has "num_vertices" vertices. The primitive type is specified by type
 * @param buffer
 * @param num_vertices
 * @param type : could be 'TRIANGLE', 'LINE' (can be extended)
 */
void drawObj(GLuint buffer, int num_vertices, std::string type)
{
	// Bind the VAO we initialized before 
	glBindVertexArray(buffer);

	///Draw a sequence of geometric objs (triangles) based on the VAO and their primitive type
	if (type == "TRIANGLES")
	{
		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	}
	else if (type == "TRIANGLE_STRIP")
	{
		glDrawArrays(GL_TRIANGLE_STRIP, 0, num_vertices);
	}
	else if (type == "TRIANGLE_FAN")
	{
		glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);
	}
	else if (type == "LINES")
	{
		glDrawArrays(GL_LINES, 0, num_vertices);
	}
	else if (type == "LINE_LOOP")
	{
		glDrawArrays(GL_LINE_LOOP, 0, num_vertices);
	}
	else if (type == "LINE_STRIP")
	{
		glDrawArrays(GL_LINE_STRIP, 0, num_vertices);
	}
	else if (type == "POINTS")
	{
		glDrawArrays(GL_POINTS, 0, num_vertices);
	}

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

	// Draw the floor
	if (floorFlag == 1)                             // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	else // Wireframe floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj(vao_floor, floor_NumVertices, "TRIANGLES"); // draw the floor

	// draw the l-system
	drawLSystem(vao_line, l_system_points.size());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	drawObj(vao_skybox, 36, "Triangle");

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


