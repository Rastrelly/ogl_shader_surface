#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"

#include <iostream>
#include <vector>
#include <chrono>

using namespace std;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
GLFWwindow *window = NULL;
Shader *shad;
float *verts;
unsigned int VBO;
unsigned int VAO;

//model, projection and view matrices
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 proj = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);

glm::vec3 lightPos(30.0f, 30.0f, -20.0f);
glm::vec3 ambLightColor(1.0f, 0.1f, 0.2f);

float camang = 45.0f; float camdist = 50.0f;
float camx = -50.0f; float camy = -50.0f; float camz = 20.0f;
float winw = 800.0f; float winh = 600.0f;
float dt = 0;

float lightOrbit = 50.0f;
float lightAng = 0.0f;
float lightSpd = 30.0f;

std::chrono::steady_clock::time_point lastUpdate = std::chrono::steady_clock::now();

vector <float> dataArray = {};
int dw = 0; int dh = 0;
int nx = 0; int ny = 0;
int funcType = 0;

int initGLFW();
float calcFunc(int func, float x, float y);
void makeDataArray(float w, float h, float step, vector <float> &dArr);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void PrepShaders(const char* vertexPath, const char* fragmentPath);
void setproj();
void drawSurface();
void drawTriangle(vector<float>vertices);
float getdeltatime();


int main()
{
    printf("Initializing program\n");
	printf("Step 1 - Input W:\n");
	dw = 0;
	cin >> dw;
	printf("Step 2 - Input H:\n");
	dh = 0;
	cin >> dh;
	printf("Step 3 - Input STEP:\n");
	float dstep = 0;
	cin >> dstep;
	printf("Step 4 - Input function type [0 .. 4]:\n");
	cin >> funcType;
	printf("Step 5 - Generating Data Array\n");
	makeDataArray(dw,dh,dstep,dataArray);

	verts = new float[dataArray.size()];
	for (int i = 0; i < dataArray.size(); i++)
	{
		verts[i] = dataArray[i];
	}

	printf("         Data Array Generated: %d Entities (%d x %d)\n", dataArray.size(), nx, ny);


	printf("Step 6 - Initializing GLFW\n");
	if (initGLFW() == -1)
	{
		printf("ERROR in GLFW initialization/n");
	}

	printf("Step 7 - Prepping Shaders\n");
	PrepShaders("vert_shader.gls","frag_shader.gls");
	shad->use();
	printf("Step 8 - Setting Up Model View\n");
	setproj();

	printf("Step 9 - Initiating Main Loop\n");
	while (!glfwWindowShouldClose(window))
	{

		dt = getdeltatime();

		processInput(window);

		setproj();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		drawSurface();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

	printf("Program complete.\n");
	system("pause");

	return 0;
}

//makes defined function calculation
float calcFunc(int func, float x, float y)
{
	//place the function definition here
	
	//return 0;
	switch (func) {
		case 0: {return pow(0.2*x,2)+2*sin(0.2*y) - 15; break; }
		case 1: {return sin(x) + cos(y); break; }
		case 2: {return x + sin(y); break; }
		case 3: {return cos(x) + y; break; }
		case 4: {return 0.3*pow(x,3) + 2*sin(y); break; }
		default: {return 0; break; }
	}
	
	return 0;
}


//generates data array for further rendering
void makeDataArray(float w, float h, float step, vector <float> &dArr)
{
	int triType = 0; //0 - normal, 1 - inverted
	int triStep = 0; //0, 1, 2

	vector <float> dArrTmp = {};

	int nst = ((w - 1)*(h - 1)*3)/step;
	int i = 0;

	dArr.clear();
	nx = 0; ny = 0;

	bool getnx = true;
	float x1 = -w / 2;
	float y1 = -h / 2;
	float x2 =  w / 2;
	float y2 =  h / 2;
	float cx = x1; float cy = y1; float cz = 0;

	//generating triangles of direct and inverted types
	while (true)
	{
		float ccx = 0; float ccy = 0; float ccz = 0;
		if (triType == 0)
		{
			if (triStep == 0)
			{
				ccx = cx;
				ccy = cy;
			}
			if (triStep == 1)
			{
				ccx = cx+step;
				ccy = cy;
			}
			if (triStep == 2)
			{
				ccx = cx;
				ccy = cy+step;
			}
		}
		if (triType == 1)
		{
			if (triStep == 0)
			{
				ccx = cx+step;
				ccy = cy;
			}
			if (triStep == 1)
			{
				ccx = cx + step;
				ccy = cy + step;
			}
			if (triStep == 2)
			{
				ccx = cx;
				ccy = cy + step;
			}
		}

		ccz = calcFunc(funcType, ccx, ccy);

		//each point calculated here will become
		//a vertex when getting to rendering
		dArrTmp.push_back(ccx);
		dArrTmp.push_back(ccy);
		dArrTmp.push_back(ccz);

		//now we need to calculate normals if a triangle is complete
		glm::vec3 normal = glm::vec3(1.0f);
		if (triStep == 2)
		{
			glm::vec3 p1 = glm::vec3(dArrTmp[0], dArrTmp[1], dArrTmp[2]);
			glm::vec3 p2 = glm::vec3(dArrTmp[3], dArrTmp[4], dArrTmp[5]);
			glm::vec3 p3 = glm::vec3(dArrTmp[6], dArrTmp[7], dArrTmp[8] );
			normal = glm::normalize(glm::cross(p3 - p1, p2 - p1));
			
			//dump temp array and fill true array with normal and vertex data 3 - 3
			int nc = 0;
			for (int ii = 0; ii < 9; ++ii)
			{
				dArr.push_back(dArrTmp[ii]);

				if (nc == 2)
				{
					dArr.push_back(normal.x);
					dArr.push_back(normal.y);
					dArr.push_back(normal.z);
				}
				nc++;
				if (nc > 2) nc = 0;
			}
			dArrTmp.clear();
		}

		//increase counter
		i++;
		//increase step
		triStep++;
		if (triStep > 2)
		{
			triStep = 0;
			//define triangle type
			triType++;
			if (triType > 1)
			{
				triType = 0;
				//define core X point
				cx += step;
				if (cx > (x2-step))
				{
					cx = x1;
					cy += step;
					if (cy > (y2 - step)) break;
				}	
			}
		}
	}
}

int initGLFW()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Shader Surface Maker", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glDepthFunc(GL_LESS);
	glDepthRange(-100.0f, 100.0f);
	glEnable(GL_DEPTH_TEST);

}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camdist += 15 * dt;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camdist -= 15 * dt;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camang += 15 * dt;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camang -= 15 * dt;
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
		camz += 15 * dt;
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
		camz -= 15 * dt;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
	winw = (float)width;
	winh = (float)height;
}

void PrepShaders(const char* vertexPath, const char* fragmentPath)
{
	delete(shad);
	shad = new Shader(vertexPath, fragmentPath);
	shad->use();
}

void setproj()
{
	
	//camera orientation
	camx = camdist * cos(camang*3.14 / 180);
	camy = camdist * sin(camang*3.14 / 180);


	//modelview scaling for elements
	float scl = 50/(((float)dw+(float)dh)/2);
	model = glm::scale(glm::mat4(1.0f), glm::vec3(scl));

	//projection view
	proj = glm::perspective(glm::radians(75.0f), (GLfloat)winw / (GLfloat)winh, 1.0f, 100.0f);
	
	//viewmode (camera effect)
	view = glm::lookAt(glm::vec3(camx, camy, camz),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));
	view = glm::translate(view, glm::vec3(0.0f, 0.0f, 1.0f));

	//assigning uniforms
	shad->setMatrix4f("model", model);
	shad->setMatrix4f("projection", proj);
	shad->setMatrix4f("view", view);

	//light manipulation
	lightOrbit = ((float)dw + (float)dh);
	lightAng += lightSpd * dt;
	lightPos.x = lightOrbit * sin(lightAng*3.14/180);
	lightPos.y = lightOrbit * cos(lightAng*3.14 / 180);
	shad->setVector3f("lightColor",ambLightColor.r,ambLightColor.g,ambLightColor.b);
	shad->setVector3f("lightPos", lightPos.x, lightPos.y, lightPos.z);
}

void drawSurface()
{	
	shad->setVector4f("nColor", 1.0f, 0.0f, 0.0f, 1.0f);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * dataArray.size(),
		verts,
		GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glDrawArrays(GL_TRIANGLES, 0, (int)dataArray.size() / 6);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void drawTriangle(vector<float>vertices)
{
	shad->setVector4f("nColor", 1.0f, 0.0f, 0.0f, 1.0f);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(float) * vertices.size(),
		verts,
		GL_STATIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glDrawArrays(GL_TRIANGLES, 0, (int)dataArray.size() / 6);

	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

float getdeltatime()
{
	auto now = std::chrono::steady_clock::now();
	float deltaTime = std::chrono::duration_cast<std::chrono::microseconds>(now - lastUpdate).count() / 1000000.0f;
	lastUpdate = now;
	return deltaTime;
}