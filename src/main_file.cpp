/*
Niniejszy program jest wolnym oprogramowaniem; możesz go
rozprowadzać dalej i / lub modyfikować na warunkach Powszechnej
Licencji Publicznej GNU, wydanej przez Fundację Wolnego
Oprogramowania - według wersji 2 tej Licencji lub(według twojego
wyboru) którejś z późniejszych wersji.

Niniejszy program rozpowszechniany jest z nadzieją, iż będzie on
użyteczny - jednak BEZ JAKIEJKOLWIEK GWARANCJI, nawet domyślnej
gwarancji PRZYDATNOŚCI HANDLOWEJ albo PRZYDATNOŚCI DO OKREŚLONYCH
ZASTOSOWAŃ.W celu uzyskania bliższych informacji sięgnij do
Powszechnej Licencji Publicznej GNU.

Z pewnością wraz z niniejszym programem otrzymałeś też egzemplarz
Powszechnej Licencji Publicznej GNU(GNU General Public License);
jeśli nie - napisz do Free Software Foundation, Inc., 59 Temple
Place, Fifth Floor, Boston, MA  02110 - 1301  USA
*/

#define GLM_FORCE_RADIANS
#define GLM_FORCE_SWIZZLE

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <iostream>
#include <regex>

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>

#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"

#include "loadobj.h"

struct board_pos {
	unsigned int x;
	unsigned int y;

	bool operator==(const board_pos& o) const {
		return x == o.x && y == o.y;
	}

	bool operator<(const board_pos& o)  const {
		return x < o.x || (x == o.x && y < o.y);
	}
};


struct chess_move {
	unsigned int index;
	int type;
	union {
		struct {
			board_pos from;
			board_pos to;
		};

		struct  {
			board_pos pos;
			char piece;
		};
	};
};


board_pos strToBoardPos(std::string pos) {
	return { (unsigned int) (toupper(pos[0]) - 65), (unsigned int) (pos[1] - 49) };
}





std::map<char, std::string> piece_names = {
	{ 'p', "pawn" },
	{ 'r', "rook" },
	{ 'n', "knight" },
	{ 'b', "bishop" },
	{ 'k', "king" },
	{ 'q', "queen" }
};


std::vector<chess_move> loadGameFromFile(const std::string path) {

	std::ifstream file(path);
	std::string line;

	std::regex pattern("^(\\d*) (\\S*) (\\S*)(?: (\\S*))?$");
	std::vector<chess_move> moves;

	while (std::getline(file, line)) {
		std::smatch result;
		if (std::regex_match(line, result, pattern)) {

			unsigned int index = std::stoi(result.str(1));
			std::string type_str = result.str(2);
			std::string word1 = result.str(3);
			std::string word2 = result.str(4);

			chess_move m;
			m.index = index;

			if (type_str == std::string("new")) {

				m.type = 0;
				m.pos = strToBoardPos(word1);
				m.piece = word2[0];

				moves.push_back(m);
			}
			else if (type_str == std::string("mov")) {

				m.type = 1;
				m.from = strToBoardPos(word1);
				m.to = strToBoardPos(word2);

				moves.push_back(m);
			}
			else if (type_str == std::string("del")) {

				m.type = 2;
				m.pos = strToBoardPos(word1);

				moves.push_back({ index, 2, strToBoardPos(word1) });
			}

			//chess_move m = { st/*d::stoi(result.str(1)), result.str(2), result.str(3), result.str(4) };
			//moves.push_back(m);*/
		}
		else
			std::cout << "read board moves error\n";
	}
	
	return moves;
}

glm::vec2 posToVec(std::string pos) {
	return glm::vec2(toupper(pos[0]) - 65, pos[1] - 49);
}

float speed_x=0;
float speed_y=0;

float angle_x = 0;
float angle_y = 0;

float aspectRatio=1;

float last_frame_time;

unsigned int vertexbuffer2d;
unsigned int vertexbuffer2dtextures;
unsigned int indeciesbuffer;

std::vector<chess_move> moves = loadGameFromFile("game1.txt");;
unsigned int current_move_index = 0;

ShaderProgram *sp;
ShaderProgram* sp2d;

glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, -14.0f);
glm::vec3 camera_mov_speed = glm::vec3(0);

unsigned int CreateBuffer(GLenum type, int size, const void* data, GLenum usage) {
	unsigned int buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(type, buffer);
	glBufferData(type, size, data, usage);
	return buffer;
}

class GameObject {
private:
	unsigned int vertices_buffer;
	unsigned int uvs_buffer;
	unsigned int normals_buffer;

public:

	unsigned int vertex_count;

	GameObject(std::string path) {
		std::vector<glm::vec3> vertices;
		std::vector<glm::vec2> uvs;
		std::vector<glm::vec3> normals;

		loadOBJ(path, vertices, uvs, normals);

		vertex_count = vertices.size();

		vertices_buffer = CreateBuffer(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
		uvs_buffer = CreateBuffer(GL_ARRAY_BUFFER, sizeof(uvs[0]) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
		normals_buffer = CreateBuffer(GL_ARRAY_BUFFER, sizeof(normals[0]) * normals.size(), &normals[0], GL_STATIC_DRAW);
	}

	void BindAll(ShaderProgram *shader) {
		glBindBuffer(GL_ARRAY_BUFFER, vertices_buffer);
		glEnableVertexAttribArray(shader->a("vertex"));
		glVertexAttribPointer(shader->a("vertex"), 3, GL_FLOAT, false, sizeof(float) * 3, (const void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, uvs_buffer);
		glEnableVertexAttribArray(shader->a("texCoord"));
		glVertexAttribPointer(shader->a("texCoord"), 2, GL_FLOAT, false, sizeof(float) * 2, (const void*)0);

		glBindBuffer(GL_ARRAY_BUFFER, normals_buffer);
		glEnableVertexAttribArray(shader->a("normal"));
		glVertexAttribPointer(shader->a("normal"), 3, GL_FLOAT, false, sizeof(float) * 3, (const void*)0);
	}

	void UnbindAll(ShaderProgram* shader) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDisableVertexAttribArray(shader->a("vertex"));
		glDisableVertexAttribArray(shader->a("texCoord"));
		glDisableVertexAttribArray(shader->a("normal"));
	}
};

std::map<std::string, GameObject> objects;
std::map<std::string, unsigned int> textures;

class GameEntity {
private:
	GameObject *obj;
	unsigned int texture;
public:
	float v_angle = 0.0f;
	float scale = 1.0f;
	glm::vec3 position;

	GameEntity(GameObject* object, unsigned int object_texture, float scale, float v_angle, glm::vec3 position) {
		obj = object;
		texture = object_texture;
		this->scale = scale;
		this->v_angle = v_angle;
		this->position = position;
	}

	void Draw(ShaderProgram* shader, glm::mat4 model = glm::mat4(1.0f)) {
		glm::mat4 M_temp = glm::translate(model, glm::vec3(-position.x, position.y, position.z));
		M_temp = glm::scale(M_temp, glm::vec3(scale));
		M_temp = glm::rotate(M_temp, v_angle, glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M_temp));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glUniform1i(shader->u("tex"), 0);

		obj->BindAll(shader);
		glDrawArrays(GL_TRIANGLES, 0, obj->vertex_count);
	}
};

class GameBoard {
private:
	std::map<board_pos, GameEntity*> board;
	
	float velocity = 1.2f;
	board_pos moving_from = { 0, 0 };
	board_pos moving_to = { 0, 0 };

public:
	bool moving = false;

	GameBoard() {

		for (unsigned int i = 0; i < 8; ++i)
		board.insert({ {i, 1}, new GameEntity(&objects.at("pawn"),   textures.at("white_pawn"),   2.0f, 0.0f,   glm::vec3((int)i, 0, 1)) });
		board.insert({ {0, 0}, new GameEntity(&objects.at("rook"),   textures.at("white_rook"),   2.0f, 0.0f,   glm::vec3(0, 0, 0)) });
		board.insert({ {1, 0}, new GameEntity(&objects.at("knight"), textures.at("white_knight"), 2.0f, 0.0f,   glm::vec3(1, 0, 0)) });
		board.insert({ {2, 0}, new GameEntity(&objects.at("bishop"), textures.at("white_bishop"), 2.0f, 0.0f,   glm::vec3(2, 0, 0)) });
		board.insert({ {3, 0}, new GameEntity(&objects.at("queen"),  textures.at("white_queen"),  2.0f, 0.0f,   glm::vec3(3, 0, 0)) });
		board.insert({ {4, 0}, new GameEntity(&objects.at("king"),   textures.at("white_king"),   2.0f, PI / 2, glm::vec3(4, 0, 0)) });
		board.insert({ {5, 0}, new GameEntity(&objects.at("bishop"), textures.at("white_bishop"), 2.0f, 0.0f,   glm::vec3(5, 0, 0)) });
		board.insert({ {6, 0}, new GameEntity(&objects.at("knight"), textures.at("white_knight"), 2.0f, 0.0f,   glm::vec3(6, 0, 0)) });
		board.insert({ {7, 0}, new GameEntity(&objects.at("rook"),   textures.at("white_rook"),   2.0f, 0.0f,   glm::vec3(7, 0, 0)) });

		for (unsigned int i = 0; i < 8; ++i)
		board.insert({ {i, 6}, new GameEntity(&objects.at("pawn"),   textures.at("black_pawn"),   2.0f, PI,		 glm::vec3((int)i, 0, 6)) });
		board.insert({ {0, 7}, new GameEntity(&objects.at("rook"),   textures.at("black_rook"),   2.0f, PI,		 glm::vec3(0, 0, 7)) });
		board.insert({ {1, 7}, new GameEntity(&objects.at("knight"), textures.at("black_knight"), 2.0f, PI,		 glm::vec3(1, 0, 7)) });
		board.insert({ {2, 7}, new GameEntity(&objects.at("bishop"), textures.at("black_bishop"), 2.0f, PI,		 glm::vec3(2, 0, 7)) });
		board.insert({ {3, 7}, new GameEntity(&objects.at("queen"),  textures.at("black_queen"),  2.0f, PI,		 glm::vec3(3, 0, 7)) });
		board.insert({ {4, 7}, new GameEntity(&objects.at("king"),   textures.at("black_king"),   2.0f, PI*1.5f, glm::vec3(4, 0, 7)) });
		board.insert({ {5, 7}, new GameEntity(&objects.at("bishop"), textures.at("black_bishop"), 2.0f, PI,		 glm::vec3(5, 0, 7)) });
		board.insert({ {6, 7}, new GameEntity(&objects.at("knight"), textures.at("black_knight"), 2.0f, PI,		 glm::vec3(6, 0, 7)) });
		board.insert({ {7, 7}, new GameEntity(&objects.at("rook"),   textures.at("black_rook"),   2.0f, PI,		 glm::vec3(7, 0, 7)) });


	}

	void Draw(ShaderProgram* shader, glm::mat4 model = glm::mat4(1.0f)) {
		for (auto& pair : board)
			pair.second->Draw(shader, model);
	}

	void Update(float delta_time) {
		if (not moving)
			return;

		GameEntity* current_piece = board.at(moving_from);
		glm::vec2 destination = glm::vec2(moving_to.x, moving_to.y);
		glm::vec2 start = glm::vec2(moving_from.x, moving_from.y);
		glm::vec2 path = destination - current_piece->position.xz;
		glm::vec2 move_vector = glm::normalize(path) * delta_time * velocity;
		float progress = glm::length(current_piece->position.xz - start) / glm::length(destination - start);
		current_piece->position.y = sin(progress * PI) * glm::length(destination - start) / 4;

		if (glm::length(path) > glm::length(move_vector)) {
			current_piece->position.xz += move_vector;
		}
		else {
			current_piece->position.xz = destination;
			current_piece->position.y = 0;
			board.erase(moving_from);
			if (board.find(moving_to) != board.end()) {
				this->Remove(moving_to);
			}
			board.insert({ moving_to, current_piece });
			moving = false;
		}
	}

	void Move(board_pos from, board_pos to) {
		moving = true;
		moving_from = from;
		moving_to = to;
	}

	void Remove(board_pos pos) {
		delete board.at(pos);
		board.erase(pos);
	};

	void Place(board_pos pos, char piece) {
		std::string piece_name = piece_names.at(tolower(piece));
		bool color = islower(piece);
		
		unsigned int texture = textures.at((color ? "white_" : "black_") + piece_name);
		float v_angle = (color ? 0 : PI) + (tolower(piece) == 'k' ? PI / 2 : 0);

		board.insert({ pos, new GameEntity(&objects.at(piece_name), texture, 2.0f, v_angle, glm::vec3(pos.x, 0, pos.y)) });
	}
};

std::vector<GameEntity> entities;
GameBoard* board;

unsigned int readTexture(const char* filename) {
	unsigned int tex;
	glActiveTexture(GL_TEXTURE0);

	
	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, filename);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (unsigned char*)image.data());

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	return tex;
}


void error_callback(int error, const char* description) {
	fputs(description, stderr);
}

void keyCallback(GLFWwindow* window,int key,int scancode,int action,int mods) {
    if (action==GLFW_PRESS) {
        if (key==GLFW_KEY_LEFT) speed_x=-PI/2;
        if (key==GLFW_KEY_RIGHT) speed_x=PI/2;
        if (key==GLFW_KEY_UP) speed_y=PI/2;
        if (key==GLFW_KEY_DOWN) speed_y=-PI/2;
		if (key == GLFW_KEY_W) camera_mov_speed.z = 1;
		if (key == GLFW_KEY_A) camera_mov_speed.x = -1;
		if (key == GLFW_KEY_S) camera_mov_speed.z = -1;
		if (key == GLFW_KEY_D) camera_mov_speed.x = 1;
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) speed_x=0;
        if (key==GLFW_KEY_RIGHT) speed_x=0;
        if (key==GLFW_KEY_UP) speed_y=0;
        if (key==GLFW_KEY_DOWN) speed_y=0;
		if (key == GLFW_KEY_W) camera_mov_speed.z = 0;
		if (key == GLFW_KEY_A) camera_mov_speed.x = 0;
		if (key == GLFW_KEY_S) camera_mov_speed.z = 0;
		if (key == GLFW_KEY_D) camera_mov_speed.x = 0;
    }
}

void windowResizeCallback(GLFWwindow* window,int width,int height) {
    if (height==0) return;
    aspectRatio=(float)width/(float)height;
    glViewport(0, 0, width, height);
}

//Procedura inicjująca
void initOpenGLProgram(GLFWwindow* window) {
	//************Tutaj umieszczaj kod, który należy wykonać raz, na początku programu************
	glClearColor(0.5f, 0.5f, 0.5f, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	sp=new ShaderProgram("v_shiny2.glsl",NULL,"f_shiny2.glsl");
	sp2d = new ShaderProgram("v_2d.glsl", NULL, "f_2d.glsl");

	std::vector<glm::vec2> vertices = {
		{0, 0},
		{200, 0},
		{0, 100},
		//{200, 0},
		//{0, 100},
		{200, 100}
	};

	std::vector<glm::vec2> textureCoords = {
		{0, 0},
		{1, 0},
		{0, 1},
		//{1, 0},
		//{0, 1},
		{1, 1}
	};

	std::vector<unsigned int> indecies = { 0, 1, 2, 1, 2, 3 };

	vertexbuffer2d = CreateBuffer(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);\
	vertexbuffer2dtextures = CreateBuffer(GL_ARRAY_BUFFER, sizeof(textureCoords[0]) * textureCoords.size(), &textureCoords[0], GL_STATIC_DRAW);
	indeciesbuffer = CreateBuffer(GL_ELEMENT_ARRAY_BUFFER, sizeof(indecies[0]) * indecies.size(), &indecies[0], GL_STATIC_DRAW);

	std::string pieces[] = { "pawn", "rook", "knight", "bishop", "queen", "king" };

	for (const std::string& piece : pieces) {
		std::cout << "Loading " + piece + " object..." << std::endl;
		objects.insert({ piece, GameObject("objects/" + piece + ".obj") });
	}

	for (const std::string& piece : pieces) {
		std::cout << "Loading white_" + piece + " texture..." << std::endl;

		std::string white_texture = "textures/white_" + piece + ".png";
		textures.insert({ "white_" + piece, readTexture(white_texture.c_str()) });

		std::cout << "Loading black_" + piece + " texture..." << std::endl;

		std::string black_texture = "textures/black_" + piece + ".png";
		textures.insert({ "black_" + piece, readTexture(black_texture.c_str()) });
	}

	objects.insert({ "board", GameObject("objects/board.obj") });
	textures.insert({ "board", readTexture("textures/board_low.png") });
	textures.insert({ "logo", readTexture("textures/logo.png") });

	board = new GameBoard();

	std::cout << "Init finished" << std::endl;
}


//Zwolnienie zasobów zajętych przez program
void freeOpenGLProgram(GLFWwindow* window) {
    //************Tutaj umieszczaj kod, który należy wykonać po zakończeniu pętli głównej************

    delete sp;
}

// temporary piece of code
void place_piece(glm::mat4 model, ShaderProgram* shader, std::string piece, bool white, glm::vec2 pos) {
	glm::mat4 M_temp = glm::translate(model, glm::vec3(-pos.x, 0.0f, pos.y));
	M_temp = glm::scale(M_temp, glm::vec3(2.0f));
	M_temp = glm::rotate(M_temp, (white ? 0 : PI) + (piece == "king" ? PI/2 : 0), glm::vec3(0.0f, 1.0f, 0.0f));
	glUniformMatrix4fv(shader->u("M"), 1, false, glm::value_ptr(M_temp));


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures.at((white ? "white_" : "black_") + piece));
	glUniform1i(shader->u("tex"), 0);

	GameObject current_obj = objects.at(piece);
	current_obj.BindAll(shader);
	glDrawArrays(GL_TRIANGLES, 0, current_obj.vertex_count);
}

//Procedura rysująca zawartość sceny
void drawScene(GLFWwindow* window, float delta_time) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera_pos += camera_mov_speed * delta_time * 8.0f;

	glm::mat4 V=glm::lookAt(
		 camera_pos,
         glm::vec3( 0.0f,  0.0f,  0.0f),
         glm::vec3(0.0f,1.0f,0.0f)); //Wylicz macierz widoku

    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 50.0f); //Wylicz macierz rzutowania

    glm::mat4 M=glm::mat4(1.0f);

	angle_y += speed_y * delta_time;
	angle_x += speed_x * delta_time;

	M=glm::rotate(M, angle_y,glm::vec3(1.0f, 0.0f, 0.0f));
	M=glm::rotate(M, angle_x,glm::vec3(0.0f, 1.0f, 0.0f));

    sp->use();


    glUniformMatrix4fv(sp->u("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(sp->u("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M));

	{	
		glm::mat4 M_temp = glm::scale(M, glm::vec3(4.685959f));
		M_temp = glm::translate(M_temp, glm::vec3(0.0f, -0.083624f, 0.0f));
		glUniformMatrix4fv(sp->u("M"), 1, false, glm::value_ptr(M_temp));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textures.at("board"));
		glUniform1i(sp->u("tex"), 0);
		
		objects.at("board").BindAll(sp);
		glDrawArrays(GL_TRIANGLES, 0, objects.at("board").vertex_count);
	}

	glm::mat4 M_start = glm::scale(M, glm::vec3(1.0f));
	M_start = glm::translate(M_start, glm::vec3(3.5f, 0.0f, -3.5f));

	board->Draw(sp, M_start);


	if (current_move_index < moves.size() && not board->moving) {
		chess_move current_move = moves.at(current_move_index);
		switch (current_move.type) {
		case 0:
			board->Place(current_move.pos, current_move.piece);
			break;
		case 1:
			board->Move(current_move.from, current_move.to);
			break;
		case 2:
			board->Remove(current_move.pos);
		}
		current_move_index++;
	}
	board->Update(delta_time);



	
	sp2d->use();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textures.at("logo"));
	glUniform1i(sp2d->u("tex"), 0);



	int width, height;
	glfwGetWindowSize(window, &width, &height);
	glm::vec2 screen = glm::vec2(width, height);
	glUniform1f(sp2d->u("width"), (float)width);
	glUniform1f(sp2d->u("height"), (float)height);
	
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2d);
	glEnableVertexAttribArray(sp2d->a("vertex"));
	glVertexAttribPointer(sp2d->a("vertex"), 2, GL_FLOAT, false, sizeof(float) * 2, (void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer2dtextures);
	glEnableVertexAttribArray(sp2d->a("texCoord"));
	glVertexAttribPointer(sp2d->a("texCoord"), 2, GL_FLOAT, false, sizeof(float) * 2, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indeciesbuffer);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)0);

	//glDrawArrays(GL_TRIANGLES, 0, 6);


    glfwSwapBuffers(window);
}


int main(void)
{

	GLFWwindow* window; //Wskaźnik na obiekt reprezentujący okno

	glfwSetErrorCallback(error_callback);//Zarejestruj procedurę obsługi błędów

	if (!glfwInit()) { //Zainicjuj bibliotekę GLFW
		fprintf(stderr, "Nie można zainicjować GLFW.\n");
		exit(EXIT_FAILURE);
	}

	window = glfwCreateWindow(500, 500, "OpenGL", NULL, NULL);  //Utwórz okno 500x500 o tytule "OpenGL" i kontekst OpenGL.

	if (!window) //Jeżeli okna nie udało się utworzyć, to zamknij program
	{
		fprintf(stderr, "Nie można utworzyć okna.\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwMakeContextCurrent(window); //Od tego momentu kontekst okna staje się aktywny i polecenia OpenGL będą dotyczyć właśnie jego.
	glfwSwapInterval(1); //Czekaj na 1 powrót plamki przed pokazaniem ukrytego bufora

	if (glewInit() != GLEW_OK) { //Zainicjuj bibliotekę GLEW
		fprintf(stderr, "Nie można zainicjować GLEW.\n");
		exit(EXIT_FAILURE);
	}

	initOpenGLProgram(window); //Operacje inicjujące

	//Główna pętla


	last_frame_time = glfwGetTime();
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
		float current_frame = glfwGetTime();
		float delta_time = current_frame - last_frame_time;
		last_frame_time = current_frame;
	
		drawScene(window, delta_time); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
