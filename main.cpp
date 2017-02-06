// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include "glm/glm.hpp"

// specify that we want the OpenGL core profile before including GLFW headers
#define GLFW_INCLUDE_GLCOREARB
#define GL_GLEXT_PROTOTYPES
#include <GLFW/glfw3.h>

using namespace std;
using namespace glm;

//Forward definitions
bool CheckGLErrors();
void QueryGLVersion();
string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);

GLint drawMode;

int state = 1;
int levels = 1; 	//Squares & Diamonds	State = 1
int revs = 2;		//Spirals				State = 2
int iterations = 0;	//Sierpinski Triangles	State = 3
int fernDepth = 1;	//Barnsley Fern			State = 4
int snowFlakes = 0;	//Koch Snowflake		State = 5

//==========================================================================
// TUTORIAL STUFF


//vec2 and vec3 are part of the glm math library. 
//Include in your own project by putting the glm directory in your project, 
//and including glm/glm.hpp as I have at the top of the file.
//"using namespace glm;" will allow you to avoid writing everyting as glm::vec2
vector<vec2> points;
vector<vec3> colors;

//Structs are simply acting as namespaces
//Access the values like so: VAO::LINES
struct VAO{
	enum {LINES=0, COUNT};		//Enumeration assigns each name a value going up
										//LINES=0, COUNT=1
};

struct VBO{
	enum {POINTS=0, COLOR, COUNT};	//POINTS=0, COLOR=1, COUNT=2
};

struct SHADER{
	enum {LINE=0, COUNT};		//LINE=0, COUNT=1
};

GLuint vbo [VBO::COUNT];		//Array which stores OpenGL's vertex buffer object handles
GLuint vao [VAO::COUNT];		//Array which stores Vertex Array Object handles
GLuint shader [SHADER::COUNT];		//Array which stores shader program handles


//Gets handles from OpenGL
void generateIDs()
{
	glGenVertexArrays(VAO::COUNT, vao);		//Tells OpenGL to create VAO::COUNT many
														// Vertex Array Objects, and store their
														// handles in vao array
	glGenBuffers(VBO::COUNT, vbo);		//Tells OpenGL to create VBO::COUNT many
													//Vertex Buffer Objects and store their
													//handles in vbo array
}

//Clean up IDs when you're done using them
void deleteIDs()
{
	for(int i=0; i<SHADER::COUNT; i++)
	{
		glDeleteProgram(shader[i]);
	}
	
	glDeleteVertexArrays(VAO::COUNT, vao);
	glDeleteBuffers(VBO::COUNT, vbo);	
}


//Describe the setup of the Vertex Array Object
bool initVAO()
{
	glBindVertexArray(vao[VAO::LINES]);		//Set the active Vertex Array

	glEnableVertexAttribArray(0);		//Tell opengl you're using layout attribute 0 (For shader input)
	glBindBuffer( GL_ARRAY_BUFFER, vbo[VBO::POINTS] );		//Set the active Vertex Buffer
	glVertexAttribPointer(
		0,				//Attribute
		2,				//Size # Components
		GL_FLOAT,	//Type
		GL_FALSE, 	//Normalized?
		sizeof(vec2),	//Stride
		(void*)0			//Offset
		);
	
	glEnableVertexAttribArray(1);		//Tell opengl you're using layout attribute 1
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::COLOR]);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(vec3),
		(void*)0
		);	

	return !CheckGLErrors();		//Check for errors in initialize
}


//Loads buffers with data
bool loadBuffer(const vector<vec2>& points, const vector<vec3>& colors)
{
	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::POINTS]);
	glBufferData(
		GL_ARRAY_BUFFER,				//Which buffer you're loading too
		sizeof(vec2)*points.size(),	//Size of data in array (in bytes)
		&points[0],							//Start of array (&points[0] will give you pointer to start of vector)
		GL_STATIC_DRAW						//GL_DYNAMIC_DRAW if you're changing the data often
												//GL_STATIC_DRAW if you're changing seldomly
		);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO::COLOR]);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vec3)*colors.size(),
		&colors[0],
		GL_STATIC_DRAW
		);

	return !CheckGLErrors();	
}

//Compile and link shaders, storing the program ID in shader array
bool initShader()
{	
	string vertexSource = LoadSource("vertex.glsl");		//Put vertex file text into string
	string fragmentSource = LoadSource("fragment.glsl");		//Put fragment file text into string

	GLuint vertexID = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragmentID = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);
	
	shader[SHADER::LINE] = LinkProgram(vertexID, fragmentID);	//Link and store program ID in shader array

	return !CheckGLErrors();
}

void drawKochTriangles(vec2 start, vec2 end, int snowFlakes) {
	if (snowFlakes >= 1) {
		vec2 vector1 = (end - start) / ((float)3);
		
		float theta = (M_PI / ((float)3));
		float left_x = (vector1[0] * cos(theta)) - (vector1[1] * sin(theta));
		float left_y = (vector1[0] * sin(theta)) + (vector1[1] * cos(theta));
		vec2 vector2(left_x,left_y);
	
		vec2 new_left = start + vector1;
		vec2 new_top = new_left + vector2;
		vec2 new_right = end - vector1;
	
		points.push_back(new_left);
		points.push_back(new_right);
		points.push_back(new_top);
	
		colors.push_back(vec3(1.f,1.f,1.f));
		colors.push_back(vec3(1.f,1.f,1.f));
		colors.push_back(vec3(1.f,1.f,1.f));
	
		snowFlakes--;
	
		drawKochTriangles(start, new_left, snowFlakes);
		drawKochTriangles(new_left, new_top, snowFlakes);
		drawKochTriangles(new_top, new_right, snowFlakes);
		drawKochTriangles(new_right, end, snowFlakes);
	}
}

void drawKoch(vec2 left, vec2 right, vec2 top, int snowFlakes){

	points.push_back(left);
	points.push_back(right);
	points.push_back(top);

	colors.push_back(vec3(1.f,1.f,1.f));
	colors.push_back(vec3(1.f,1.f,1.f));
	colors.push_back(vec3(1.f,1.f,1.f));
	
	drawKochTriangles(right, left, snowFlakes);
	drawKochTriangles(left, top, snowFlakes);
	drawKochTriangles(top, right, snowFlakes);
}

void generateKoch(int snowFlakes){
	points.clear();
	colors.clear();
	
	drawMode = GL_TRIANGLES;
	
	vec2 left_koch(-0.4f,-0.4f);
	vec2 right_koch(0.4f,-0.4f);
	vec2 top_koch(
		0.f, (0.8f * sin(M_PI/((float)3))) / ((float)2)
	);
	
	drawKoch(left_koch, right_koch, top_koch, snowFlakes);
}

void generateFern(int fernDepth){
	points.clear();
	colors.clear();
	
	drawMode = GL_POINTS;
	float x = 0.f;
	float y = 0.f;
	float temp;
	int probability;
	for(int i = 0; i < (fernDepth * 50000); i++) {
		points.push_back(vec2((x * 0.17f - 0.2f),(y * 0.17f - 0.8f)));
		colors.push_back(vec3((0.8f * (y * 0.17f)), 1.0f , 0.0f));

		probability = (rand() % 100);
		temp = x;
		
		if (probability <= 1) {
			x = 0.f;
			y = 0.16f * y;
		}
		else if (probability <= 8) {
			x = (0.2f * x) - (0.26f * y);
			y = (0.23f * temp) + (0.22f * y) + 1.6f;
		}
		else if (probability <= 15) {
			x = (-0.15f * x) + (0.28f * y);
			y = (0.26f * temp) + (0.24f * y) + 0.44f;
		}
		else {
			x = (0.85f * x) + (0.04f * y);
			y = (-0.04f * temp) + (0.85f * y) + 1.6f;
		}
	}
}

vec2 vector_midpoint(vec2 vector1, vec2 vector2){
	vec2 two(2.f,2.f);
	vec2 finalVector = (vector1 + vector2) / two;
	return finalVector;
}


//generate sierpinski triangle
void drawTriangle(vec2 left, vec2 right, vec2 top, int iterations){
	vec3 color1(1.f,0.f,0.f);
	vec3 color2(1.f,0.5f,0.f);
	vec3 color3(0.f,0.3f,0.7f);
	vec3 color4(0.f,1.f,0.f);

	if(iterations <= 0) {
		points.push_back(left);
		points.push_back(right);
		points.push_back(top);

		if (left[0] < 0.f)
			colors.push_back(color1);
		else 
			colors.push_back(color3);
		if (top[1] < -0.06f)
			colors.push_back(color2);
		else
			colors.push_back(color4);
		colors.push_back(vec3(1.f,1.f,1.f));
	}
	else {
		vec2 left_mid = vector_midpoint(left,top);
		vec2 right_mid = vector_midpoint(right,top);
		vec2 bottom_mid = vector_midpoint(left, right);
		
		iterations--;
		
		drawTriangle(left,bottom_mid,left_mid,iterations);
		drawTriangle(bottom_mid,right,right_mid,iterations);
		drawTriangle(left_mid,right_mid,top,iterations);
	}
}

void generateSierpinski(int iterations){
	points.clear();
	colors.clear();
	
	drawMode = GL_TRIANGLES;
	
	vec2 left_corner(-0.9f,-0.9f);
	vec2 right_corner(0.9f,-0.9f);
	vec2 top_corner(
		0.f, (1.8f * sin(M_PI/((float)3))) / ((float)2)
	);
	
	drawTriangle(left_corner, right_corner, top_corner, iterations);
}

//generate squares/diamonds
void squaresDiamonds(int levels, float length, float bright){
	vec3 color1(0.f, 1.f, 0.f);
	vec3 color2(1.f, 0.f, 1.f);
	
	float next_length = length / ((float)2);

	points.push_back(vec2(-1*length,-1*length));
	points.push_back(vec2(-1*length,length));
	points.push_back(vec2(length,length));
	points.push_back(vec2(length,-1*length));
	points.push_back(vec2(-1*length,-1*length));
	
	points.push_back(vec2(0,-1*length));
	
	for(int i = 0; i < 6; i++){
		colors.push_back(vec3(color1 * bright));
	}
	
	points.push_back(vec2(0, -1*length));
	points.push_back(vec2(-1*length, 0));
	points.push_back(vec2(0,length));
	points.push_back(vec2(length,0));
	points.push_back(vec2(0, -1*length));
	
	points.push_back(vec2(
		-1 * next_length,
		-1 * next_length)
	);
	
	for(int i = 0; i < 6; i++){
		colors.push_back(vec3(color2 * bright));
	}
	
	if (levels>1){
		squaresDiamonds((levels-1),next_length, (0.7f * bright));
	}
}

void generateSquares(int levels)
{
	points.clear();
	colors.clear();
	
	drawMode = GL_LINE_STRIP;
	
	squaresDiamonds(levels, 0.9f, 1.f);
}

// generateSpiral
void generateSpiral(int revolutions)
{
	points.clear();
	colors.clear();
	
	drawMode = GL_LINE_STRIP;
	
	float max_rotation = ((float)(2*revolutions))*M_PI;
	
	float u = 0.f;
	float ustep = 1.f/(max_rotation);
	
	vec3 startColor(1.f, 0.f, 0.f);
	vec3 endColor(0.f, 1.f, 1.f);
	
	while(u<max_rotation)
	{
		u += ustep;
		points.push_back(vec2(
			ustep*u*cos(u),
			ustep*u*sin(u))
		);
		
		colors.push_back(mix(startColor,endColor,u/max_rotation));
	}
}

void increment(){
	switch(state) {
		case 1 :
			levels = std::min(levels+1, 7);
			generateSquares(levels);
			loadBuffer(points, colors);
			break;
		case 2 :
			revs++;
			generateSpiral(revs);
			loadBuffer(points, colors);
			break;
		case 3 :
			iterations = std::min(iterations+1, 6);
			generateSierpinski(iterations);
			loadBuffer(points, colors);
			break;
		case 4 :
			fernDepth = std::min(fernDepth+1, 10);
			generateFern(fernDepth);
			loadBuffer(points, colors);
			break;
		case 5 :
			snowFlakes = std::min(snowFlakes+1, 10);
			generateKoch(snowFlakes);
			loadBuffer(points, colors);
			break;
	}
}

void decrement(){
	switch(state) {
		case 1 :
			levels = std::max(levels-1, 1);
			generateSquares(levels);
			loadBuffer(points, colors);
			break;
		case 2 :
			revs = std::max(revs-1, 1);
			generateSpiral(revs);
			loadBuffer(points, colors);
			break;
		case 3 :
			iterations = std::max(iterations-1, 0);
			generateSierpinski(iterations);
			loadBuffer(points, colors);
			break;
		case 4 :
			fernDepth = std::max(fernDepth-1, 1);
			generateFern(fernDepth);
			loadBuffer(points, colors);
			break;
		case 5 :
			snowFlakes = std::max(snowFlakes-1, 0);
			generateKoch(snowFlakes);
			loadBuffer(points, colors);
			break;
	}
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
    cout << "GLFW ERROR " << error << ":" << endl;
    cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(action == GLFW_PRESS){
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_1) {
			state = 1;
			generateSquares(levels);
			loadBuffer(points, colors);
		}
		else if (key == GLFW_KEY_2) {
			state = 2;
			generateSpiral(revs);
			loadBuffer(points, colors);
		}
		else if (key == GLFW_KEY_3) {
			state = 3;
			generateSierpinski(iterations);
			loadBuffer(points, colors);
		}
		else if (key == GLFW_KEY_4) {
			state = 4;
			generateFern(fernDepth);
			loadBuffer(points, colors);
		}
		else if (key == GLFW_KEY_5) {
			state = 5;
			generateKoch(snowFlakes);
			loadBuffer(points, colors);
		}
		else if (key == GLFW_KEY_UP)
			increment();
		else if (key == GLFW_KEY_DOWN)
			decrement();
	}
}

//Initialization
void initGL()
{
	//Only call these once - don't call again every time you change geometry
	generateIDs();		//Create VertexArrayObjects and Vertex Buffer Objects and store their handles
	initShader();		//Create shader and store program ID

	initVAO();			//Describe setup of Vertex Array Objects and Vertex Buffer Objects

	//Call these two (or equivalents) every time you change geometry
	generateSquares(1);		//Create geometry - CHANGE THIS FOR DIFFERENT SCENES
	loadBuffer(points, colors);	//Load geometry into buffers
}

//Draws buffers to screen
void render()
{
	glClearColor(0.f, 0.f, 0.f, 0.f);		//Color to clear the screen with (R, G, B, Alpha)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//Clear color and depth buffers (Haven't covered yet)

	//Don't need to call these on every draw, so long as they don't change
	glUseProgram(shader[SHADER::LINE]);		//Use LINE program
	glBindVertexArray(vao[VAO::LINES]);		//Use the LINES vertex array

	glDrawArrays(
			drawMode,		//What shape we're drawing	- GL_TRIANGLES, GL_LINES, GL_POINTS, GL_QUADS, GL_TRIANGLE_STRIP
			0,					//Starting index
			points.size()		//How many vertices
		);
}


// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{   
    // initialize the GLFW windowing system
    if (!glfwInit()) {
        cout << "ERROR: GLFW failed to initilize, TERMINATING" << endl;
        return -1;
    }
    glfwSetErrorCallback(ErrorCallback);

    // attempt to create a window with an OpenGL 4.1 core profile context
    GLFWwindow *window = 0;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(512, 512, "CPSC 453 OpenGL Boilerplate", 0, 0);
    if (!window) {
        cout << "Program failed to create GLFW window, TERMINATING" << endl;
        glfwTerminate();
        return -1;
    }

    // set keyboard callback function and make our context current (active)
    glfwSetKeyCallback(window, KeyCallback);
    glfwMakeContextCurrent(window);

    // query and print out information about our OpenGL environment
    QueryGLVersion();

	initGL();
	
	cout << "\n1: Squares and Diamonds\n2: Spiral\n3: Sierpinski Triangles" << endl;
	cout << "\nUp: Increment iterations\nDown: Decrement iterations" << endl;

    // run an event-triggered main loop
    while (!glfwWindowShouldClose(window))
    {
        // call function to draw our scene
        render();

        // scene is rendered to the back buffer, so swap to front for display
        glfwSwapBuffers(window);

        // sleep until next event before drawing again
        glfwWaitEvents();
	}

	// clean up allocated resources before exit
   deleteIDs();
	glfwDestroyWindow(window);
   glfwTerminate();

   return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
    // query opengl version and renderer information
    string version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));
    string glslver  = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
    string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

    cout << "OpenGL [ " << version << " ] "
         << "with GLSL [ " << glslver << " ] "
         << "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
    bool error = false;
    for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
    {
        cout << "OpenGL ERROR:  ";
        switch (flag) {
        case GL_INVALID_ENUM:
            cout << "GL_INVALID_ENUM" << endl; break;
        case GL_INVALID_VALUE:
            cout << "GL_INVALID_VALUE" << endl; break;
        case GL_INVALID_OPERATION:
            cout << "GL_INVALID_OPERATION" << endl; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
        case GL_OUT_OF_MEMORY:
            cout << "GL_OUT_OF_MEMORY" << endl; break;
        default:
            cout << "[unknown error code]" << endl;
        }
        error = true;
    }
    return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
    string source;

    ifstream input(filename.c_str());
    if (input) {
        copy(istreambuf_iterator<char>(input),
             istreambuf_iterator<char>(),
             back_inserter(source));
        input.close();
    }
    else {
        cout << "ERROR: Could not load shader source from file "
             << filename << endl;
    }

    return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
    // allocate shader object name
    GLuint shaderObject = glCreateShader(shaderType);

    // try compiling the source as a shader of the given type
    const GLchar *source_ptr = source.c_str();
    glShaderSource(shaderObject, 1, &source_ptr, 0);
    glCompileShader(shaderObject);

    // retrieve compile status
    GLint status;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
        cout << "ERROR compiling shader:" << endl << endl;
        cout << source << endl;
        cout << info << endl;
    }

    return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    // allocate program object name
    GLuint programObject = glCreateProgram();

    // attach provided shader objects to this program
    if (vertexShader)   glAttachShader(programObject, vertexShader);
    if (fragmentShader) glAttachShader(programObject, fragmentShader);

    // try linking the program with given attachments
    glLinkProgram(programObject);

    // retrieve link status
    GLint status;
    glGetProgramiv(programObject, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint length;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
        string info(length, ' ');
        glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
        cout << "ERROR linking shader program:" << endl;
        cout << info << endl;
    }

    return programObject;
}


// ==========================================================================
