#include <GL/glut.h>
#include <cstdlib>
#include <vector>

using namespace std;

// the parameters of the window
GLsizei winWidth = 450, winHeight = 450;

// these are the coordinates of the x-axis 
// we need to change them every time the window has changed its size and there fore they are global - 
// passing them as parameters in the functions will make the functions more unreadable
GLfloat xAxisXStart = 0, xAxisXEnd = winWidth, xAxisY = (winHeight / 2);

// these are the coordinates of the y-axis
GLfloat yAxisYStart = 0, yAxisYEnd = winHeight, yAxisX = (winWidth / 2);

// these boolean variables help us determine whether the user wants to see the lines or the control points
// he can hide/show them by pressing respectively C and P
GLboolean toDrawControlLines = GL_TRUE, toDrawControlPoints = GL_TRUE;

// this boolean varaible helps us determine whether or not the user wants to see the x(t) and y(t) functions
// he can hide/show them by pressing D
GLboolean toDrawFunctions = GL_FALSE;

// with these variables we can control the color of the Bezier curve
// by pressing 1, 2 or 3 on the keyboard, the user can change its color
GLfloat curveRedColor = 0.0f, curveBlueColor = 0.0f, curveGreenColor = 1.0f;

// with these variables we can control the color of the control lines
// by pressing 4, 5 or 6 on the keyboard, the user can change their color
GLfloat lineRedColor = 1.0f, lineBlueColor = 0.0f, lineGreenColor = 0.0f;

// these are the characters we shall display next to each coordinate axes' end
char coordinateAxes[] = { 'x', 'y', 'T', 'T' };

// these are the x coordinates of the characters above
int coordinateAxesTextXPos[] = { xAxisXEnd - 15, yAxisX + 15, xAxisXStart + 15, yAxisX + 15 };

// these are the y coordinates of the characters above
int coordinateAxesTextYPos[] = { xAxisY + 15, yAxisYEnd - 15, xAxisY + 15, yAxisYStart + 15 };

// we shall define a structure to represent a point instead of pair because that way the code looks
// much simpler and more readable
struct point
{
	GLfloat x, y;

	point(GLfloat x = 0.0, GLfloat y = 0.0) :x(x), y(y) {}
};

// this vector will hold the control points
vector<point> pts;

// this vector will hold the control points for x(t)
vector<point> xFunc;

// this vector will hold the control points for y(t)
vector<point> yFunc;

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);

	// enable anti-aliasing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_LINEAR);
	glEnable(GLUT_MULTISAMPLE);
}

// a simple function to draw a given point
void plotPoint(point bezCurvePt)
{
	glBegin(GL_POINTS);
	glVertex2f(bezCurvePt.x, bezCurvePt.y);
	glEnd();
	glFlush();
}

// a simple function to draw a line by two given points
void plotLine(point a, point b)
{
	glBegin(GL_LINES);
	glVertex2f(a.x, a.y);
	glVertex2f(b.x, b.y);
	glEnd();
	glFlush();
}

// this is where we draw the coordinate axes
void drawCoordinateSystem()
{
	glColor3f(1.0, 1.0, 1.0);

	// draw coordinate axes
	plotLine({ xAxisXStart, xAxisY }, { xAxisXEnd, xAxisY });
	plotLine({ yAxisX, yAxisYStart }, { yAxisX, yAxisYEnd });

	// draw right-most arrow
	plotLine({ xAxisXEnd, xAxisY }, { xAxisXEnd - 10, xAxisY + 10 });
	plotLine({ xAxisXEnd, xAxisY }, { xAxisXEnd - 10, xAxisY - 10 });

	// draw left-most arrow
	plotLine({ xAxisXStart, xAxisY }, { xAxisXStart + 10, xAxisY + 10 });
	plotLine({ xAxisXStart, xAxisY }, { xAxisXStart + 10, xAxisY - 10 });

	// draw top-most arrow
	plotLine({ yAxisX, yAxisYEnd }, { yAxisX + 10, yAxisYEnd - 10 });
	plotLine({ yAxisX, yAxisYEnd }, { yAxisX - 10, yAxisYEnd - 10 });

	// draw bottom-most arrow
	plotLine({ yAxisX, yAxisYStart }, { yAxisX + 10, yAxisYStart + 10 });
	plotLine({ yAxisX, yAxisYStart }, { yAxisX - 10, yAxisYStart + 10 });

	// display 'x', 'y', 'T' and 'T' at each end of the coordinate axes
	for (int i = 0; i < 4; i++)
	{
		glRasterPos2i(coordinateAxesTextXPos[i], coordinateAxesTextYPos[i]);
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, coordinateAxes[i]);
	}
}

/* when the user presses D button on the keyboard, then the program will need to draw the functions x(t)
and y(t), so we need to calculate the points for those functions */

// calculates the points of x(t) function
void computeXFuncPts()
{
	// we get the number of control points + 1
	int size = (int)pts.size() + 1;

	// number of points of x(t) function = number of control points
	xFunc.resize(size - 1);

	for (int i = 1; i <= size - 1; i++)
	{
		// for more information about this calculation check the documentation
		xFunc[i - 1].y = (winHeight / 2) - i * ((winHeight / 2) / size);

		// since this is x(t) the x coordinate stays the same
		xFunc[i - 1].x = pts[i - 1].x;
	}
}

// calculates the points of y(t) function
void computeYFuncPts()
{
	// we get the number of control points + 1
	int size = (int)pts.size() + 1;

	// number of points of y(t) function = number of control points
	yFunc.resize(size - 1);

	for (int i = 1; i <= size - 1; i++)
	{
		// for more information about this calculation check the documentation
		yFunc[i - 1].x = (winWidth / 2) - i * ((winWidth / 2) / size);

		// since this is y(t) the y coordinate stays the same
		yFunc[i - 1].y = pts[i - 1].y;
	}
}

// this is the most significant function in this project because this is where we calculate the points
// of the curve for some t
// for more info about how this function works check the documentation
point computeBezPt(int r, int i, double t, vector<point>& pts)
{
	if (r == 0)
		return pts[i];

	point p1 = computeBezPt(r - 1, i, t, pts);
	point p2 = computeBezPt(r - 1, i + 1, t, pts);

	point newPt;
	newPt.x = (1 - t) * p1.x + t * p2.x;
	newPt.y = (1 - t) * p1.y + t * p2.y;

	return newPt;
}

// this is where we draw the curve
void computeBezier(vector<point>& pts)
{
	glColor3f(curveRedColor, curveGreenColor, curveBlueColor);
	point previous = pts[0];
	point next;

	// t increases by 0.02 in order for the curve to look much smoother than for instance if it increased
	// by 0.2 (t goes from 0 to 1)
	for (float t = 0.0; t <= 1.0; t += 0.02)
	{
		next = computeBezPt((int)pts.size() - 1, 0, t, pts);
		plotLine(previous, next);
		previous = next;
	}
}

// this is where we draw the curve, the lines, the points and the coordinate system each time a new point
// is added or an existing one is deleted, or if the window is resized and therefore the control points
// and the curve points have new values
void drawBezier()
{
	glClear(GL_COLOR_BUFFER_BIT);

	// draw the coordinate axes first
	drawCoordinateSystem();

	// get the number of the control points
	int numPts = (int)pts.size();

	glLineWidth(1);

	// if we have any points to draw (note that they have to be more than 1) and the user has requested 
	// to see them, draw the control lines
	if (numPts > 1 && toDrawControlLines)
	{
		glColor3f(lineRedColor, lineGreenColor, lineBlueColor);
		for (int i = 0; i < numPts - 1; i++)
			plotLine(pts[i], pts[i + 1]);
	}

	glLineWidth(2);

	/* If the user has requested to see the control points, draw them.
	We draw them after the control lines in order to be on top of them.
	That way the points will be displayed. If we did it the other way around the points would have
	been underneath the control lines and even though they are drawn they would not be visible (or at
	least not clearly). */
	if (toDrawControlPoints)
	{
		glColor3f(0.0, 1.0, 1.0);
		glPointSize(6);
		for (int i = 0; i < numPts; i++)
			plotPoint(pts[i]);
	}

	// if there are any control points, then draw the Bezier curve
	if (numPts > 0)
		computeBezier(pts);

	// if there are any control points and the user has requested to see the x(t) and y(t) functions
	if (toDrawFunctions && numPts > 0)
	{
		// calculate the points of x(t)
		computeXFuncPts();

		// calculate the points of y(t) 
		computeYFuncPts();

		/* down below is the same pattern of drawing as when we drew the Bezier curve, its control lines
		and points */

		glLineWidth(1);

		if (numPts > 1 && toDrawControlLines)
		{
			glColor3f(lineRedColor, lineGreenColor, lineBlueColor);
			for (int i = 0; i < numPts - 1; i++)
				plotLine(xFunc[i], xFunc[i + 1]);
		}

		glLineWidth(2);

		if (toDrawControlPoints)
		{
			glColor3f(0.0, 1.0, 1.0);
			glPointSize(6);
			for (int i = 0; i < numPts; i++)
				plotPoint(xFunc[i]);
		}

		computeBezier(xFunc);

		glLineWidth(1);

		if (numPts > 1 && toDrawControlLines)
		{
			glColor3f(lineRedColor, lineGreenColor, lineBlueColor);
			for (int i = 0; i < numPts - 1; i++)
				plotLine(yFunc[i], yFunc[i + 1]);
		}

		glLineWidth(2);

		if (toDrawControlPoints)
		{
			glColor3f(0.0, 1.0, 1.0);
			glPointSize(6);
			for (int i = 0; i < numPts; i++)
				plotPoint(yFunc[i]);
		}

		computeBezier(yFunc);
	}

	glFlush();
}

// checks if the user has clicked in the first quadrant of the coordinate system
// note that the y coordinate we get from the mouse function is winHeight - y
bool isInFirstQuadrant(int x, int y)
{
	return (x >= winWidth / 2 && x <= winWidth) && (y >= winHeight / 2 && y <= winHeight);
}

// triggers when either mouse button is pressed
void mouse(int button, int state, int x, int y)
{
	// if the left button is pressed, add a control point to the vector of control points
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN && isInFirstQuadrant(x, winHeight - y))
	{
		point p;
		p.x = x;
		p.y = winHeight - y;
		pts.push_back(p);
	}
	// if the right button is pressed, remove the last control point if there is such
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN && isInFirstQuadrant(x, winHeight - y))
	{
		if (!pts.empty())
			pts.pop_back();
	}
}

// triggers when certain keys are pressed
void keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		// if key c is pressed, then the user wants to show/hide the control lines
	case 'c':
		toDrawControlLines = !toDrawControlLines;
		break;
		// if key p is pressed, then the user wants to show/hide the control points
	case 'p':
		toDrawControlPoints = !toDrawControlPoints;
		break;
		// if key d is pressed, then the user wants to show/hide the x(t) and y(t) functions
	case 'd':
		toDrawFunctions = !toDrawFunctions;
		break;
		// if key 1 is pressed, then change the red value of the curve color
	case '1':
		if (curveRedColor < 0.1)
			curveRedColor = 1;
		else
			curveRedColor -= 0.1;
		break;
		// if key 2 is pressed, then change the blue value of the curve color
	case '2':
		if (curveBlueColor < 0.1)
			curveBlueColor = 1;
		else
			curveBlueColor -= 0.1;
		break;
		// if key 3 is pressed, then change the green value of the curve color
	case '3':
		if (curveGreenColor < 0.1)
			curveGreenColor = 1;
		else
			curveGreenColor -= 0.1;
		break;
		// if key 4 is pressed, then change the red value of the line color
	case '4':
		if (lineRedColor < 0.1)
			lineRedColor = 1;
		else
			lineRedColor -= 0.1;
		break;
		// if key 5 is pressed, then change the blue value of the line color
	case '5':
		if (lineBlueColor < 0.1)
			lineBlueColor = 1;
		else
			lineBlueColor -= 0.1;
		break;
		// if key 6 is pressed, then change the green value of the line color
	case '6':
		if (lineGreenColor < 0.1)
			lineGreenColor = 1;
		else
			lineGreenColor -= 0.1;
		break;
	default:
		break;
	}

	/* draw the bezier curve, control points and lines, coordinate axes and the new change since we
	didn't change the curve at all and therefore if we don't do it now the user's wish will be
	carried out after he has added, removed a point or resized the window */
	drawBezier();
}

void update(GLint newWidth, GLint newHeight)
{
	GLfloat widthRatio = (GLfloat)newWidth / winWidth; // compute the change of the width
	GLfloat heightRatio = (GLfloat)newHeight / winHeight; // compute the change of the height
	winWidth = newWidth; // change the old width value to the new one
	winHeight = newHeight; // change the old height value to the new one

	// change the values for the coordinate axes to the new ones so that they draw according to the new width/height
	xAxisXEnd = winWidth;
	yAxisYEnd = winHeight;
	xAxisY = (winHeight / 2);
	yAxisX = (winWidth / 2);

	// update the position of the characters: x, y, T and T
	coordinateAxesTextXPos[0] = xAxisXEnd - 15;
	coordinateAxesTextXPos[1] = yAxisX + 15;
	coordinateAxesTextXPos[2] = xAxisXStart + 15;
	coordinateAxesTextXPos[3] = yAxisX + 15;
	coordinateAxesTextYPos[0] = xAxisY + 15;
	coordinateAxesTextYPos[1] = yAxisYEnd - 15;
	coordinateAxesTextYPos[2] = xAxisY + 15;
	coordinateAxesTextYPos[3] = yAxisYStart + 15;

	// compute the new coordinates of the Bezier points
	for (int i = 0; i < pts.size(); i++)
	{
		pts[i].x *= widthRatio; // multiply the old x value to the ratio of the change of the width
		pts[i].y *= heightRatio; // multiply the old y value to the ratio of the change of the height
	}
}

// this functions is called each time the window size is changed
void reshape(GLint newWidth, GLint newHeight)
{
	// set up the viewport to the new width and height
	glViewport(0, 0, (GLsizei)newWidth, (GLsizei)newHeight);

	// update the figures in order to draw the according to the new width and height
	update(newWidth, newHeight);

	glMatrixMode(GL_PROJECTION); // Switch to the projection matrix so that we can manipulate how our scene is viewed
	glLoadIdentity(); // Reset the projection matrix to the identity matrix so that we don't get any artifacts (cleaning up)
	gluOrtho2D(0.0, winWidth, 0.0, winHeight);
	glMatrixMode(GL_MODELVIEW); // Switch back to the model view matrix, so that we can start drawing shapes correctly
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT);
}

// redraw the curve whenever there is a change to it
void displayFunc()
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawBezier();

	glFlush();
}

int main(int argc, char **argv)
{
	pts.reserve(1000);
	glutInit(&argc, argv); // Initialize GLUT
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE); // Set display mode
	glutInitWindowPosition(300, 100); // Set top-left display-window position
	glutInitWindowSize(winWidth, winHeight); // Set display-window width and height
	glutCreateWindow("Bezier Curve"); // Creates display window

	init(); // Execute initialization procedure
	glutReshapeFunc(reshape);
	glutDisplayFunc(displayFunc); // Send graphics to display window
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);

	glutMainLoop(); // Display everything and wait
	return 0;
}
