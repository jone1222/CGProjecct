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

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 640
#define LINE_MARGIN 5

using namespace std;

GLuint g_programID;


GLuint VertexBufferID[2];
GLuint VertexArrayID;


vector<float> point_vertices;
vector<float> line_vertices;
vector<vector<float>> lineS_vertices;


vector<float> *target_vertices = &point_vertices;

enum MyMode {
	Points,
	Lines,
	LineStrips
};
enum MyColor {
	RED,
	BLUE,
	GREEN,
	BLACK,
	RANDOM
};

int DrawMode = Points;		// 0 : Points / 1 : Lines / 2 : LineStrips

int ColorMode = RANDOM;

bool isSelect = false;

bool isUniform = false;

void ScreenToNorm(float *x, float *y);
void NormToScreen(float* nx, float* ny);

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
void swapColor(float* r, float* g, float* b) {
	switch (ColorMode) {
	case RED: *r = 1.0; *g = 0.0; *b = 0.0;  break;
	case BLUE: *r = 0.0; *g = 0.0; *b = 1.0; break;
	case GREEN: *r = 0.0; *g = 1.0; *b = 0.0; break;
	case BLACK: *r = 0.0; *g = 0.0; *b = 0.0; break;
	case RANDOM: *r = (rand() % 100) / 99.f; *g = (rand() % 100) / 99.f; *b = (rand() % 100) / 99.f; break;
	default: break;
	}
}

void swapAllColor(bool doChange) {
	/*
	for (int i = 3; i < vec.size(); i+= 6)
		swapColor(&vec.at(i), &vec.at(i + 1), &vec.at(i + 2));
	*/
	
	GLuint inUseLoc;
	inUseLoc = glGetAttribLocation(g_programID, "UseUniform");
	if (doChange)
		glVertexAttrib1f(inUseLoc, 1.0);
	else
		glVertexAttrib1f(inUseLoc, 0.0);

	isUniform = doChange;

	GLuint colLoc = glGetUniformLocation(g_programID, "mCol");
	float r, g, b;
	swapColor(&r, &g, &b);
	glUniform3f(colLoc, r, g, b);
}

void ColorSelectEvent(int x, int y , float* r, float* g, float* b) {
	/* 
		RED: 0~20
		GREEN 20~40
		BLUE 40~60
		BLACK 60~80
	*/
	if (y < 20) {
		if (x >= 0 && x <= 20)
			ColorMode = RED;
		else if( x > 20 && x <= 40 )
			ColorMode = GREEN;
		else if (x > 40 && x <= 60)
			ColorMode = BLUE;
		else if (x > 60 && x <= 80)
			ColorMode = BLACK;

		swapColor(r, g, b);
	}

}

bool isPtOnLine(float pt_x, float pt_y, float start_x, float start_y, float end_x, float end_y) {
	POINT pts[4];
	pts[0].x = start_x; pts[0].y = start_y - 5;
	pts[1].x = start_x; pts[1].y = start_y + 5;
	pts[2].x = end_x; pts[2].y = end_y + 5;
	pts[3].x = end_x; pts[3].y = end_y - 5;
	HRGN rgn = CreatePolygonRgn(pts, 4, ALTERNATE);
	return PtInRegion(rgn,pt_x, pt_y);
}

void myMouse(int button, int state, int x, int y) {

	static float r = 1.0;
	static float g = 0.0;
	static float b = 0.0;

	
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		if (!isSelect) {
			if ((x >= 0 && x <= 120) && (y <= 25)) {
				ColorSelectEvent(x, y, &r, &g, &b);
				return;
			}
			float nx, ny;

			nx = 2.0 * (float)x / (float)(SCREEN_WIDTH - 1.0) - 1.0;
			ny = -2.0 * (float)y / (float)(SCREEN_HEIGHT - 1.0) + 1.0;



			swapColor(&r, &g, &b);
			//GLuint colLoc = glGetUniformLocation(g_programID, "mCol");
			//glUniform3f(colLoc, r, g, b);


			target_vertices->push_back(nx);
			target_vertices->push_back(ny);
			target_vertices->push_back(0.0);
			target_vertices->push_back(r);
			target_vertices->push_back(g);
			target_vertices->push_back(b);

			glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[0]);
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*target_vertices->size(), target_vertices->data(), GL_DYNAMIC_DRAW);
		}
		else {
			bool isFound = false;
			float nx = x;
			float ny = y;
			//float x_margin = 5 / (float)SCREEN_WIDTH;
			//float y_margin = 5 / (float)SCREEN_HEIGHT;

			//NormToScreen(&nx, &ny);
			//Select Mode
			for (int i = 0; i < point_vertices.size(); i += 6) {
				float pt_x = point_vertices.at(i);
				float pt_y = point_vertices.at(i + 1);
				NormToScreen(&pt_x, &pt_y);
				HRGN PtRgn = CreateRectRgn(pt_x - 5, pt_y - 5, 
					pt_x + 5, pt_y + 5);
				if (PtInRegion(PtRgn, nx, ny)){
					isFound = true;
					MessageBox(NULL, "Point Selected", "SELECT EVENT", MB_OK | MB_ICONEXCLAMATION);
					swapColor(&r, &g, &b);
					point_vertices.at(i + 3) = r;	point_vertices.at(i + 4) = g;	point_vertices.at(i + 5) = b;
					break;
				}
			}
			if (!isFound) {
				for (int i = 0; i < line_vertices.size(); i += 12) {
					float start_x = line_vertices.at(i);
					float start_y = line_vertices.at(i+1);
					float end_x = line_vertices.at(i+6);
					float end_y = line_vertices.at(i+7);
					NormToScreen(&start_x, &start_y);
					NormToScreen(&end_x, &end_y);

					if (isPtOnLine(nx, ny, start_x, start_y, end_x, end_y)) {
						isFound = true;
						MessageBox(NULL, "Line Selected", "SELECT EVENT", MB_OK | MB_ICONEXCLAMATION);
						swapColor(&r, &g, &b);
						line_vertices.at(i + 3) = r;	line_vertices.at(i + 4) = g;	line_vertices.at(i + 5) = b;
						line_vertices.at(i + 9) = r;	line_vertices.at(i + 10) = g;	line_vertices.at(i + 11) = b;
						break;
					}

				}
			}
			if (!isFound) {
				for (int i = 0; i < lineS_vertices.size(); i++) {
					if (lineS_vertices.at(i).size() < 12)
						continue;

					for (int j = 0; j < lineS_vertices.at(i).size() - 6; j += 6) {
						float start_x = lineS_vertices.at(i).at(j);
						float start_y = lineS_vertices.at(i).at(j + 1);
						float end_x = lineS_vertices.at(i).at(j + 6);
						float end_y = lineS_vertices.at(i).at(j + 7);
						NormToScreen(&start_x, &start_y);
						NormToScreen(&end_x, &end_y);

						if (isPtOnLine(nx, ny, start_x, start_y, end_x, end_y)) {
							isFound = true;
							MessageBox(NULL, "LineStrip Selected", "SELECT EVENT", MB_OK | MB_ICONEXCLAMATION);
							swapColor(&r, &g, &b);
							
							for (int k = 3; k < lineS_vertices.at(i).size(); k += 6) {
								lineS_vertices.at(i).at(k) = r; lineS_vertices.at(i).at(k + 1) = g; lineS_vertices.at(i).at(k + 2) = b;
							}

							break;
						}

					}
				}
			}
		}
		glutPostRedisplay(); //새로 다시 그리기
	}
	else if ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN)) {
		if (DrawMode == 2) {
			DrawMode -= 2;
			MessageBox(NULL, "Point Drawing Mode", "POINT / LINE / STRIP CHANGE EVENT", MB_OK | MB_ICONEXCLAMATION);
			target_vertices = &point_vertices;
		}
		else if ( DrawMode == 0 ) {
			DrawMode++;
			MessageBox(NULL, "Line Drawing Mode", "POINT/ LINE / STRIP CHANGE EVENT", MB_OK | MB_ICONEXCLAMATION);
			if ((line_vertices.size() / 6) % 2 == 1)
				for(int i = 0 ; i < 6 ; i++)
					line_vertices.pop_back();
			target_vertices = &line_vertices;
		}
		else {
			//DrawMode = 1
			DrawMode++;
			MessageBox(NULL, "Line Strip Drawing Mode", "POINT/ LINE / STRIP CHANGE EVENT", MB_OK | MB_ICONEXCLAMATION);
			lineS_vertices.push_back(vector<float>());
			target_vertices = &lineS_vertices.at( lineS_vertices.size() - 1 );
		}

		glutPostRedisplay();
	}
	else if((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN)) {
		//유니폼으로 할수 있는 방법은 없는가?
		int doChange = MessageBox(NULL, "YES : CHANGE ALL COLOR / CANCEL : Reset ALL COLOR", "CHANGE ALL COLOR MSGBOX", MB_OKCANCEL | MB_ICONEXCLAMATION);
		if (doChange == IDOK) {
			swapAllColor(true);
		}
		else {
			swapAllColor(false);
		}

		glutPostRedisplay();
	}
}

void myKeyBoard(unsigned char key, int x, int y) {
	if (key == 'S' || key == 's') {
		isSelect = !isSelect;
		if (isSelect)
			MessageBox(NULL, "Select Mode", "Drawing / Selection CHANGE EVENT", MB_OK | MB_ICONEXCLAMATION);
		else
			MessageBox(NULL, "Drawing Mode", "Drawing / Selection CHANGE EVENT", MB_OK | MB_ICONEXCLAMATION);
	}

	
}
//
void ScreenToNorm(float* x, float* y) {
	float nx, ny;

	nx = 2.0 * (float)*x / (float)(SCREEN_WIDTH - 1.0) - 1.0;
	ny = -2.0 * (float)*y / (float)(SCREEN_HEIGHT - 1.0) + 1.0;
	
	*x = nx;
	*y = ny;
}
void NormToScreen(float* nx, float* ny) {
	float x, y;

	//nx = (2.0 * (float)*x) / (float)(SCREEN_WIDTH - 1.0) - 1.0;
	//ny = (-2.0 * (float)*y) / (float)(SCREEN_HEIGHT - 1.0) + 1.0;
	x = (*nx + 1.0) * (float)(SCREEN_WIDTH - 1.0) / 2.0;
	y = (*ny - 1.0) * (float)(SCREEN_HEIGHT - 1.0) / (-2.0);

	*nx = x;
	*ny = y;
}

void DrawColorSelectionArea() {
	//색상 선택 영역 그리기
	
	
	float vtxData[] = {
		10, 10, 0.0, 1.0, 0.0, 0.0,
		30, 10, 0.0, 0.0, 1.0, 0.0,
		50, 10, 0.0, 0.0, 0.0, 1.0,
		70, 10, 0.0, 0.0, 0.0, 0.0,
	};
	for (int i = 0; i < 24; i+=6) {
		ScreenToNorm(&vtxData[i], &vtxData[i+1]);
	}

	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[1]);

	GLuint posLoc = glGetAttribLocation(g_programID, "pos");
	glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(0));
	glEnableVertexAttribArray(posLoc);


	//Color
	//GLuint colLoc = glGetUniformLocation(g_programID, "mCol");
	//glUniform3f(colLoc, 0.0, 1.0, 0.0);
	GLuint colLoc = glGetAttribLocation(g_programID, "iColor");
	glVertexAttribPointer(colLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(colLoc);

	GLuint sizeLoc;
	sizeLoc = glGetAttribLocation(g_programID, "pSize");
	glVertexAttrib1f(sizeLoc, 20.0);
	/*glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[1]);
	glVertexAttribPointer(sizeLoc, 1, GL_FLOAT, GL_FALSE, 0, (void*)(0));
	glEnableVertexAttribArray(sizeLoc);*/

	glBufferData(GL_ARRAY_BUFFER, sizeof(float)*4*6, vtxData, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_POINTS, 0, 4);
}

void renderScene(void)
{
	//입력받은 곳에 출력하기 ( vertex shader와 통신)
	//	GLuint posLoc;
	//	posLoc = glGetAttribLocation(g_programID, "pos");
	//	glVertexAttrib3f(posLoc, -0.5, 0.5, 0.0);

	//Clear all pixels
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//색상선택영역 그리기
	DrawColorSelectionArea();

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

	GLuint sizeLoc;
	sizeLoc = glGetAttribLocation(g_programID, "pSize");
	glVertexAttrib1f(sizeLoc, 10.0);
	


	//glDrawArrays(GL_POINTS, 0, 2);
	if (point_vertices.size() != 0 || line_vertices.size() != 0 || lineS_vertices.size() != 0 ) {

		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*point_vertices.size(), point_vertices.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_POINTS, 0, (point_vertices.size() / 6));		

		glBufferData(GL_ARRAY_BUFFER, sizeof(float)*line_vertices.size(), line_vertices.data(), GL_DYNAMIC_DRAW);
		glDrawArrays(GL_LINES, 0, (line_vertices.size() / 6));

		for (int i = 0; i < lineS_vertices.size(); i++) {
			glBufferData(GL_ARRAY_BUFFER, sizeof(float)*lineS_vertices.at(i).size() , lineS_vertices.at(i).data(), GL_DYNAMIC_DRAW);
			glDrawArrays(GL_LINE_STRIP, 0, (lineS_vertices.at(i).size() / 6));
		}
	}
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
	glutInitWindowSize(SCREEN_WIDTH, SCREEN_HEIGHT);
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
	
	//Create Vertex Buffer
	//GLuint VertexBufferID;
	glGenBuffers(2, VertexBufferID);
	//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[0]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vtxData, GL_DYNAMIC_DRAW);

	//glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID[1]);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2, sizes, GL_DYNAMIC_DRAW);


	glutMouseFunc(myMouse);	//마우스 콜백 등록
	glutKeyboardFunc(myKeyBoard);

	glutDisplayFunc(renderScene);

	//enter GLUT event processing cycle
	glutMainLoop();

	glDeleteVertexArrays(1, &VertexArrayID);

}

