
#include <iostream>
#include <cfloat>
#include <vector>

#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 


/* assimp include files. These three are usually needed. */
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "shaders/initShaders.h"

#include <string.h>
#include <map>

//#include "src/SOIL.h"

#ifndef BUFFER_OFFSET 
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
#ifndef MIN
#define MIN(x,y) (x<y?x:y)
#endif
#ifndef MAX
#define MAX(x,y) (y>x?y:x)
#endif

#define Helper_ColorIsAbsent(c)		((c.r == 0) && (c.g == 0) && (c.b == 0) && (c.a == 0))


bool controle = true;
using namespace std;

GLuint 	shaderAmbient,
shaderGouraud,
shaderGooch,
shaderPhong,
shader;

GLuint 	axisVBO[3];
GLuint 	meshVBO[3];
GLuint 	meshSize;

double  last;

vector<GLfloat> vboVertices;
vector<GLfloat> vboNormals;
vector<GLfloat> vboColors;
vector<GLfloat> vboCoords;



std::map<std::string, GLuint*> textureIdMap; // map image filenames to textureIds
GLuint*	textureIds = NULL; // pointer to texture Array



int winWidth = 600,
winHeight = 600;

// 
float
angleX = 0.0f,
angleY = 0.0f,
angleZ = 0.0f,

lmoveX = 0.0f,
lmoveY = 0.0f,

camMoveX = 0.0f,
camMoveY = 0.0f;

/* the global Assimp scene object */
const aiScene* scene = NULL;
GLuint scene_list = 0;
aiVector3D scene_min, scene_max, scene_center;



// Aplicando texturas

/// ***********************************************************************
/// **
/// ***********************************************************************

void get_bounding_box_for_node(const struct aiNode* nd,
	aiVector3D* min,
	aiVector3D* max,
	aiMatrix4x4* trafo
) {
	aiMatrix4x4 prev;
	unsigned int n = 0, t;

	prev = *trafo;
	aiMultiplyMatrix4(trafo, &nd->mTransformation);

	for (; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		for (t = 0; t < mesh->mNumVertices; ++t) {

			aiVector3D tmp = mesh->mVertices[t];
			aiTransformVecByMatrix4(&tmp, trafo);

			min->x = MIN(min->x, tmp.x);
			min->y = MIN(min->y, tmp.y);
			min->z = MIN(min->z, tmp.z);

			max->x = MAX(max->x, tmp.x);
			max->y = MAX(max->y, tmp.y);
			max->z = MAX(max->z, tmp.z);
		}
	}

	for (n = 0; n < nd->mNumChildren; ++n) {
		get_bounding_box_for_node(nd->mChildren[n], min, max, trafo);
	}
	*trafo = prev;
}

/// ***********************************************************************
/// **
/// ***********************************************************************

void color4_to_float4(const aiColor4D *c, float f[4])
{
	f[0] = c->r;
	f[1] = c->g;
	f[2] = c->b;
	f[3] = c->a;
}

/// ***********************************************************************
/// **
/// ***********************************************************************

void set_float4(float f[4], float a, float b, float c, float d)
{
	f[0] = a;
	f[1] = b;
	f[2] = c;
	f[3] = d;
}

/// ***********************************************************************
/// **
/// ***********************************************************************

void apply_material(const aiMaterial *mtl)
{
	float c[4];
	int ret1, ret2;
	aiColor4D diffuse;
	aiColor4D specular;
	aiColor4D ambient;
	aiColor4D emission;
	float shininess, strength;
	unsigned int max;	// changed: to unsigned

	set_float4(c, 0.8f, 0.8f, 0.8f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse)) color4_to_float4(&diffuse, c);

	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_SPECULAR, &specular)) color4_to_float4(&specular, c);

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);

	set_float4(c, 0.2f, 0.2f, 0.2f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_AMBIENT, &ambient)) color4_to_float4(&ambient, c);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, c);

	set_float4(c, 0.0f, 0.0f, 0.0f, 1.0f);
	if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_EMISSIVE, &emission)) color4_to_float4(&emission, c);

	glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, c);

	max = 1;
	ret1 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS, &shininess, &max);
	max = 1;
	ret2 = aiGetMaterialFloatArray(mtl, AI_MATKEY_SHININESS_STRENGTH, &strength, &max);
	if ((ret1 == AI_SUCCESS) && (ret2 == AI_SUCCESS))
	{
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess * strength);
	}
	else
	{
		glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 0.0f);
		set_float4(c, 0.0f, 0.0f, 0.0f, 0.0f);
		glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, c);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int traverseScene(const aiScene *sc, const aiNode* nd) {

	int totVertices = 0;
	GLuint prev_tex_id_idx = 0;

	/* draw all meshes assigned to this node */
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		const aiMaterial* mtl = scene->mMaterials[mesh->mMaterialIndex];
		
		if (mesh->HasTextureCoords(0))
		{
			// enable first texture as default
			prev_tex_id_idx = 0;
			glBindTexture(GL_TEXTURE_2D, textureIds[prev_tex_id_idx]);
		}

		apply_material(sc->mMaterials[mesh->mMaterialIndex]);

		if (!mesh->HasTextureCoords(0) && mesh->HasVertexColors(0))
			glEnable(GL_COLOR_MATERIAL);
		else
			glDisable(GL_COLOR_MATERIAL);
	
		
		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace* face = &mesh->mFaces[t];
		

			for (unsigned int i = 0; i < face->mNumIndices; i++) {
				const GLfloat vtx_def_col[4] = { 0.5f, 0.5f, 0.5f, 1 };
				int index = face->mIndices[i];
				
				if (mesh->HasTextureCoords(0))
				{
					// get current texture ID and check if need to enable new texture
					if (mesh->mTextureCoords[1][t].x != prev_tex_id_idx)
					{
						prev_tex_id_idx = mesh->mTextureCoords[1][t].x;
						glBindTexture(GL_TEXTURE_2D, textureIds[prev_tex_id_idx]);
					}
				}
			

				//texturas
				if ((mesh->mColors[1] != NULL) && !Helper_ColorIsAbsent(mesh->mColors[1][t]))// check if color set for face
					glColor4fv((GLfloat*)&mesh->mColors[1][t]);
				else if ((mesh->mColors[0] != NULL) && !Helper_ColorIsAbsent(mesh->mColors[0][index]))// check if color set for vertex
					glColor4fv((GLfloat*)&mesh->mColors[0][index]);
				else// default color for vertex.
					glColor4fv(vtx_def_col);///TODO: IME thru AI_*
				//
				// textures
				//
				if (mesh->HasTextureCoords(0))
				{
					glTexCoord2f(mesh->mTextureCoords[0][t * 3 + i].x, 1 - mesh->mTextureCoords[0][t * 3 + i].y);//mTextureCoords[channel][vertex]
				}

				//texturas

				if (mesh->mNormals != NULL) {
					vboNormals.push_back(mesh->mNormals[index].x);
					vboNormals.push_back(mesh->mNormals[index].y);
					vboNormals.push_back(mesh->mNormals[index].z);
				}
				vboVertices.push_back(mesh->mVertices[index].x);
				vboVertices.push_back(mesh->mVertices[index].y);
				vboVertices.push_back(mesh->mVertices[index].z);

				if (mesh->HasTextureCoords(0)){
			
			
					glTexCoord2f(mesh->mTextureCoords[0][index].x, 1 - mesh->mTextureCoords[0][index].y);

				}

				totVertices++;
			}
		}
	}

	for (unsigned int n = 0; n < nd->mNumChildren; ++n) {
		totVertices += traverseScene(sc, nd->mChildren[n]);
	}
	return totVertices;
}

/// ***********************************************************************
/// **
/// ***********************************************************************

void createVBOs(const aiScene *sc) {

	int totVertices = 0;
	cout << "Scene:	 	#Meshes 	= " << sc->mNumMeshes << endl;
	cout << "			#Textures	= " << sc->mNumTextures << endl;

	totVertices = traverseScene(sc, sc->mRootNode);

	cout << "			#Vertices	= " << totVertices << endl;
	cout << "			#vboVertices= " << vboVertices.size() << endl;
	cout << "			#vboColors= " << vboColors.size() << endl;
	cout << "			#vboNormals= " << vboNormals.size() << endl;

	glGenBuffers(4, meshVBO);

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[0]);

	glBufferData(GL_ARRAY_BUFFER, vboVertices.size() * sizeof(float),
		vboVertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[1]);

	glBufferData(GL_ARRAY_BUFFER, vboColors.size() * sizeof(float),
		vboColors.data(), GL_STATIC_DRAW);

	

	if (vboCoords.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO[2]);
		glBufferData(GL_ARRAY_BUFFER, vboCoords.size() * sizeof(float),
			vboCoords.data(), GL_STATIC_DRAW);
	}

	if (vboNormals.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO[3]);

		glBufferData(GL_ARRAY_BUFFER, vboNormals.size() * sizeof(float),
			vboNormals.data(), GL_STATIC_DRAW);
	}

	meshSize = vboVertices.size() / 3;
	cout << "			#meshSize= " << meshSize << endl;
}

/// ***********************************************************************
/// **
/// ***********************************************************************

void createAxis() {

	GLfloat vertices[] = { 0.0, 0.0, 0.0,
								scene_max.x * 2, 0.0, 0.0,
								0.0, scene_max.y * 2, 0.0,
								0.0, 0.0, scene_max.z * 2
	};

	GLuint lines[] = { 0, 3,
							0, 2,
							0, 1
	};

	GLfloat colors[] = { 1.0, 1.0, 1.0, 1.0,
							1.0, 0.0, 0.0, 1.0,
							0.0, 1.0, 0.0, 1.0,
							0.0, 0.0, 1.0, 1.0
	};



	glGenBuffers(3, axisVBO);

	glBindBuffer(GL_ARRAY_BUFFER, axisVBO[0]);

	glBufferData(GL_ARRAY_BUFFER, 4 * 3 * sizeof(float),
		vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, axisVBO[1]);

	glBufferData(GL_ARRAY_BUFFER, 4 * 4 * sizeof(float),
		colors, GL_STATIC_DRAW);



	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axisVBO[2]);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(unsigned int),
		lines, GL_STATIC_DRAW);

}

/// ***********************************************************************
/// **
/// ***********************************************************************

void drawAxis(GLuint shader) {

	int attrV, attrC;
	//posi��o
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO[0]);
	attrV = glGetAttribLocation(shader, "aPosition");
	glVertexAttribPointer(attrV, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrV);
	//cor
	glBindBuffer(GL_ARRAY_BUFFER, axisVBO[1]);
	attrC = glGetAttribLocation(shader, "aColor");
	glVertexAttribPointer(attrC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrC);
	//
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axisVBO[3]);
	glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, BUFFER_OFFSET(0));

	glDisableVertexAttribArray(attrV);
	glDisableVertexAttribArray(attrC);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/// ***********************************************************************
/// **
/// ***********************************************************************

void drawMesh(GLuint shader) {

	int attrV, attrC, attrN, attrO;
	// posi��o
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[0]);
	attrV = glGetAttribLocation(shader, "aPosition");
	glVertexAttribPointer(attrV, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrV);
	// cor
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[1]);
	attrC = glGetAttribLocation(shader, "aColor");
	glVertexAttribPointer(attrC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrC);

	// coords

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[2]);
	attrO = glGetAttribLocation(shader, "aCoords");
	glVertexAttribPointer(attrO, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrO);


	// Normal
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[3]);
	attrN = glGetAttribLocation(shader, "aNormal");
	glVertexAttribPointer(attrN, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrN);

	glDrawArrays(GL_TRIANGLES, 0, meshSize);

	glDisableVertexAttribArray(attrV);
	glDisableVertexAttribArray(attrC);
	glDisableVertexAttribArray(attrO);
	glDisableVertexAttribArray(attrN);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



/// ***********************************************************************
/// **
/// ***********************************************************************

void display(void) {

	angleY += 0.02;

	float Max = max(scene_max.x, max(scene_max.y, scene_max.z));

	//########phong

	glm::vec3 lightPos1 = glm::vec3(Max + lmoveX, Max + lmoveY, 0.0) ;

	//camera
	glm::vec3 camPos1 = glm::vec3(1.5f*Max + camMoveX, 1.5f*Max + camMoveY, 1.5f*Max);
	glm::vec3 lookAt1 = glm::vec3(scene_center.x, scene_center.y, scene_center.z);
	glm::vec3 up1 = glm::vec3(0.0, 1.0, 0.0);
	//matriz de vista
	glm::mat4 ViewMat1 = glm::lookAt(camPos1,
		lookAt1,
		up1);
	//matriz de proje��o
	glm::mat4 ProjMat1 = glm::perspective(70.0, 1.0, 0.01, 100.0);
	//matris de modelo
	glm::mat4 ModelMat1 = glm::mat4(1.0);



	//turn table
		 /*
		 ModelMat1 = glm::rotate(ModelMat1, angleX, glm::vec3(1.0, 0.0, 0.0));
		 ModelMat1 = glm::rotate(ModelMat1, angleY, glm::vec3(0.0, 1.0, 0.0));
		 ModelMat1 = glm::rotate(ModelMat1, angleZ, glm::vec3(0.0, 0.0, 1.0));
		 */


	glm::mat4 normalMat1 = glm::transpose(glm::inverse(ModelMat1));
	glm::mat4 MVP1 = ProjMat1 * ViewMat1 * ModelMat1;


	

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderPhong);

		int loc = glGetUniformLocation(shaderPhong, "uMVP1");
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP1));

		loc = glGetUniformLocation(shaderPhong, "uN1");
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(normalMat1));
		loc = glGetUniformLocation(shaderPhong, "uM1");
		glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(ModelMat1));
		loc = glGetUniformLocation(shaderPhong, "uLPos1");
		glUniform3fv(loc, 1, glm::value_ptr(lightPos1));
		loc = glGetUniformLocation(shaderPhong, "uCamPos1");
		glUniform3fv(loc, 1, glm::value_ptr(camPos1));

		glViewport(0, 0, winWidth / 2, winHeight );

		drawAxis(shaderPhong);
		drawMesh(shaderPhong);

       // LIMPANDO O BUFFER

	

		//#######Gooch


		glUseProgram(shaderGooch);

		int loc2 = glGetUniformLocation(shaderGooch, "uMVP1");
		glUniformMatrix4fv(loc2, 1, GL_FALSE, glm::value_ptr(MVP1));

		loc2 = glGetUniformLocation(shaderGooch, "uN1");
		glUniformMatrix4fv(loc2, 1, GL_FALSE, glm::value_ptr(normalMat1));
		loc2 = glGetUniformLocation(shaderGooch, "uM1");
		glUniformMatrix4fv(loc2, 1, GL_FALSE, glm::value_ptr(ModelMat1));
		loc2 = glGetUniformLocation(shaderGooch, "uLPos1");
		glUniform3fv(loc2, 1, glm::value_ptr(lightPos1));
		loc2 = glGetUniformLocation(shaderGooch, "uCamPos1");
		glUniform3fv(loc2, 1, glm::value_ptr(camPos1));



		//glViewport((winWidth / 2),0, winWidth/2 , winHeight);

		drawAxis(shaderGooch);
		drawMesh(shaderGooch);

}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

void initGL(GLFWwindow* window) {

	glClearColor(0.0, 0.0, 0.0, 0.0);

	if (glewInit()) {
		cout << "Unable to initialize GLEW ... exiting" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Status: Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	cout << "Opengl Version: " << glGetString(GL_VERSION) << endl;
	cout << "Opengl Vendor : " << glGetString(GL_VENDOR) << endl;
	cout << "Opengl Render : " << glGetString(GL_RENDERER) << endl;
	cout << "Opengl Shading Language Version : " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	glPointSize(3.0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);

}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

void initShaders(void) {

	// Load shaders and use the resulting shader program
	shaderAmbient = InitShader("shaders/basicShader.vert", "shaders/basicShader.frag");
	shaderGouraud = InitShader("shaders/Gouraud.vert", "shaders/Gouraud.frag");
	shaderPhong = InitShader("shaders/Phong.vert", "shaders/Phong.frag");
	shaderGooch = InitShader("shaders/Gooch.vert", "shaders/Gooch.frag");

	shader = shaderAmbient;
}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static void error_callback(int error, const char* description) {
	cout << "Error: " << description << endl;
}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static void window_size_callback(GLFWwindow* window, int width, int height) {

	glViewport(0, 0, width, height);

}




/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (action == GLFW_PRESS)
		switch (key) {
		case GLFW_KEY_ESCAPE: 	glfwSetWindowShouldClose(window, true);
			break;

		case '.': 	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			break;

		case '-': 	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			break;

		case 'F':
		case 'f': 	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
		case '1':
			lmoveY -= 0.1;
			break;
		case '2':
			lmoveY += 0.1;
			break;
		case '3':
			lmoveX -= 0.1;
			break;
		case '4':
			lmoveX += 0.1;						
			break;
		case '5':
			camMoveX += 0.1;
			break;
		case '6':
			camMoveX -= 0.1;
			break;
		case '7':
			camMoveY -= 0.1;
			break;
		case '8':
			camMoveY += 0.1;
			break;
		case 'R':
		case 'r':
			lmoveX = 0.0f;
			lmoveY = 0.0f;
			camMoveX = 0.0f;
			camMoveY = 0.0f;
			break;

		}

}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static GLFWwindow* initGLFW(char* nameWin, int w, int h) {

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
		exit(EXIT_FAILURE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	GLFWwindow* window = glfwCreateWindow(w, h, nameWin, NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetWindowSizeCallback(window, window_size_callback);

	glfwSetKeyCallback(window, key_callback);

	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);

	return (window);
}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static void GLFW_MainLoop(GLFWwindow* window) {

	while (!glfwWindowShouldClose(window)) {

		double now = glfwGetTime();
		double ellapsed = now - last;

		if (ellapsed > 1.0f / 30.0f) {
			last = now;
			display();
			glfwSwapBuffers(window);
		}

		glfwPollEvents();
	}
}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static void initASSIMP() {

	struct aiLogStream stream;

	/* get a handle to the predefined STDOUT log stream and attach
	   it to the logging system. It remains active for all further
	   calls to aiImportFile(Ex) and aiApplyPostProcessing. */
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, NULL);
	aiAttachLogStream(&stream);
}

/* ************************************************************************* */
/*                                                                           */
/* ************************************************************************* */

static void loadMesh(char* filename) {

	scene = aiImportFile(filename, aiProcessPreset_TargetRealtime_MaxQuality);

	if (!scene) {
		cout << "## ERROR loading mesh" << endl;
		exit(-1);
	}
	



	aiMatrix4x4 trafo;
	aiIdentityMatrix4(&trafo);

	aiVector3D min, max;

	scene_min.x = scene_min.y = scene_min.z = FLT_MAX;
	scene_max.x = scene_max.y = scene_max.z = -FLT_MAX;

	get_bounding_box_for_node(scene->mRootNode, &scene_min, &scene_max, &trafo);

	scene_center.x = (scene_min.x + scene_max.x) / 2.0f;
	scene_center.y = (scene_min.y + scene_max.y) / 2.0f;
	scene_center.z = (scene_min.z + scene_max.z) / 2.0f;

	scene_min.x *= 1.2;
	scene_min.y *= 1.2;
	scene_min.z *= 1.2;
	scene_max.x *= 1.2;
	scene_max.y *= 1.2;
	scene_max.z *= 1.2;

	createAxis();

	if (scene_list == 0)
		createVBOs(scene);

	cout << "Bounding Box: " << " ( " << scene_min.x << " , " << scene_min.y << " , " << scene_min.z << " ) - ( "
		<< scene_max.x << " , " << scene_max.y << " , " << scene_max.z << " )" << endl;
	cout << "Bounding Box: " << " ( " << scene_center.x << " , " << scene_center.y << " , " << scene_center.z << " )" << endl;
}

// carregando texturas
void LoadGLTextures(const aiScene* scene, const string& pModelPath)
{
	// Check if scene has textures.
	if (scene->HasTextures())
	{
		textureIds = new GLuint[scene->mNumTextures];
		glGenTextures(scene->mNumTextures, textureIds);// generate GL-textures ID's
		// upload textures
		for (size_t ti = 0; ti < scene->mNumTextures; ti++)
		{
			glBindTexture(GL_TEXTURE_2D, textureIds[ti]);// Binding of texture name
			//
			//redefine standard texture values
			//
			// We will use linear interpolation for magnification filter
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			// tiling mode
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (scene->mTextures[ti]->achFormatHint[0] & 0x01) ? GL_REPEAT : GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (scene->mTextures[ti]->achFormatHint[0] & 0x01) ? GL_REPEAT : GL_CLAMP);
			// Texture specification
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, scene->mTextures[ti]->mWidth, scene->mTextures[ti]->mHeight, 0, GL_BGRA, GL_UNSIGNED_BYTE,
				scene->mTextures[ti]->pcData);
		}
	}
}


/* ************************************************************************* */
/* ************************************************************************* */
/* *****                                                               ***** */
/* ************************************************************************* */
/* ************************************************************************* */

int main(int argc, char *argv[]) {

	GLFWwindow* window;

	char meshFilename[] = "bunny3.obj";

	window = initGLFW(argv[0], winWidth, winHeight);

	initASSIMP();
	initGL(window);
	initShaders();

	if (argc > 1)
		loadMesh(argv[1]);
	else
		loadMesh(meshFilename);

	LoadGLTextures(scene, meshFilename);

	GLFW_MainLoop(window);

	glfwDestroyWindow(window);
	glfwTerminate();

	aiReleaseImport(scene);
	aiDetachAllLogStreams();

	exit(EXIT_SUCCESS);
}


