// I promise to work on this exam on my own without receiving any help from any other persons. All of my answers will be my own. 

// Nathan Olah 
// Student Number: 124723198
// 2020-12-09
// GAM531 - Final Exam

/////////////////////////////////////////////////////////////////////////////////////
//
// This code is used to teach the course "game engine foundations" in Seneca college
// Developed by Alireza Moghaddam on Sep. 2020 
//
////////////////////////////////////////////////////////////////////////////////////

using namespace std;

#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\rotate_vector.hpp"
#include "..\SOIL\src\SOIL.h"
#include <iostream>
#include <vector>

//Final Exam code Added 
//Rationale: SceneNode and derived classes of the SceneNode

class SceneNode {
	glm::vec3 m_location;
public : 
	std::vector<SceneNode> m_children; 
	SceneNode(glm::vec3 location) : m_location(location) {}
	
	glm::vec3 getLocation() { return this->m_location; }

	void setLocation(glm::vec3 location) {
		this->m_location = location;
	}

	void draw(float scale, GLuint texture, GLuint rStart, GLuint rEnd, bool isEnemy);
	void render(bool isEnemy);

};

class Wheel : public SceneNode {
public:
	Wheel(glm::vec3 location) : SceneNode(location) {}
	~Wheel() {}
};

class Tank : public SceneNode {
	bool m_isEnemy;
public:
	Tank(glm::vec3 location, bool isEnemy) : SceneNode(location), m_isEnemy(isEnemy) {
		// Wheel positions
		glm::vec3 wheel1Pos = glm::vec3(0.45, 0.45, 0.05); 
		glm::vec3 wheel2Pos = glm::vec3(-0.45, 0.45, 0.05);
		glm::vec3 wheel3Pos = glm::vec3(0.45, -0.45, 0.05);
		glm::vec3 wheel4Pos = glm::vec3(-0.45, -0.45, 0.05);

		Wheel w1(wheel1Pos);
		m_children.push_back(w1);
		Wheel w2(wheel2Pos);
		m_children.push_back(w2);
		Wheel w3(wheel3Pos);
		m_children.push_back(w3);
		Wheel w4(wheel4Pos);
		m_children.push_back(w4);

	}

	bool isEnemy() { return this->m_isEnemy; }

};

class Bullet {
	glm::vec3 m_location;
	glm::vec3 m_enemyLocation;
	int m_bulletLifeTime;
	bool m_isEnemyGun;
public: 
	Bullet(glm::vec3 location, glm::vec3 enemyLocation, bool isEnemyGun) 
		: m_location(location), m_enemyLocation(enemyLocation), m_bulletLifeTime(0), m_isEnemyGun(isEnemyGun) {}
	glm::vec3 getLocation() { return this->m_location; }
	void setLocation(glm::vec3 location) { this->m_location = location; }
	glm::vec3 getEnemyLocation() { return this->m_enemyLocation; }
	void setEnemyLocation(glm::vec3 eLocation) { this->m_enemyLocation = eLocation; }
	int getBulletLifetime() { return this->m_bulletLifeTime; }
	void setBulletLifetime(int dTime) { this->m_bulletLifeTime += dTime; }
	bool isEnemyGun() { return this->m_isEnemyGun; }
};

void spawnTank(bool isEnemy);

//Final Exam code Added 
//Rationale: My global variables i used for the game functionality

std::vector<Tank> sceneGraph; 
std::vector<Bullet> bulletList; // Friendly tank bullets
std::vector<Bullet> enemyBulletList; // Enemies bullets
GLuint spawnTime = 0;
GLuint enemyFireBulletTimer = 0;
GLuint clearBulletsTimer = 0; 
bool alive = true;
GLuint playerScore = 0;
bool playerWins = false;

glm::vec3 randomLocations[7];

enum VAO_IDs { Triangles, NumVAOs };
enum Buffer_IDs { ArrayBuffer};
enum Attrib_IDs { vPosition = 0 };

const GLint NumBuffers = 2;
GLuint VAOs[NumVAOs];
GLuint Buffers[NumBuffers];
GLuint location;
GLuint cam_mat_location;
GLuint proj_mat_location;
GLuint texture[4];	//Array of pointers to textrure data in VRAM. We use two textures in this example.


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
		{ -0.45, -0.45 ,-0.7 }

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
	unsigned char* textureData2 = SOIL_load_image("tank.png", &width2, &height2, 0, SOIL_LOAD_RGB);

	GLint width3, height3;
	unsigned char* textureData3 = SOIL_load_image("ammo.png", &width3, &height3, 0, SOIL_LOAD_RGB);

	GLint width4, height4;
	unsigned char* textureData4 = SOIL_load_image("gunshot.png", &width4, &height4, 0, SOIL_LOAD_RGB);

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

	//Final Exam code Added 
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
	//////////////////////////////////////////////////////////////

	// Fill random location array with random locations	
	for (int i = 0; i < 7; i++) {
		randomLocations[i] = glm::vec3(randomFloat(-40, 40), randomFloat(-40, 40), randomFloat(-40, 40));
	}

}

//Helper function to draw a cube
void drawCube(float scale)
{
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);

	//Select the forth texture (gunshot.png) when drawing the second geometry (cube)
	glBindTexture(GL_TEXTURE_2D, texture[3]); 
	glDrawArrays(GL_QUADS, 4, 24);
}

//Renders level
void draw_level()
{
	//Select the first texture (grass.png) when drawing the first geometry (floor)
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glDrawArrays(GL_QUADS, 0, 4);

}

////////////////////////////////////////////

//Final Exam code Added 
//Rationale: Functions that i used to draw the Tank with wheels SceneNode

GLfloat local = 0;

void drawTank(float scale, GLuint texture, GLuint rowStart, GLuint rowEnd) {
	model_view = glm::scale(model_view, glm::vec3(scale, scale, scale));
	glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
	glBindTexture(GL_TEXTURE_2D, texture);
	glDrawArrays(GL_QUADS, rowStart, rowEnd);
}

void SceneNode::draw(float scale, GLuint tex, GLuint rowStart, GLuint rowEnd, bool isEnemy) {
	drawTank(scale, tex, rowStart, rowEnd);

	if (isEnemy) { // Draws the wheels of the tank
		for (int i = 0; i < m_children.size(); i++) {
			glm::mat4 backup = model_view;
			glm::vec3 vec = m_children.at(i).getLocation();

			model_view = glm::translate(model_view, vec);
			model_view = glm::rotate(model_view, local, glm::vec3(1, 0, 0));
			m_children.at(i).draw(0.3, texture[2], 4, 59, isEnemy); // draw wheel texture 
			model_view = backup;
		}
	}

}

void SceneNode::render(bool isEnemy) {
	glm::mat4 backup = model_view;
	model_view = glm::translate(model_view, this->getLocation());
	draw(3.5, texture[1], 4, 24, isEnemy); // draw body texture
	model_view = backup;
}


void drawTanks() {
	for (int i = 0; i < sceneGraph.size(); i++) {
		sceneGraph.at(i).render(sceneGraph.at(i).isEnemy());
	}
}


void spawnTank(bool isEnemy) {
	if (isEnemy) {
		glm::vec3 randomLocation(randomFloat(-50, 50), randomFloat(-50, 50), 0.0); // Spawn in random location
		Tank tank(randomLocation, true); // This enemy tank status set to true
		sceneGraph.push_back(tank);
	}
}

/* BULLET */

// Collision detection for both the player and the enemy
void checkCollision(int index, bool enemyFired) {
	if (enemyFired) {
		// Players current position
		GLfloat highX = cam_pos.x + 1.5;
		GLfloat lowX = cam_pos.x - 1.5;
		GLfloat highY = cam_pos.y + 1.5;
		GLfloat lowY = cam_pos.y - 1.5;

		// Check if enemy bullet hit the current player position
		if (enemyBulletList.size() > 0) {
			if (enemyBulletList.at(index).getLocation().x <= highX && enemyBulletList.at(index).getLocation().x >= lowX
				&& enemyBulletList.at(index).getLocation().y <= highY && enemyBulletList.at(index).getLocation().y >= lowY) {

				alive = false;
				cout << "Player died" << endl;

				enemyBulletList.erase(enemyBulletList.begin() + index);
			}
			else {
				if (enemyBulletList.at(index).getBulletLifetime() >= 8000) {
					cout << "enemy bullet deleted" << endl;
					enemyBulletList.erase(enemyBulletList.begin() + index);
				}
			}
		} 

	}
	else if (!enemyFired) { // Check if player bullet hit any enemies
		for (int i = 0; i < sceneGraph.size(); i++) {
			if (sceneGraph.at(i).isEnemy()) {
				// Enemy current location
				GLfloat highX = sceneGraph.at(i).getLocation().x + 2.0;
				GLfloat lowX = sceneGraph.at(i).getLocation().x - 2.0;
				GLfloat highY = sceneGraph.at(i).getLocation().y + 2.0;
				GLfloat lowY = sceneGraph.at(i).getLocation().y - 2.0;

				if (bulletList.size() > 0) { // Check each player bullet if it hit an enemy
					if (bulletList.at(index).getLocation().x <= highX && bulletList.at(index).getLocation().x >= lowX
						&& bulletList.at(index).getLocation().y <= highY && bulletList.at(index).getLocation().y >= lowY) {
						cout << "Enemy died" << endl;
						sceneGraph.erase(sceneGraph.begin() + i);

						playerScore++;
					}
					else {
						if (bulletList.at(index).getBulletLifetime() >= 6000) { // Remove bullet after 6 seconds
							bulletList.erase(bulletList.begin() + index);
						}
					}

				}

			}
		}

	}


}

// Draws either the player's bullets or the enemy's bullets
void drawBullet(bool isEnemy) {
	if (isEnemy) {
		for (size_t i = 0; i < enemyBulletList.size(); i++) {
			model_view = glm::translate(model_view, enemyBulletList.at(i).getLocation());
			glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
			drawCube(0.60); // set scale of cube
			model_view = glm::mat4(1.0);
		}
	}
	else if (!isEnemy) {
		for (size_t i = 0; i < bulletList.size(); i++) {
			model_view = glm::translate(model_view, bulletList.at(i).getLocation());
			glUniformMatrix4fv(location, 1, GL_FALSE, &model_view[0][0]);
			drawCube(0.60); // set scale of cube
			model_view = glm::mat4(1.0);
		}

	}

}

// Update the current bullet locations for both the enemy bullets and player bullets
void updateBullet() {
	for (int i = 0; i < enemyBulletList.size(); i++) {
		glm::vec3 bullet_dir = enemyBulletList.at(i).getLocation() - glm::vec3(0.01, 0.01, 0.01) * enemyBulletList.at(i).getEnemyLocation(); // current player position
		bullet_dir.z = 0.5;
		enemyBulletList.at(i).setLocation(bullet_dir);
		drawBullet(true);

		enemyBulletList.at(i).setBulletLifetime(deltaTime);
		checkCollision(i, true);
	}
		 
	for (int i = 0; i < bulletList.size(); i++) {
		glm::vec3 bullet_dir = bulletList.at(i).getLocation() + glm::vec3(0.05, 0.05, 0.05) * bulletList.at(i).getEnemyLocation();
		bulletList.at(i).setLocation(bullet_dir);
		drawBullet(false);

		bulletList.at(i).setBulletLifetime(deltaTime);
		checkCollision(i, false);
	}
}

// Enemy will fire bullet at player position
void fireBullet(int index) {
	bool isEnemyGun = true;
	if (enemyFireBulletTimer >= 2000) { // Enemy fires every 2 seconds
		cout << "firing at player" << endl;
		Bullet bullet(sceneGraph.at(index).getLocation(), looking_dir_vector, isEnemyGun); 
		//bulletList.push_back(bullet);
		enemyBulletList.push_back(bullet);
		drawBullet(true);
		enemyFireBulletTimer = 0;
	}
}

// The enemy range to target the player
bool enemyInRange(int enemyIdx) {
	bool inRange = false;
	GLfloat highX = cam_pos.x + 100.0;
	GLfloat lowX = cam_pos.x - 100.0;
	GLfloat highY = cam_pos.y + 100.0;
	GLfloat lowY = cam_pos.y - 100.0;

	// if enemy is within range then return true else return false
	if (sceneGraph.at(enemyIdx).getLocation().x <= highX && sceneGraph.at(enemyIdx).getLocation().x >= lowX
			&& sceneGraph.at(enemyIdx).getLocation().y <= highY && sceneGraph.at(enemyIdx).getLocation().y >= lowY) {
		inRange = true;
	}

	return inRange;
}

// Updates current enemy tank location and speed, also checks if a player is within the enemy tank's range 
void updateTank() {
	for (int i = 0; i < sceneGraph.size(); i++) {
		if (sceneGraph.at(i).isEnemy()) {

			// if enemy is near cam_pos follow the cam_pos else move in random directions
			if (enemyInRange(i)) {
				// move toward cam_pos
				glm::vec3 tank_dir = sceneGraph.at(i).getLocation() - glm::vec3(0.0005, 0.0005, 0.0005) * (looking_dir_vector + glm::vec3(1,1,1)); 

				tank_dir.z = 0.2;
				sceneGraph.at(i).setLocation(tank_dir);

				// Enemy will fire at player
				fireBullet(i);
			}
			else if (!enemyInRange(i)) {
				// move in random directions with random speeds
				glm::vec3 tank_dir = sceneGraph.at(i).getLocation() + 
					glm::vec3(randomFloat(0.00010, 0.00005), randomFloat(0.00010, 0.00005), randomFloat(0.00010, 0.00005)) * randomLocations[i];

				tank_dir.z = 0.2;
				sceneGraph.at(i).setLocation(tank_dir);
			}

		}

	}
}

////////////////////////////////////////////

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

	drawTanks();
	updateBullet();

	glFlush();
}


void keyboard(unsigned char key, int x, int y)
{
	//Final Exam code Added 
	//Rationale: Check if player wins game or is not alive

	if (!playerWins) {
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
			if (key == 'f' || key == 'F') {
				bool isPlayer = false;
				Bullet bullet(cam_pos, looking_dir_vector, isPlayer); // player bullet boolean
				bulletList.push_back(bullet);
			}

		}
	}

}

//Controlling Pitch with vertical mouse movement
void mouse(int x, int y)
{
	//Final Exam code Added 
	//Rationale: Check if player wins game or is not alive

	if (!playerWins) {
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
}

void idle()
{
	//Final Exam code Added 
	//Rationale: For each frame rate check the game status if player wins or loses, updates tank and bullet locations and updates timers
	if (playerScore == 5) {
		playerWins = true;
		cout << "Player Wins" << endl;
	}
	else if (!alive) {
		cout << "Game over player died" << endl;
	}

	if (spawnTime >= 1000) { // 1 second enemy spawn
		spawnTank(true);
		spawnTime = 0;
	}

	local += 0.0050;
	updateTank();
	updateBullet();

	//Calculating the delta time between two frames
	//We will use this delta time when moving forward (in keyboard function)
	int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
	deltaTime = timeSinceStart - oldTimeSinceStart;
	oldTimeSinceStart = timeSinceStart;
	//cout << timeSinceStart << " " << oldTimeSinceStart << " " << deltaTime << endl;

	enemyFireBulletTimer += deltaTime;
	spawnTime += deltaTime;

	glutPostRedisplay();
	

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
	glutCreateWindow("GAM531 Final Exam - Nathan Olah");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	init();

	glutDisplayFunc(display);

	glutKeyboardFunc(keyboard);

	glutIdleFunc(idle);

	glutPassiveMotionFunc(mouse);

	glutMainLoop();
	

}
