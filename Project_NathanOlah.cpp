// Nathan Olah
// GAM531 - Project
// 2020-12-05
// Student Number: 124723198

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <iostream>
#include <vector>

class GameObject {
	char m_type;
	glm::vec3 m_location;
	glm::vec3 m_direction;
	int m_bulletTime;
	bool m_isAlive;
	bool m_isEnemy;
public:
	GameObject() {}
	GameObject(char type, glm::vec3 loc, glm::vec3 dir, bool isEnemy)
		: m_type(type), m_location(loc), m_direction(dir), m_isAlive(true), m_isEnemy(isEnemy), m_bulletTime(0) {}
	~GameObject() {}
	glm::vec3 getLocation() { return m_location; }
	glm::vec3 getDirection() { return m_direction; }

	int getBulletTime() { return m_bulletTime; }
	void setBulletTime(int dTime) { this->m_bulletTime += dTime; }

	void setDirection(glm::vec3 direction) { this->m_direction = direction; }

	void setLocation(glm::vec3 location) {
		this->m_location = location;

		// Checks if object's location is out of bounds
		if (this->m_location.x < -200 || this->m_location.x > 200) {
			m_isAlive = false;
		}
		else {
			m_isAlive = true;
		}

	}

	void setAlive(bool alive) { this->m_isAlive = alive; }
	bool isAlive() { return this->m_isAlive; }
	bool isEnemy() { return this->m_isEnemy; }

};

class SceneNode {
	glm::vec3 m_transformation;
	bool m_isAlive;
public:
	std::vector<SceneNode> m_children;
	SceneNode(glm::vec3 transformation) : m_transformation(transformation), m_isAlive(true) {}
	glm::vec3 getTransformation() { return m_transformation; }
	void setLocation(glm::vec3 location) { m_transformation = location; }

	void draw(float scale, GLuint tex, GLuint rowStart, GLuint rowEnd, GLenum mode);
	void render();

	void setAlive(bool alive) { this->m_isAlive = alive; }
	bool isAlive() { return this->m_isAlive; }

};

class Wheel : public SceneNode {
public:
	Wheel(glm::vec3 transform) : SceneNode(transform) {
		//std::cout << "wheel created" << endl;
	}

};

class Tank : public SceneNode {
public:
	Tank(glm::vec3 transformation) : SceneNode(transformation) {
		//cout << "body created " << endl;
		glm::vec3 wt1 = glm::vec3(0.45, 0.45, 0.1);
		glm::vec3 wt2 = glm::vec3(-0.45, 0.45, 0.1);
		glm::vec3 wt3 = glm::vec3(0.45, -0.45, 0.1);
		glm::vec3 wt4 = glm::vec3(-0.45, -0.45, 0.1);

		Wheel w1(wt1);
		m_children.push_back(w1);

		Wheel w2(wt2);
		m_children.push_back(w2);

		Wheel w3(wt3);
		m_children.push_back(w3);

		Wheel w4(wt4);
		m_children.push_back(w4);
	}

};


std::vector<GameObject> gameScene;
std::vector<SceneNode> sceneGraph;
int spawnTime = 0;
GLuint playerScore = 0;
bool alive = true; // Player's life status

void drawObjects();


enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer};
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[4]; //Array of pointers to textrure data in VRAM. We use two textures in this example.


const GLuint NumVertices = 56;

//Height of camera (player) from the level
float height = 0.8f;

//Player motion speed for movement and pitch/yaw
float travel_speed = 300.0f;		//Motion speed
float mouse_sensitivity = 0.01f;	//Pitch/Yaw speed

//Used for tracking mouse cursor position on screen
int x0 = 0;	
int y_0 = 0;
 
//Transformation matrices and camera vectors
glm::mat4 model_view;
glm::vec3 unit_z_vector = glm::vec3(0, 0, 1);	//Assigning a meaningful name to (0,0,1) :-)
glm::vec3 cam_pos = glm::vec3(0.0f, 0.0f, height);
glm::vec3 forward_vector = glm::vec3(1, 1, 0);	//Forward vector is parallel to the level at all times (No pitch)

//The direction which the camera is looking, at any instance
glm::vec3 looking_dir_vector = glm::vec3(1, 1, 0);
glm::vec3 up_vector = unit_z_vector;
glm::vec3 side_vector = glm::cross(up_vector, forward_vector);


//Used to measure time between two frames
int oldTimeSinceStart = 0;
int deltaTime;

//Creating and rendering bunch of objects on the scene to interact with
const int Num_Obstacles = 50;
float obstacle_data[Num_Obstacles][3];


//Helper function to generate a random float number within a range
float randomFloat(float a, float b)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

// inititializing buffers, coordinates, setting up pipeline, etc.
void init(void)
{
	glEnable(GL_DEPTH_TEST);

	//Normalizing all vectors
	up_vector = glm::normalize(up_vector);
	forward_vector = glm::normalize(forward_vector);
	looking_dir_vector = glm::normalize(looking_dir_vector);
	side_vector = glm::normalize(side_vector);

	//Randomizing the position and scale of obstacles
	for (int i = 0; i < Num_Obstacles; i++)
	{
		obstacle_data[i][0] = randomFloat(-50, 50); //X
		obstacle_data[i][1] = randomFloat(-50, 50); //Y
		obstacle_data[i][2] = randomFloat(0.1, 10.0); //Scale
	}

	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};

	GLuint program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up


	//Since we use texture mapping, to simplify the task of texture mapping, 
	//and to clarify the demonstration of texture mapping, we consider 4 vertices per face.
	//Overall, we will have 24 vertices and we have 4 vertices to render the sky (a large square).
	//Therefore, we'll have 28 vertices in total.
	GLfloat vertices[NumVertices][3] = {
		
		{ -100.0, -100.0, 0.0 }, //Plane to walk on and a sky
		{ 100.0, -100.0, 0.0 },
		{ 100.0, 100.0, 0.0 },
		{ -100.0, 100.0, 0.0 },

		{ -0.45, -0.70 ,0.01 }, // bottom face
		{ 0.45, -0.70 ,0.01 },
		{ 0.45, 0.70 ,0.01 },
		{ -0.45, 0.70 ,0.01 }, 

		{ -0.45, -0.45 ,0.7 }, //top face 
		{ 0.45, -0.45 ,0.7 },
		{ 0.45, 0.45 ,0.7 },
		{ -0.45, 0.45 ,0.7 },

		{ 0.45, -0.70 , 0.01 }, //left face
		{ 0.45, 0.70 , 0.01 },
		{ 0.45, 0.45 ,0.7 },
		{ 0.45, -0.45 ,0.7 },

		{ -0.45, -0.70, 0.01 }, //right face
		{ -0.45, 0.70 , 0.01 },
		{ -0.45, 0.45 ,0.7 },
		{ -0.45, -0.45 ,0.7 },

		{ -0.45, 0.70 , 0.01 }, //front face 
		{ 0.45, 0.70 , 0.01 },
		{ 0.45, 0.45 ,0.7 },
		{ -0.45, 0.45 ,0.7 },
	
		{ -0.45, -0.70 , 0.01 }, //back face
		{ 0.45, -0.70, 0.01 },
		{ 0.45, -0.45 ,0.7 },
		{ -0.45, -0.45 ,0.7 },

		// Hexagon
		{ 0.45, 0.70 ,-0.01 }, // bottom face
		{ -0.45, 0.70 ,-0.01 },
		{ -0.45, -0.70 ,-0.01 },
		{ 0.45, -0.70 ,-0.01 },

		{ -0.45, -0.45 ,-0.7 }, //top face 
		{ 0.45, -0.45 ,-0.7 },
		{ 0.45, 0.45 ,-0.7 },
		{ -0.45, 0.45 ,-0.7 },

		{ 0.45, -0.70 , -0.01 }, //left face
		{ 0.45, 0.70 , -0.01 },
		{ 0.45, 0.45 ,-0.7 },
		{ 0.45, -0.45 ,-0.7 },

		{ -0.45, -0.70, -0.01 }, //right face
		{ -0.45, 0.70 , -0.01 },
		{ -0.45, 0.45 ,-0.7 },
		{ -0.45, -0.45 ,-0.7 },

		{ -0.45, 0.70 , -0.01 }, //front face 
		{ 0.45, 0.70 , -0.01 },
		{ 0.45, 0.45 ,-0.7 },
		{ -0.45, 0.45 ,-0.7 },

		{ -0.45, -0.70 , -0.01 }, //back face
		{ 0.45, -0.70, -0.01 },
		{ 0.45, -0.45 ,-0.7 },
		{ -0.45, -0.45 ,-0.7 },


	};

	//These are the texture coordinates for the second texture
	GLfloat textureCoordinates[56][2] = { 
		0.0f, 0.0f,
		200.0f, 0.0f,
		200.0f, 200.0f,
		0.0f, 200.0f,
		
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
		
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		// hexagon
		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,

		0.0f, 1.0f,
		1.0f, 1.0f,
		1.0f, 0.0f,
		0.0f, 0.0f,
	};


	//Creating our texture:
	//This texture is loaded from file. To do this, we use the SOIL (Simple OpenGL Imaging Library) library.
	//When using the SOIL_load_image() function, make sure the you are using correct patrameters, or else, your image will NOT be loaded properly, or will not be loaded at all.
	GLint width1, height1;
	unsigned char* textureData1 = SOIL_load_image("grass.png", &width1, &height1, 0, SOIL_LOAD_RGB);

	GLint width2, height2;
	unsigned char* textureData2 = SOIL_load_image("gunshot.png", &width2, &height2, 0, SOIL_LOAD_RGB);

	GLint width3, height3;
	unsigned char* textureData3 = SOIL_load_image("tank.png", &width3, &height3, 0, SOIL_LOAD_RGB); 

	GLint width4, height4;
	unsigned char* textureData4 = SOIL_load_image("ammo.png", &width4, &height4, 0, SOIL_LOAD_RGB);

	glGenBuffers(2, Buffers);
	glBindBuffer(GL_ARRAY_BUFFER, Buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindAttribLocation(program, 0, "vPosition");
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, Buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(textureCoordinates), textureCoordinates, GL_STATIC_DRAW);
	glBindAttribLocation(program, 1, "vTexCoord");
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glEnableVertexAttribArray(1);

	location = glGetUniformLocation(program, "model_matrix");
	cam_mat_location = glGetUniformLocation(program, "camera_matrix");
	proj_mat_location = glGetUniformLocation(program, "projection_matrix");

	///////////////////////TEXTURE SET UP////////////////////////
	
	//Allocating two buffers in VRAM
	glGenTextures(4, texture);

	//First Texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[0]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData1);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//And now, second texture: 

	//Set the type of the allocated buffer as "TEXTURE_2D"
	glBindTexture(GL_TEXTURE_2D, texture[1]);

	//Loading the second texture into the second allocated buffer:
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width2, height2, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData2);

	//Setting up parameters for the texture that recently pushed into VRAM
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//////////////////////////////////////////////////////////////

	// Texture 3
	glBindTexture(GL_TEXTURE_2D, texture[2]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width3, height3, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData3);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, texture[3]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width4, height4, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData4);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

}

//Helper function to draw a cube
void drawCube(float scale)
{
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//Select the second texture (gunshot.png) when drawing the second geometry (cube)
	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glDrawArrays(GL_QUADS, 4, 24);
}

///////////////////////////////////////////////////
GLfloat local = 0.0;

void drawTank(float scale, GLuint texture, GLuint rowStart, GLuint rowEnd, GLenum mode) {
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(mode, rowStart, rowEnd);
}

void SceneNode::draw(float scale, GLuint tex, GLuint rowStart, GLuint rowEnd, GLenum mode) {
	
	drawTank(scale, tex, rowStart, rowEnd, mode);

	for (int i = 0; i < m_children.size(); i++) {
		glm::mat4 backup = model_view;

		glm::vec3 vec = m_children.at(i).getTransformation();
		model_view = glm::translate(model_view, vec);
		model_view = glm::rotate(model_view, local, glm::vec3(1,0,0));

		m_children.at(i).draw(0.27, texture[3], 4, 59, GL_QUADS); // vertices for hexagon
	
		model_view = backup;

	}
}

void SceneNode::render() {
	glm::mat4 backup = model_view;
	model_view = glm::translate(model_view, this->getTransformation());

	draw(3.5, texture[2], 4, 24, GL_QUADS); // vertices 4 to 24 for body
	model_view = backup;
}

void drawTanks() {
	for (int i = 0; i < sceneGraph.size(); i++) {
		sceneGraph.at(i).render();
	}
}

void checkCollision(int index) {
	// Player and Tank detection
	// Range of current player position
	GLfloat highX = 0.0, lowX = 0.0, highY = 0.0, lowY = 0.0;
	highX = cam_pos.x + 1.7;
	lowX = cam_pos.x - 1.7;
	highY = cam_pos.y + 1.7;
	lowY = cam_pos.y - 1.7;

	// Player gets hit by tank and game is over.
	if (sceneGraph.at(index).getTransformation().x <= highX && sceneGraph.at(index).getTransformation().x >= lowX
		&& sceneGraph.at(index).getTransformation().y <= highY && sceneGraph.at(index).getTransformation().y >= lowY) {

		alive = false;
		looking_dir_vector.z = 1;
	}

	// Delete tank if it reaches the center
	if (sceneGraph.size() > 0 || sceneGraph.at(index).isAlive()) {
		if ((int)sceneGraph.at(index).getTransformation().x == 0 && (int)sceneGraph.at(index).getTransformation().y == 0) {

			sceneGraph.at(index).setAlive(false);
			sceneGraph.erase(sceneGraph.begin() + index);
		}
	}

	int deleted = 0;

	// Vehicle has collided with another vehicle
	for (int i = 0; i < sceneGraph.size(); i++) {
		if (i != index) {
			// create collision range of tank
			highX = sceneGraph.at(i).getTransformation().x + 3.5; 
			lowX = sceneGraph.at(i).getTransformation().x - 3.5;
			highY = sceneGraph.at(i).getTransformation().y + 3.5;
			lowY = sceneGraph.at(i).getTransformation().y - 3.5;

			if (sceneGraph.size() > 0 || sceneGraph.at(index).isAlive()) {
				if (sceneGraph.at(index).getTransformation().x <= highX && sceneGraph.at(index).getTransformation().x >= lowX
					&& sceneGraph.at(index).getTransformation().y <= highY && sceneGraph.at(index).getTransformation().y >= lowY) {

					sceneGraph.at(index).setAlive(false);
					sceneGraph.erase(sceneGraph.begin() + index);

					deleted = index;
					break; 
				}
			}

		}

	}

	// Vehicle is collided by a bullet(shot by the player)
	GLfloat hiX = 0.0, loX = 0.0, hiY = 0.0, loY = 0.0, hiZ = 0.0, loZ = 0.0;
	for (int i = 0; i < gameScene.size(); i++) {
		if (deleted == 0 && sceneGraph.size() > 0 && sceneGraph.at(index).isAlive()) {

			hiX = sceneGraph.at(index).getTransformation().x + 2.2;
			loX = sceneGraph.at(index).getTransformation().x - 2.2;
			hiY = sceneGraph.at(index).getTransformation().y + 2.2;
			loY = sceneGraph.at(index).getTransformation().y - 2.2;
			hiZ = sceneGraph.at(index).getTransformation().z + 2.2;
			loZ = sceneGraph.at(index).getTransformation().z - 2.2;
			if (gameScene.at(i).getLocation().x <= hiX && gameScene.at(i).getLocation().x >= loX
				&& gameScene.at(i).getLocation().y <= hiY && gameScene.at(i).getLocation().y >= loY 
				&& gameScene.at(i).getLocation().z <= hiZ && gameScene.at(i).getLocation().z >= loZ) {
				

				playerScore++; 

				sceneGraph.at(index).setAlive(false);
				sceneGraph.erase(sceneGraph.begin() + index);

				gameScene.at(i).setAlive(false);
				gameScene.erase(gameScene.begin() + i);

				break;
			}


		}


	}

}

void updateTank() {
	for (int i = 0; i < sceneGraph.size(); i++) {
		
		glm::vec3 tank_dir = sceneGraph.at(i).getTransformation() - glm::vec3(0.0001, 0.0001, 0.0001) * sceneGraph.at(i).getTransformation();

		tank_dir.z = 0.2;
		sceneGraph.at(i).setLocation(tank_dir);
		
		checkCollision(i);

	}
}

void spawnTank() {
	glm::vec3 tankSpawn(randomFloat(-50, 50), randomFloat(-50, 50), 0.0);
	Tank tank(tankSpawn);
	sceneGraph.push_back(tank);
}

/////////////////////////////////////////////////////

//Renders level
void draw_level()
{
	//Select the first texture (grass.png) when drawing the first geometry (floor)
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_QUADS, 0, 4);

}

// Updates location of the object in the scene
void updateScene() {
	for (size_t i = 0; i < gameScene.size(); i++) {
		if (!gameScene.at(i).isEnemy()) {
			gameScene.at(i).setLocation(gameScene.at(i).getLocation() + glm::vec3(0.05, 0.05, 0.05) * gameScene.at(i).getDirection());

			gameScene.at(i).setBulletTime(deltaTime);

			// If the bullet moves out of the boundaries, this removes the bullet from the gameScene
			// Or 2 seconds after spawn
			if (!gameScene.at(i).isAlive() || gameScene.at(i).getBulletTime() >= 2000) {
				gameScene.erase(gameScene.begin() + i);
			}
		}
	}

}

// Draws cube at current position
void drawObjects() {
	for (size_t i = 0; i < gameScene.size(); i++) {
		if (!gameScene.at(i).isEnemy()) {
			model_view = glm::translate(model_view, gameScene.at(i).getLocation());

			glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
			drawCube(0.40); // set scale of cube
			model_view = glm::mat4(1.0);

		}
	}

}



//---------------------------------------------------------------------
//
// display
//
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	model_view = glm::mat4(1.0);
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//The 3D point in space that the camera is looking
	glm::vec3 look_at = cam_pos + looking_dir_vector;

	glm::mat4 camera_matrix = glm::lookAt(cam_pos, look_at, up_vector);
	glUniformMatrix4fv(cam_mat_location, 1, GL_FALSE, &camera_matrix[0][0]);

	glm::mat4 proj_matrix = glm::frustum(-0.01f, +0.01f, -0.01f, +0.01f, 0.01f, 100.0f);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, &proj_matrix[0][0]);

	draw_level();
	
	drawObjects();
	drawTanks();

	glFlush();
}


void keyboard(unsigned char key, int x, int y) {

	if (alive) {
		if (key == 'a')
		{
			//Moving camera along opposit direction of side vector
			cam_pos += side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'd')
		{
			//Moving camera along side vector
			cam_pos -= side_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 'w')
		{
			//Moving camera along forward vector. To be more realistic, we use X=V.T equation in physics
			cam_pos += forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}
		if (key == 's')
		{
			//Moving camera along backward (negative forward) vector. To be more realistic, we use X=V.T equation in physics
			cam_pos -= forward_vector * travel_speed * ((float)deltaTime) / 1000.0f;
		}

		// Adds instance of GameObject to "gameScene" collection
		if (key == 'f' || key == 'F') {
			glm::vec3 currentLocation = cam_pos; // current position
			GameObject obj('p', currentLocation, looking_dir_vector, false); // sets to current location and current looking direction
			gameScene.push_back(obj);
			drawObjects();
		}
	
	}

}


//Controlling Pitch with vertical mouse movement
void mouse(int x, int y)
{

	if (alive) {
		//Controlling Yaw with horizontal mouse movement
		int delta_x = x - x0;
	
		//The following vectors must get updated during a yaw movement
		forward_vector = glm::rotate(forward_vector, -delta_x * mouse_sensitivity, unit_z_vector);
		looking_dir_vector = glm::rotate(looking_dir_vector, -delta_x * mouse_sensitivity, unit_z_vector);
		side_vector = glm::rotate(side_vector, -delta_x * mouse_sensitivity, unit_z_vector);
		up_vector = glm::rotate(up_vector, -delta_x * mouse_sensitivity, unit_z_vector);
		x0 = x;

		//The following vectors must get updated during a pitch movement
		int delta_y = y - y_0; 
		glm::vec3 tmp_up_vec = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
		glm::vec3 tmp_looking_dir = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);

		//The dot product is used to prevent the user from over-pitch (pitching 360 degrees)
		//The dot product is equal to cos(theta), where theta is the angle between looking_dir and forward vector
		GLfloat dot_product = glm::dot(tmp_looking_dir, forward_vector);

		//If the angle between looking_dir and forward vector is between (-90 and 90) degress 
		if (dot_product > 0)
		{
			up_vector = glm::rotate(up_vector, delta_y * mouse_sensitivity, side_vector);
			looking_dir_vector = glm::rotate(looking_dir_vector, delta_y * mouse_sensitivity, side_vector);
		}
		y_0 = y;
	
	}

}

void idle() {

	if (playerScore < 10) {
		if (spawnTime >= 2000) {
			spawnTank();
			spawnTime = 0;
		}

		local += 0.001;
		updateScene();
		updateTank();

		//Calculating the delta time between two frames
		//We will use this delta time when moving forward (in keyboard function)
		int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
		deltaTime = timeSinceStart - oldTimeSinceStart;
		oldTimeSinceStart = timeSinceStart;
		//cout << timeSinceStart << " " << oldTimeSinceStart << " " << deltaTime << endl;

		spawnTime += deltaTime; // once it reaches 2 seconds call the spawn function

		glutPostRedisplay();
	}
	else {
		cout << "Player Wins" << endl;
	}

}

//---------------------------------------------------------------------
//
// main
//
int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("GAM531 Project - Nathan Olah");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();
	
	

}
