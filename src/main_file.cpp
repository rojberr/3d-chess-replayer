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

#include "constants.h"
#include "lodepng.h"
#include "shaderprogram.h"
#include "myCube.h"
#include "myTeapot.h"

#include "loadobj.h"

float speed_x=0;
float speed_y=0;
float aspectRatio=1;

ShaderProgram *sp;

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
    }
    if (action==GLFW_RELEASE) {
        if (key==GLFW_KEY_LEFT) speed_x=0;
        if (key==GLFW_KEY_RIGHT) speed_x=0;
        if (key==GLFW_KEY_UP) speed_y=0;
        if (key==GLFW_KEY_DOWN) speed_y=0;
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
	glClearColor(0, 0, 0, 1);
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowSizeCallback(window, windowResizeCallback);
	glfwSetKeyCallback(window, keyCallback);

	sp=new ShaderProgram("v_textured.glsl",NULL,"f_textured.glsl");

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
	textures.insert({ "board", readTexture("textures/board.png") });

	//objects.insert({ "pawn", GameObject("objects/pawn.obj") });
	//objects.insert({ "rook", GameObject("objects/rook.obj") });
	//objects.insert({ "knight", GameObject("objects/knight.obj") });
	//objects.insert({ "bishop", GameObject("objects/bishop.obj") });
	//objects.insert({ "queen", GameObject("objects/queen.obj") });
	//objects.insert({ "king", GameObject("objects/king.obj") });


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
void drawScene(GLFWwindow* window,float angle_x,float angle_y) {
	//************Tutaj umieszczaj kod rysujący obraz******************l
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 V=glm::lookAt(
         glm::vec3( 0.0f,  0.0f, -14.0f),
         glm::vec3( 0.0f,  0.0f,  0.0f),
         glm::vec3(0.0f,1.0f,0.0f)); //Wylicz macierz widoku

    glm::mat4 P=glm::perspective(50.0f*PI/180.0f, aspectRatio, 0.01f, 50.0f); //Wylicz macierz rzutowania

    glm::mat4 M=glm::mat4(1.0f);
	M=glm::rotate(M,angle_y,glm::vec3(1.0f, 0.0f, 0.0f));
	M=glm::rotate(M,angle_x,glm::vec3(0.0f, 1.0f, 0.0f));

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


	for (int i = 0; i < 8; ++i)
		place_piece(M_start, sp, "pawn", 1, glm::vec2(i, 1));
	place_piece(M_start, sp, "rook", 1, glm::vec2(0, 0));
	place_piece(M_start, sp, "knight", 1, glm::vec2(1, 0));
	place_piece(M_start, sp, "bishop", 1, glm::vec2(2, 0));
	place_piece(M_start, sp, "queen", 1, glm::vec2(3, 0));
	place_piece(M_start, sp, "king", 1, glm::vec2(4, 0));
	place_piece(M_start, sp, "bishop", 1, glm::vec2(5, 0));
	place_piece(M_start, sp, "knight", 1, glm::vec2(6, 0));
	place_piece(M_start, sp, "rook", 1, glm::vec2(7, 0));


	for (int i = 0; i < 8; ++i)
		place_piece(M_start, sp, "pawn", 0, glm::vec2(i, 6));
	place_piece(M_start, sp, "rook", 0, glm::vec2(0, 7));
	place_piece(M_start, sp, "knight", 0, glm::vec2(1, 7));
	place_piece(M_start, sp, "bishop", 0, glm::vec2(2, 7));
	place_piece(M_start, sp, "queen", 0, glm::vec2(3, 7));
	place_piece(M_start, sp, "king", 0, glm::vec2(4, 7));
	place_piece(M_start, sp, "bishop", 0, glm::vec2(5, 7));
	place_piece(M_start, sp, "knight", 0, glm::vec2(6, 7));
	place_piece(M_start, sp, "rook", 0, glm::vec2(7, 7));

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
	float angle_x = 0;
	float angle_y = 0;
	glfwSetTime(0);
	while (!glfwWindowShouldClose(window)) //Tak długo jak okno nie powinno zostać zamknięte
	{
        angle_x += speed_x * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        angle_y += speed_y * glfwGetTime(); //Zwiększ/zmniejsz kąt obrotu na podstawie prędkości i czasu jaki upłynał od poprzedniej klatki
        glfwSetTime(0); //Zeruj timer
		drawScene(window, angle_x, angle_y); //Wykonaj procedurę rysującą
		glfwPollEvents(); //Wykonaj procedury callback w zalezności od zdarzeń jakie zaszły.
	}

	freeOpenGLProgram(window);

	glfwDestroyWindow(window); //Usuń kontekst OpenGL i okno
	glfwTerminate(); //Zwolnij zasoby zajęte przez GLFW
	exit(EXIT_SUCCESS);
}
