// Nathan Olah - Lab 4
// Student Number: 124723198
// nolah@myseneca.ca
// 2020-10-08

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const GLint NumBuffers = 2;
GLuint Buffers[NumBuffers];
const GLuint NumVertices = 6;

// Will be used to store the location of our model-view matrix in VRAM
//GLuint location;
GLuint model_view_matrix_location;
GLuint camera_matrix_location;
GLuint projection_matrix_location;

GLfloat global = 0.0;
GLfloat local = 0.0;

glm::vec3 cam_pos = glm::vec3(0, 0, 0.1); 
glm::vec3 cam_dir = glm::vec3(0, 0, 0);
glm::vec3 cam_up_vector = glm::vec3(0, 1, 0);

//---------------------------------------------------------------------
//
// initialization: Setting up our rendering pipeline. Loading Shaders. Creating buffers in VRAM. 
//
//---------------------------------------------------------------------
void init(void)
{
	glEnable(GL_DEPTH_TEST); //Enabling the depth test so that closer objects to camera obscure further objects
	
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);	//Loading the shaders from files: triangles.vert & triangles.frag
	glUseProgram(program);	//Compiling and running the shaders. And now, my Pipeline is set up :-)

	GLfloat vertices[] = {
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		  0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // front panel
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		  0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // back panel
		  0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		 -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // left panel
		 -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		  0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		  0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // right panel
		  0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		  0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		  0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, // bottom panel 
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,

		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f, // top panel
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		  0.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	};

	GLfloat colorData[][3] = {
		// red
		{ 1,0,0 }, 
		{ 1,0,0 },
		{ 1,0,0 },

		{ 1,0,0 }, 
		{ 1,0,0 },
		{ 1,0,0 },
		
		// green
		{ 0,1,0 }, 
		{ 0,1,0 },
		{ 0,1,0 },

		{ 0,1,0 }, 
		{ 0,1,0 },
		{ 0,1,0 },
		
		// orange
		{ 1,0.5,0.0 }, 
		{ 1,0.5,0.0 },
		{ 1,0.5,0.0 },

		{ 1,0.5,0.0 }, 
		{ 1,0.5,0.0 },
		{ 1,0.5,0.0 },
		
		// cyan
		{ 0,1,1 }, 
		{ 0,1,1 },
		{ 0,1,1 },

		{ 0,1,1 }, 
		{ 0,1,1 },
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
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0)); // 3 for 3D
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

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
//-------------------------------------------------------------------------
//---------------We are done initializing the second buffer----------------
//-------------------------------------------------------------------------

	model_view_matrix_location = glGetUniformLocation(program, "model_view");
	camera_matrix_location = glGetUniformLocation(program, "camera");
	projection_matrix_location = glGetUniformLocation(program, "projection");
}


//---------------Initialization accomplished :-)




//---------------------------------------------------------------------
//
// display function. All drawings happen here in the "display" function
//
//---------------------------------------------------------------------
void display(void)
{	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//Clears the screen and ready to draw

	glm::mat4 model_view_matrix = glm::mat4(1.0);
	model_view_matrix = glm::translate(model_view_matrix, glm::vec3(0.0, 0.0, 0.0));
	model_view_matrix = glm::scale(model_view_matrix, glm::vec3(0.5, 0.5, 0.05));
	model_view_matrix = glm::rotate(model_view_matrix, local, glm::vec3(1, 1, 0.6));
	
	glUniformMatrix4fv(model_view_matrix_location, 1, GL_FALSE, &model_view_matrix[0][0]);
	
	glm::mat4 camera_mat = glm::lookAt(cam_pos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glUniformMatrix4fv(camera_matrix_location, 1, GL_FALSE, &camera_mat[0][0]);

	glm::mat4 projection_mat = glm::frustum(-0.1, +0.1, -0.1, +0.1, 0.01, 10.0);
	glUniformMatrix4fv(projection_matrix_location, 1, GL_FALSE, &projection_mat[0][0]);



	glDrawArrays(GL_TRIANGLES, 0, 36);	//Connect the vertices using "GL_TRIANGLES" modes.
										//Read more about draw modes here:
										//https://www.glprogramming.com/red/chapter02.html
										//https://www.khronos.org/registry/OpenGL-Refpages/es2.0/xhtml/glDrawArrays.xml


	glFlush();							//Flush the rendered contents on the screen.
}

void keyboard(unsigned char key, int x, int y)
{
	//Handler function for keyboard events
	//To be developed in next lectures
	switch (key) {
	case 'i':
		cam_pos.z -= 0.005;
		break;
	case 'o':
		cam_pos.z += 0.005;
		break;
	case 'w':
		cam_pos.y += 0.001;
		break;
	case 's':
		cam_pos.y -= 0.001;
		break;
	case 'a':
		cam_pos.x -= 0.001;
		break;
	case 'd':
		cam_pos.x += 0.001;
		break;

	}

	glutPostRedisplay();
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
	glutCreateWindow("Lab 4 - Nathan Olah");

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
