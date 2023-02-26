/*********
  
ADDITIONAL FEATURES: The program counts the number of targets that were hit during the game and displays it at the end screen as the number of cats that were fed, the timer
stops when the barrel stops to fire the fish to target and than resumes after the target is hit, the background color changes when the barrel stops moving and 
it turns back to the original after the target is hit, when the barrel stops to throw a fish at the cat a heart shape appears in the background.
*********/

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <GL/glut.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#define WINDOW_WIDTH  660
#define WINDOW_HEIGHT 500

#define TIMER_PERIOD  16 // Period for the timer.
#define TIMER_ON         1 // 0:disable timer, 1:enable timer

#define D2R 0.0174532

/* Global Variables for Template File */
bool up = false, down = false, right = false, left = false, leftButton = false;
int  winWidth, winHeight; // current Window width and height

bool stopTime = false, //boolean variable to check if the timer stopped or not
	 mode = false, //boolean variable to determine if the game started or if the initial screen will be shown
	 lineactive = true; //boolean variable to check if the barrel will move or stay still

float r, g, b, //variables to assign random rgb colors to targets
	  timer = 3600, //the amount of seconds that the game will be going on (1 minute)
	  dx, dy, //amount of change in x and amount of change in y values that will be calculated in order to find the direction that the fires will follow
	  xP, yP, //variables that will be assigned as random generated numbers, they will be used to draw randomly placed targets (cat heads)
	  calculatedAngle; //the angle between random cats and the central player in degrees

int count = 0,//variable that counts the number of cats that were fed with fish
speed = 5; // the speed that the fired object will move with



typedef struct { //to store the starting positions of a shape
	int x, y;
}point_t;

typedef struct { //to store data about the central player that has a barrel and hits the targets
	point_t pos;
	int angle;
	float r;
}player_t;

typedef struct { //to store data about fire
	point_t pos;
	int angle;
	bool active;
}fire_t;

player_t p = { {0,0},45,30 }; //initial data of the central player which doesn't move and has a spinning barrel 

fire_t fr = { {0, 0}, 0, false }; //initial data of the firing shape

//
// to draw circle, center at (x,y)
// radius r
//
void circle(int x, int y, int r)
{
#define PI 3.1415
	float angle;
	glBegin(GL_POLYGON);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void circle_wire(int x, int y, int r)
{
#define PI 3.1415
	float angle;

	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 100; i++)
	{
		angle = 2 * PI * i / 100;
		glVertex2f(x + r * cos(angle), y + r * sin(angle));
	}
	glEnd();
}

void print(int x, int y, const char* string, void* font)
{
	int len, i;

	glRasterPos2f(x, y);
	len = (int)strlen(string);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, string[i]);
	}
}

// display text with variables.
// vprint(-winWidth / 2 + 10, winHeight / 2 - 20, GLUT_BITMAP_8_BY_13, "ERROR: %d", numClicks);
void vprint(int x, int y, void* font, const char* string, ...)
{
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);

	int len, i;
	glRasterPos2f(x, y);
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutBitmapCharacter(font, str[i]);
	}
}

// vprint2(-50, 0, 0.35, "00:%02d", timeCounter);
void vprint2(int x, int y, float size, const char* string, ...) {
	va_list ap;
	va_start(ap, string);
	char str[1024];
	vsprintf_s(str, string, ap);
	va_end(ap);
	glPushMatrix();
	glTranslatef(x, y, 0);
	glScalef(size, size, 1);

	int len, i;
	len = (int)strlen(str);
	for (i = 0; i < len; i++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
	}
	glPopMatrix();
}

void drawCenterChar(player_t tp) {
	glColor3ub(112, 8, 74); //center circle which is purple
	circle(tp.pos.x, tp.pos.y, tp.r);
	glLineWidth(3);
	glColor3f(1, 1, 1);
	glBegin(GL_LINES); //spinning barrel
	glVertex2f(tp.pos.x, tp.pos.y);
	glVertex2f(tp.pos.x + 40 * cos(tp.angle * D2R), tp.pos.y + 40 * sin(tp.angle * D2R));
	glEnd();
	glColor3ub(112, 8, 74);
	circle(tp.pos.x, tp.pos.y, 20);
	glColor3f(1, 1, 1);
	vprint(tp.pos.x - 12, tp.pos.y - 5, GLUT_BITMAP_8_BY_13, "%d", tp.angle);
}
void resetCat() { //function to reset the randomly located targets (cat heads)
	xP = (rand() % (280 - (-280)) + (-280));
	yP = (rand() % (220 - (-220)) + (-220));

	if (xP <= 100 && xP >= -100) {
		xP = (rand() % (280 - (-280)) + (-280));
		yP = (rand() % (220 - (-220)) + (-220));
	}

	if (yP <= 100 && yP >= -100) {
		xP = (rand() % (280 - (-280)) + (-280));
		yP = (rand() % (220 - (-220)) + (-220));
	}
	if (atan2(yP, xP) / D2R == 10 || atan2(yP, xP) / D2R == 99 || atan2(yP, xP) / D2R == 259 || atan2(yP, xP) / D2R == 349 || atan2(yP, xP) / D2R == 101 || atan2(yP, xP) / D2R == 261) {
		xP = (rand() % (280 - (-280)) + (-280));
		yP = (rand() % (220 - (-220)) + (-220));
	}
}

void drawCat() { //function to draw the cat heads
	glColor3f(0, 0, 0); //head background (outline part which is black)
	circle(xP - 5, yP, 30);
	circle(xP + 5, yP, 30);
	glBegin(GL_TRIANGLES);
	glVertex2f(xP - 5, yP + 30);
	glVertex2f(xP + 5, yP + 30);
	glVertex2f(xP, yP);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2f(xP - 5, yP - 30);
	glVertex2f(xP + 5, yP - 30);
	glVertex2f(xP, yP);
	glEnd();

	glBegin(GL_TRIANGLES); //ears background (outline part which is black)
	glVertex2f(xP - 27, yP + 10);
	glVertex2f(xP - 5, yP + 17);
	glVertex2f(xP - 22, yP + 50);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2f(xP + 27, yP + 10);
	glVertex2f(xP + 5, yP + 17);
	glVertex2f(xP + 22, yP + 50);
	glEnd();

	glColor3ub(r, g, b);//head front part (randomly colored)
	circle(xP - 5, yP, 27);
	circle(xP + 5, yP, 27);
	glBegin(GL_TRIANGLES);
	glVertex2f(xP - 5, yP + 27);
	glVertex2f(xP + 5, yP + 27);
	glVertex2f(xP, yP);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2f(xP - 5, yP - 27);
	glVertex2f(xP + 5, yP - 27);
	glVertex2f(xP, yP);
	glEnd();

	glBegin(GL_TRIANGLES);//ears front (randomly colored)
	glVertex2f(xP - 25, yP + 5);
	glVertex2f(xP - 6, yP + 15);
	glVertex2f(xP - 21, yP + 43);
	glEnd();
	glBegin(GL_TRIANGLES);
	glVertex2f(xP + 25, yP + 5);
	glVertex2f(xP + 6, yP + 15);
	glVertex2f(xP + 21, yP + 43);
	glEnd();

	glColor3f(0, 0, 0);//eyes
	circle(xP - 15, yP - 7, 4);
	circle(xP - 15, yP - 5, 4);
	circle(xP + 15, yP - 7, 4);
	circle(xP + 15, yP - 5, 4);

	glBegin(GL_TRIANGLES);//nose
	glVertex2f(xP - 4, yP - 13);
	glVertex2f(xP + 4, yP - 13);
	glVertex2f(xP, yP - 16);
	glEnd();

}
void colorCat() { //function to determine the random values of r,g,b to randomly color the cat heads
	r = rand() % 256;
	g = rand() % 256;
	b = rand() % 256;
}

void startBackground() { //function to draw the basic backgrounf without extra explanation
	glColor3ub(148, 246, 246);
	glBegin(GL_POLYGON);
	glVertex2f(-450, 400);
	glVertex2f(450, 400);
	glColor3ub(255, 153, 204);
	glVertex2f(450, -350);
	glVertex2f(-450, -350);
	glEnd();
}

void drawFire(fire_t f) { //function to draw the fires
	if (!lineactive) {
		glColor3ub(70, 105, 118);
		circle(f.pos.x, f.pos.y, 10);
		circle(f.pos.x + 3, f.pos.y, 10);
		glBegin(GL_TRIANGLES);
		glVertex2f(f.pos.x + 5, f.pos.y);
		glVertex2f(f.pos.x + 20, f.pos.y + 5);
		glVertex2f(f.pos.x + 20, f.pos.y - 5);
		glEnd();

		glColor3f(0, 0, 0);
		circle(f.pos.x - 5, f.pos.y + 3, 2);

	}
}
void changeBackg() { //changes the background color everytime a cat is fed 
	glColor3ub(255, 204, 255);
	glBegin(GL_POLYGON);
	glVertex2f(-450, 350);
	glVertex2f(450, 350);
	//glColor3ub(255, 255, 153);
	glVertex2f(450, -350);
	glVertex2f(-450, -350);
	glEnd();

	glColor3ub(255, 153, 204);
	circle(40, 50, 50);
	circle(-40, 50, 50);

	glBegin(GL_TRIANGLES);
	glVertex2f(87, 30);
	glVertex2f(-87, 30);
	glVertex2f(0, -80);
	glEnd();
}

//
// To display onto window using OpenGL commands
//
void display() {
	//
	// clear window to black
	//
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);


	switch (mode) {
	case false: //diaplays the initial screen
		startBackground();
		glColor3f(0, 0, 0);
		vprint(-110, 0, GLUT_BITMAP_8_BY_13, "Press Space To Start The Game");
		vprint(-320, 230, GLUT_BITMAP_8_BY_13, "Sera Altuner");
		break;
	case true: //displays the screen with the game
		if (lineactive == false)
			changeBackg();
		else
			startBackground();
		drawCat();
		drawCenterChar(p);
		drawFire(fr);
		glColor3f(0, 0, 0);
		vprint(-320, 230, GLUT_BITMAP_8_BY_13, "Sera Altuner");
		vprint(-320, -230, GLUT_BITMAP_8_BY_13, "Time Left: %.2f", timer / 60);
		calculatedAngle = atan2(yP, xP) / D2R;
		
		
		break;
	}

	if (stopTime) { //displays the ending screen 
		startBackground();
		glColor3f(0, 0, 0);
		vprint(-320, 230, GLUT_BITMAP_8_BY_13, "Sera Altuner");
		vprint(-110, 0, GLUT_BITMAP_8_BY_13, "%d Cats Were Fed", count);
		vprint(-110, -20, GLUT_BITMAP_8_BY_13, "Press <Escape> To Exit From The Game");
	}
	glutSwapBuffers();
}


//
// key function for ASCII charachters like ESC, a,b,c..,A,B,..Z
//
void onKeyDown(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);
	
	if (key == ' ') { //with the press on the space bar, it makes the mode true and starts the game
		mode = true;
		lineactive = true;
		resetCat();
		colorCat();
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

void onKeyUp(unsigned char key, int x, int y)
{
	// exit when ESC is pressed.
	if (key == 27)
		exit(0);

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyDown(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = true; break;
	case GLUT_KEY_DOWN: down = true; break;
	case GLUT_KEY_LEFT: left = true; break;
	case GLUT_KEY_RIGHT: right = true; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// Special Key like GLUT_KEY_F1, F2, F3,...
// Arrow Keys, GLUT_KEY_UP, GLUT_KEY_DOWN, GLUT_KEY_RIGHT, GLUT_KEY_RIGHT
//
void onSpecialKeyUp(int key, int x, int y)
{
	// Write your codes here.
	switch (key) {
	case GLUT_KEY_UP: up = false; break;
	case GLUT_KEY_DOWN: down = false; break;
	case GLUT_KEY_LEFT: left = false; break;
	case GLUT_KEY_RIGHT: right = false; break;
	}

	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// When a click occurs in the window,
// It provides which button
// buttons : GLUT_LEFT_BUTTON , GLUT_RIGHT_BUTTON
// states  : GLUT_UP , GLUT_DOWN
// x, y is the coordinate of the point that mouse clicked.
//
void onClick(int button, int stat, int x, int y)
{
	// Write your codes here.
	
	// to refresh the window it calls display() function
	glutPostRedisplay();
}

//
// This function is called when the window size changes.
// w : is the new width of the window in pixels.
// h : is the new height of the window in pixels.
//
void onResize(int w, int h)
{
	winWidth = w;
	winHeight = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-w / 2, w / 2, -h / 2, h / 2, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	display(); // refresh window.
}

void onMoveDown(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function   
	glutPostRedisplay();
}

// GLUT to OpenGL coordinate conversion:
//   x2 = x1 - winWidth / 2
//   y2 = winHeight / 2 - y1
void onMove(int x, int y) {
	// Write your codes here.



	// to refresh the window it calls display() function
	glutPostRedisplay();
}

#if TIMER_ON == 1
void onTimer(int v) {

	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
	// Write your codes here.

	if (lineactive == true) { //if statement controlling the barrel spin
		if (mode && !stopTime)
			if (p.angle >= 0 && p.angle <= 360)
				if (calculatedAngle >= 0)
					if (p.angle - calculatedAngle >= 0 && p.angle - calculatedAngle <= 180)
						p.angle--;
					else
						p.angle++;
				else
					if (p.angle - (calculatedAngle + 360) >= 0 && p.angle - (calculatedAngle + 360) <= 180)
						p.angle--;
					else
						p.angle++;
			if (p.angle > 360)
				p.angle = 0;
			if (p.angle < 0)
				p.angle += 360;


		if (mode && timer >= 0) //checking if the time is over or not
			timer--;
		else if (mode && timer < 0)
			stopTime = true;
	}

	if (calculatedAngle >= 0) {  //calculates the angle between the center and the random target and checks if its equal to the barrel's angle at the time
		if ((int)calculatedAngle == (int)p.angle) 
			lineactive = false;
	}
	else
		if ((int)calculatedAngle + 360 == (int)p.angle)
			lineactive = false;
	  
	dx = (int)xP - (int)p.pos.x; //calculates the amount of change in x and y when going from the center of the player and the randomly located target
	dy = (int)yP - (int)p.pos.y;
	fr.angle = atan2(dy, dx) / D2R; //finds the angle between the target and the center in degrees

	 
	if (!lineactive) { //increases the x and y values of the fired object to make them move
		fr.pos.x += speed * cos(fr.angle * D2R);
		fr.pos.y += speed * sin(fr.angle * D2R);
	}
	if (fr.pos.x + 15 > xP - 55 && fr.pos.x + 10 < xP + 45 && fr.pos.y + 10 > yP - 45 && fr.pos.y + 10 < yP + 65) { //checks if the fired object hit the target or not 
		resetCat();																									//if its hit it displays another target
		colorCat();
		fr.pos.x = 0;
		fr.pos.y = 0;
		lineactive = true;
		count++;
	}
	
	// to refresh the window it calls display() function
	glutPostRedisplay(); // display()

}
#endif

void Init() {

	// Smoothing shapes
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	srand(time(NULL));

}

void main(int argc, char* argv[]) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	//glutInitWindowPosition(100, 100);
	glutCreateWindow("Angular Firing Game by Sera Altuner");

	glutDisplayFunc(display);
	glutReshapeFunc(onResize);

	//
	// keyboard registration
	//
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecialKeyDown);

	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecialKeyUp);

	//
	// mouse registration
	//
	glutMouseFunc(onClick);
	glutMotionFunc(onMoveDown);
	glutPassiveMotionFunc(onMove);

#if  TIMER_ON == 1
	// timer event
	glutTimerFunc(TIMER_PERIOD, onTimer, 0);
#endif

	Init();

	glutMainLoop();
}
