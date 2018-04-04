#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <Windows.h>


#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>
#include <GL/GLU.h>
#include <GL/glut.h>
#include <GL/wglew.h>

using namespace std;

GLuint g_programID;


GLuint VertexBufferID[2];
GLuint VertexArrayID;

vector<float> g_vertices;

GLuint LoadShaders(const char* vertex_file_path, const char* fragment_file_path)
{
	//create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	GLint Result = GL_FALSE;
	int InfoLogLength;

	//Read the vertex shader code from the file
	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path, ios::in);
	if (VertexShaderStream.is_open())
	{
		string Line = "";
		while (getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	//Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const* VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	//Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength != 0) {
		vector<char> VertexShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);
	}
	//Read the fragment shader code from the file
	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path, ios::in);
	if (FragmentShaderStream.is_open())
	{
		string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	//Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const* FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	//Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength != 0) {
		vector<char> FragmentShaderErrorMessage(InfoLogLength);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);
	}
	//Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength != 0) {
		vector<char> ProgramErrorMessage(max(InfoLogLength, int(1)));
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);
	}
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

void myMouse(int button, int state, int x, int y) {

	static float r = 1.0;
	static float g = 0.0;
	static float b = 0.0;

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		float nx, ny;

		nx = 2.0 * (float)x / (float)479 - 1.0;
		ny = -2.0 * (float)y / (float)479 + 1.0;

		//GLuint posLoc;

		//posLoc = glGetAttribLocation(g_programID, "pos");
		//glVertexAttrib3f(posLoc, nx, ny, 0.0);


		r = (rand() % 100) / 99.f;
		g = (rand() % 100) / 99.f;
		b = (rand() % 100) / 99.f;
		//GLuint colLoc = glGetUniformLocation(g_programID, "mCol");
		//glUniform3f(colLoc, r, g, b);
		g_vertices.push_back(nx);
		g_vertices.push_back(ny);
		g_vertices.push_back(0.0);
		g_vertices.push_back(r);
		g_vertices.push_back(g);
		g_vertices.push_back(b);

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*g_vertices.size(), g_vertices.data(), GL_DYNAMIC_DRAW);

		glutPostRedisplay(); //새로 다시 그리기
	}
}

void renderScene(void)
{
	//입력받은 곳에 출력하기 ( vertex shader와 통신)
	//	GLuint posLoc;
	//	posLoc = glGetAttribLocation(g_programID, "pos");
	//	glVertexAttrib3f(posLoc, -0.5, 0.5, 0.0);

	//Clear all pixels
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//Let's draw something here
	//Draw Line
	GLuint posLoc;

	posLoc = glGetAttribLocation(g_programID, "pos");
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[0]);
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(0));
	glEnableVertexAttribArray(posLoc);


	//Color
	//GLuint colLoc = glGetUniformLocation(g_programID, "mCol");
	//glUniform3f(colLoc, 0.0, 1.0, 0.0);
	GLuint colLoc = glGetAttribLocation(g_programID, "iColor");
	glVertexAttribPointer(colLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(colLoc);


	//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[1]);
	//GLuint sizeLoc;
	//sizeLoc = glGetAttribLocation(g_programID, "pSize");
	////glVertexAttribPointer(sizeLoc, 1, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*4, (void*)(sizeof(GLfloat)*3));
	//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[1]);
	//glVertexAttribPointer(sizeLoc, 1, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	//glEnableVertexAttribArray(sizeLoc);


	//glDrawArrays(GL_POINTS, 0, 2);
	if (g_vertices.size() != 0)
		glDrawArrays(GL_LINES, 0, (g_vertices.size() / 6));
	//Double buffer
	glutSwapBuffers();
}


void init()
{
	//initilize the glew and check the errors.

	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
	}

	//select the background color

	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

}

void main(int argc, char **argv)
{
	//init GLUT and create Window
	//initialize the GLUT
	glutInit(&argc, argv);
	//GLUT_DOUBLE enables double buffering (drawing to a background buffer while the other buffer is displayed)
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	//These two functions are used to define the position and size of the window. 
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(480, 480);
	//This is used to define the name of the window.
	glutCreateWindow("Simple OpenGL Window");

	//call initization function
	init();

	//1.
	//Generate VAO
	//GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	//3. 
	g_programID = LoadShaders("VertexShader.txt", "FragmentShader.txt");
	glUseProgram(g_programID);

	//float vertices[] = { -0.5, -0.3, 0.0, 0.2, 0.7, 0.0 };
	//float sizes[] = { 10.0, 20.0, 30.0 };
	float vtxData[] = { -0.5, -0.3, 0.0, 0.3, 0.2, 0.1, 0.2, 0.7, 0.0, 0.7, 0.8, 0.9 };
	//Create Vertex Buffer
	//GLuint VertexBufferID;
	glGenBuffers(2, VertexBufferID);
	//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[0]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vtxData, GL_DYNAMIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[1]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2, sizes, GL_DYNAMIC_DRAW);


	glutMouseFunc(myMouse);	//마우스 콜백 등록

	glutDisplayFunc(renderScene);

	//enter GLUT event processing cycle
	glutMainLoop();

	glDeleteVertexArrays(1, &VertexArrayID);

}

