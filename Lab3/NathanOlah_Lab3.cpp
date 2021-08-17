// Nathan Olah
// nolah@myseneca.ca
// Student Number: 124723198
// GAM531 - Lab 3
// 2020-10-04

///////////////////////////////////////////////////////////////////////
//
// triangles.cpp
//
///////////////////////////////////////////////////////////////////////

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const GLint NumBuffers = 2;

GLuint Buffers[NumBuffers];

const GLuint NumVertices = 6;

// Will be used to store the location of our model-view matrix in VRAM
GLuint location;
GLfloat global = 0.0;
GLfloat local = 0.0;

//---------------------------------------------------------------------
//
// initialization: Setting up our rendering pipeline. Loading Shaders. Creating buffers in VRAM. 
//
//---------------------------------------------------------------------
void init(void)
{

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);	//Loading the shaders from files: triangles.vert & triangles.frag
	glUseProgram(program);	//Compiling and running the shaders. And now, my Pipeline is set up :-)

	GLfloat vertices[NumVertices][2] = {
		// We use two triangles and connect them together to get a square. We're ONLY allowed to use triangles in this course.

		//First triangle
		{ -0.45, 0.45 },
		{ 0.45, 0.45 },
		{ 0.45, -0.45 },

		//Second triangle
		{ 0.45, -0.45 },
		{ -0.45, -0.45 },
		{ -0.45, 0.45 }

	};

	GLfloat colorData[NumVertices][3] = {
		{ 0,1,1 }, // colors for vertices of the first triangle
		{ 0,0.2,1 },
		{ 0,1,1 },

		{ 0,1,1 }, // colors for vertices of the second triangle
		{ 0,0.2,1 },
		{ 0,1,1 }

	};

	//We allocate two buffers in VRAM: One for vertex data and the other for colors
	//Once created, we use Buffers[0] to point to the first, and Buffers[1] to point to the second.
	glGenBuffers(2, Buffers);

	//-----------------------------------------
	//Initializing the first buffer: Buffers[0]
	//-----------------------------------------

	//Selecting Buffers[0]
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);

	//Pushing the vertices data into the buffer (transmission from RAM to VRAM)
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	//During the run-time, the buffer data should be transferred to the "vPosition" variable in the vertex shader
	glBindAttribLocation(program, 0, "vPosition");

	//We specify the format of the data in the buffer: 
	// GL_FLOAT: They are float as we have used a GLfloat[] array up in the code
	// 2: They must be considered as couples since we have 2D vertices
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//Enable the buffer
	glEnableVertexAttribArray(0);
	//-------------------------------------------------------------------------
	//---------------We are done initializing the first buffer-----------------
	//-------------------------------------------------------------------------







	//-----------------------------------------
	//Initializing the second buffer: Buffers[1]
	//-----------------------------------------

	//Selecting Buffers[1]
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);

	//Pushing the color data into the buffer (transmission from RAM to VRAM)
	glBufferData(GL_ARRAY_BUFFER, sizeof(colorData), colorData, GL_STATIC_DRAW);

	//During the run-time, the buffer data should be transferred to the "vertexColor" variable in the vertex shader
	glBindAttribLocation(program, 1, "vertexColor");

	//We specify the format of the data in the buffer: 
	// GL_FLOAT: They are float as we have used a GLfloat[] array up in the code
	// 3: They must be considered as triplets since we use 3 values for each color(red, green, blue)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	//Enable the buffer
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_view");

}
//-------------------------------------------------------------------------
//---------------We are done initializing the second buffer----------------
//-------------------------------------------------------------------------


//---------------Initialization accomplished :-)




//---------------------------------------------------------------------
//
// display function. All drawings happen here in the "display" function
//
//---------------------------------------------------------------------
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);	//Clears the screen and ready to draw

	glm::mat4 model_view_matrix = glm::rotate(glm::mat4(1.0), global, glm::vec3(0, 0, 1));
	model_view_matrix = glm::translate(model_view_matrix, glm::vec3(1.0, 0.0, 0.0));
	model_view_matrix = glm::scale(model_view_matrix, glm::vec3(0.3, 0.3, 0.3));
	model_view_matrix = glm::rotate(model_view_matrix, local, glm::vec3(0, 0, 1));

	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view_matrix[0][0]);
	
	glDrawArrays(GL_TRIANGLES, 0, 6);	//Connect the vertices using "GL_TRIANGLES" modes.
										//Read more about draw modes here:
										//https://www.glprogramming.com/red/chapter02.html
										//https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glDrawArrays.xml

	glFlush();							//Flush the rendered contents on the screen.
}

void keyboard(unsigned char key, int x, int y)
{
	//Handler function for keyboard events
	//To be developed in next lectures
}

void mouse(int state, int button, int x, int y)
{
	//Handler function for keyboard events
	//To be developed in next lectures
}

//This function gets called for every frame. This will be used to animate the world 
void idle()
{	
	local += 0.0015; // local axis spin
	global += 0.0005; // global axis spin
	glutPostRedisplay();	//This is the explicit call to display function: display()
}

//---------------------------------------------------------------------
//
// main
//

int
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);

	//The dimensions of the window
	glutInitWindowSize(512, 512);

	//The title for the window
	glutCreateWindow("Nathan Olah - Lab 3");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	//Defining our 'display', 'idle', 'mouse' and 'keyboard' functions.
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	//The following function makes the OpenGL to go through an infinite loop and waits for any event from keyboard, mouse, etc.
	glutMainLoop();



}
