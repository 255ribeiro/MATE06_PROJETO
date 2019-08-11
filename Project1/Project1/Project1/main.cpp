
#include <iostream>
#include <cfloat>
#include <vector>
//
#include <glm/vec3.hpp> 
#include <glm/vec4.hpp> 
#include <glm/mat4x4.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 
//
/* assimp include files. These three are usually needed. */
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include "shaders/initShaders.h"

#include <src/SOIL.h>

//to map image filenames to textureIds
#include <string.h>


#ifndef BUFFER_OFFSET 
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#endif
#ifndef MIN
#define MIN(x,y) (x<y?x:y)
#endif
#ifndef MAX
#define MAX(x,y) (y>x?y:x)
#endif


using namespace std;

// shadres
GLuint
shaderGooch,
shaderPhong,
shaderAmbient;

GLuint 	axisVBO[3];
GLuint 	meshVBO[5];
GLuint 	meshSize;
GLuint  texLoad;

GLuint tex1dID;
GLuint textureID;
double  last;
double  lastTT;

vector<GLfloat> vboVertices;
vector<GLfloat> vboNormals;
vector<GLfloat> vboColors;
vector<GLfloat> vboTxCoords;
vector<GLfloat> vboTx1dCoords;



GLfloat tex1dData[64][4];
glm::vec3 camPos1;
glm::vec3 lightPos1;
glm::vec3 lookAt1; 
glm::vec3 up1 = glm::vec3(0.0, 1.0, 0.0);
glm::mat4 ViewMat1;
glm::mat4 ModelMat1;

glm::mat4 lightM;
glm::vec4 lightPos2;


float Max;

glm::mat4 ProjMat1;

// attributes int
int loc;

// dimensões da tela
int winWidth = 600,
winHeight = 600;

// Turn Table ang

bool turntable = false;

// Mudança de luz
float lMove = 0.1f;
float lRot = 0.5f;

// Mudança de cam:
float camMove = 0.5f;
float camRot = 1.0f;

/* the global Assimp scene object */
const aiScene* scene = NULL;
GLuint scene_list = 0;
aiVector3D scene_min, scene_max, scene_center;


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

void make1dtexture() {
	int i;
	GLfloat c;
	for (i = 0; i < 64; i++) {
		c = GLfloat(i) / 63;
		tex1dData[i][0] = c;
		tex1dData[i][1] = .3;
		tex1dData[i][2] = .3;
		tex1dData[i][3] = 1;

		//cout << i << "__" << tex1dData[i][0] << " " << tex1dData[i][1] << " " << tex1dData[i][2] << " " << tex1dData[i][3] << endl;
		
	}

	


}

/// ***********************************************************************
/// **
/// ***********************************************************************

int LoadGLTextures(const char *  txNome)
{
	textureID = SOIL_load_OGL_texture
	(
		txNome,
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y
	);

	if (textureID == 0)
		return false;


	glBindTexture(GL_TEXTURE_2D, textureID);


	return true;
}

int traverseScene(const aiScene *sc, const aiNode* nd) {
	
	int totVertices = 0;

	/* draw all meshes assigned to this node */
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n) {
		const aiMesh* mesh = scene->mMeshes[nd->mMeshes[n]];
		const aiMaterial* mtl = scene->mMaterials[mesh->mMaterialIndex];
	
	
		// cores
		glm::vec4 color = glm::vec4(0.0, 1.0, 0.0, 1.0);
		// caso tenham cores no mtl
		aiColor4D diffuse;
		if (AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
			color = glm::vec4(diffuse.r, diffuse.g, diffuse.b, diffuse.a);
		
		// # textura começo

		aiString  path;
		aiReturn texFound = scene->mMaterials[n]->GetTexture(aiTextureType_DIFFUSE, 0, &path);
		string pathstr = path.C_Str();
		
		// tamanho da string carregada
		texLoad = pathstr.length();
		// se carregou uma string
		if (texLoad > 0) {
			// carregar a textura
			int texBool = LoadGLTextures(path.C_Str());
		
		}
	
		// # textura fim


		for (unsigned int t = 0; t < mesh->mNumFaces; ++t) {
			const aiFace* face = &mesh->mFaces[t];


			for (unsigned int i = 0; i < face->mNumIndices; i++) {
				int index = face->mIndices[i];

				//if(mesh->mColors[0] != NULL) {

				vboColors.push_back(color.r);
				vboColors.push_back(color.g);
				vboColors.push_back(color.b);
				vboColors.push_back(color.a);

				//	}

				if (mesh->HasTextureCoords(0)) {
					
					vboTxCoords.push_back(mesh->mTextureCoords[0][index][0]);
					vboTxCoords.push_back(mesh->mTextureCoords[0][index][1]);
					
				}
				else{ texLoad = 0; }
			
				if (mesh->mNormals != NULL) {
					vboNormals.push_back(mesh->mNormals[index].x);
					vboNormals.push_back(mesh->mNormals[index].y);
					vboNormals.push_back(mesh->mNormals[index].z);
				}
				vboVertices.push_back(mesh->mVertices[index].x);
				vboVertices.push_back(mesh->mVertices[index].y);
				vboVertices.push_back(mesh->mVertices[index].z);
				

				//vboTx1dCoords.push_back(mesh->mVertices[index].y);
				vboTx1dCoords.push_back( (mesh->mVertices[index].y - scene_min.y) / (scene_max.y - scene_min.y) );
								
				totVertices++;
				//cout << totVertices << endl;
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
	cout << "			#vboTxCoords= " << vboTxCoords.size() << endl;

	//gerando o buffer ( num de atributos, id do buffer)
	glGenBuffers(5, meshVBO); 
	//passando o primeiro atributo do buffer para a gpu
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[0]);
	// colocando dados no primeiro atributo do buffer
	glBufferData(GL_ARRAY_BUFFER, vboVertices.size() * sizeof(float),
		vboVertices.data(), GL_STATIC_DRAW);
	// 
	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[1]);

	glBufferData(GL_ARRAY_BUFFER, vboColors.size() * sizeof(float),
		vboColors.data(), GL_STATIC_DRAW);

	if (vboNormals.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO[2]);

		glBufferData(GL_ARRAY_BUFFER, vboNormals.size() * sizeof(float),
			vboNormals.data(), GL_STATIC_DRAW);
	}


	if (vboTxCoords.size() > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO[3]);

		glBufferData(GL_ARRAY_BUFFER, vboTxCoords.size() * sizeof(float),
			vboTxCoords.data(), GL_STATIC_DRAW);
	}

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[4]);

	glBufferData(GL_ARRAY_BUFFER, vboTx1dCoords.size() * sizeof(float),
		vboTx1dCoords.data(), GL_STATIC_DRAW);

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

	glBindBuffer(GL_ARRAY_BUFFER, axisVBO[0]);
	attrV = glGetAttribLocation(shader, "aPosition");
	glVertexAttribPointer(attrV, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrV);

	glBindBuffer(GL_ARRAY_BUFFER, axisVBO[1]);
	attrC = glGetAttribLocation(shader, "aColor");
	glVertexAttribPointer(attrC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrC);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axisVBO[2]);
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

	int attrV, attrC, attrN, attrCo, attrC1d;

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[0]);
	attrV = glGetAttribLocation(shader, "aPosition");
	glVertexAttribPointer(attrV, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrV);

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[1]);
	attrC = glGetAttribLocation(shader, "aColor");
	glVertexAttribPointer(attrC, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrC);

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[2]);
	attrN = glGetAttribLocation(shader, "aNormal");
	glVertexAttribPointer(attrN, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrN);


	if (texLoad > 0) {
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO[3]);
		attrCo = glGetAttribLocation(shader, "aCoords");
		glVertexAttribPointer(attrCo, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(attrCo);
	}

	glBindBuffer(GL_ARRAY_BUFFER, meshVBO[4]);
	attrC1d = glGetAttribLocation(shader, "aC1d");
	glVertexAttribPointer(attrC1d, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(attrC1d);

	glDrawArrays(GL_TRIANGLES, 0, meshSize);

	glDisableVertexAttribArray(attrV);
	glDisableVertexAttribArray(attrC);
	glDisableVertexAttribArray(attrN);


	if (texLoad > 0) {
		glDisableVertexAttribArray(attrCo);
	}
	glDisableVertexAttribArray(attrC1d);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}



/// ***********************************************************************
/// **
/// ***********************************************************************

void display(void) {
	

	//turn table
	if (turntable == true) {

		double nowTT = glfwGetTime();
		double ellapsedTT = nowTT - lastTT;

		if (ellapsedTT > 1.0f / 30.0f) {
			lastTT = nowTT;
		
			ModelMat1 = glm::rotate(ModelMat1, 0.02f, glm::vec3(0.0, 1.0, 0.0));
		}
	}
		 
	
	glm::mat4 normalMat1 = glm::transpose(glm::inverse(ModelMat1));
	glm::mat4 MVP1 = ProjMat1 * ViewMat1 * ModelMat1;

	//limpando os buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	

	//########phong

	// definindo o shader
	glUseProgram(shaderPhong);

	// definindo uniforms
	loc = glGetUniformLocation(shaderPhong, "uMVP1");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP1));
	loc = glGetUniformLocation(shaderPhong, "uN1");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(normalMat1));
	loc = glGetUniformLocation(shaderPhong, "uM1");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(ModelMat1));
	loc = glGetUniformLocation(shaderPhong, "uLPos1");
	glUniform3fv(loc, 1, glm::value_ptr(lightPos1));
	loc = glGetUniformLocation(shaderPhong, "uCamPos1");
	glUniform3fv(loc, 1, glm::value_ptr(camPos1));
	loc = glGetUniformLocation(shaderPhong, "utexLoad");
	glUniform1i(loc, texLoad);



	glViewport(0, 0, winWidth / 2, winHeight);

	drawAxis(shaderAmbient);
	drawMesh(shaderPhong);
	
	glDisable(GL_TEXTURE_2D);

	//#######Gooch
	
	glEnable(GL_TEXTURE_1D);

	glActiveTexture(GL_TEXTURE1);
	// trocando o shader
	glUseProgram(shaderGooch);
	
	// redefinindo uniforms
	loc = glGetUniformLocation(shaderGooch, "uMVP2");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(MVP1));

	loc = glGetUniformLocation(shaderGooch, "uN2");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(normalMat1));
	loc = glGetUniformLocation(shaderGooch, "uM2");
	glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(ModelMat1));
	loc = glGetUniformLocation(shaderGooch, "uLPos2");
	glUniform3fv(loc, 1, glm::value_ptr(lightPos1));
	loc = glGetUniformLocation(shaderGooch, "uCamPos2");
	glUniform3fv(loc, 1, glm::value_ptr(camPos1));
	//loc = glGetUniformLocation(shaderGooch, "utex1dID");
	//glUniform1i(loc, tex1dID);

	glViewport(winWidth / 2, 0, winWidth / 2, winHeight);

	drawAxis(shaderAmbient);
	drawMesh(shaderGooch);

	glDisable(GL_TEXTURE_1D);
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

	shaderPhong = InitShader("shaders/Phong.vert", "shaders/Phong.frag");
	shaderGooch = InitShader("shaders/Gooch.vert", "shaders/Gooch.frag");
	shaderAmbient = InitShader("shaders/basicShader.vert", "shaders/basicShader.frag");
	
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
	winWidth = width;
	winHeight = height;
	
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


		// move camera da direção +x
		case '1':
					
			ViewMat1 = glm::translate(ViewMat1, glm::vec3(camMove, 0.0, 0.0));
					break;

		// move camera da direção -x	
		case '2':
			ViewMat1 = glm::translate(ViewMat1, glm::vec3(-camMove, 0.0, 0.0));
		
			break;

		// move camera da direção +y	
		case '3':
			ViewMat1 = glm::translate(ViewMat1, glm::vec3(0, camMove, 0.0));
			
			break;

		// move camera da direção -y	
		case '4':
			ViewMat1 = glm::translate(ViewMat1, glm::vec3(0, -camMove, 0.0));
			break;

		// move camera da direção z
		case '5':
			ViewMat1 = glm::translate(ViewMat1, glm::vec3(0, 0.0 , camMove));
			break;

		// move camera da direção -z	
		case '6':
			ViewMat1 = glm::translate(ViewMat1, glm::vec3(0, 0.0, -camMove));
			break;

		// rotaciona a camera ao redor do eixo y
		case '7':
			ViewMat1 = glm::rotate(ViewMat1, camRot, glm::vec3(0.0, 1.0, 0.0));
			break;

		// rotaciona a camera ao redor do eixo -y
		case '8':
			ViewMat1 = glm::rotate(ViewMat1, -camRot, glm::vec3(0.0, 1.0, 0.0));
			break;
		// rotaciona a luz ao redor do eixo y
		case 'A':
			
			lightM = glm::mat4(1.0f);
			lightPos2 = glm::vec4(lightPos1, 1.0);
			lightM = glm::rotate(lightM, lRot, glm::vec3(0.0f, 1.0f, 0.0f));
			lightPos2 = lightM * lightPos2;
			lightPos1 = glm::vec3(lightPos2.x, lightPos2.y, lightPos2.z);
		break;

		// rotaciona a luz ao redor do eixo -y
		case 'a':
			
			lightM = glm::mat4(1.0f);
			lightPos2 = glm::vec4(lightPos1, 1.0);
			lightM = glm::rotate(lightM, -lRot, glm::vec3(0.0f, 1.0f, 0.0f));
			lightPos2 = lightM * lightPos2;
			lightPos1 = glm::vec3(lightPos2.x, lightPos2.y, lightPos2.z);
		break; 

		// rotaciona a luz ao redor do eixo x
		case 'S':
		
			lightM = glm::mat4(1.0f);
			lightPos2 = glm::vec4(lightPos1, 1.0);
			lightM = glm::rotate(lightM, lRot, glm::vec3(1.0f, 0.0f, 0.0f));
			lightPos2 = lightM * lightPos2;
			lightPos1 = glm::vec3(lightPos2.x, lightPos2.y, lightPos2.z);
		break;
		
		// rotaciona a luz ao redor do eixo -x
		case 's':
		
			lightM = glm::mat4(1.0f);
			lightPos2 = glm::vec4(lightPos1, 1.0);
			lightM = glm::rotate(lightM, -lRot, glm::vec3(1.0f, 0.0f, 0.0f));
			lightPos2 = lightM * lightPos2;
			lightPos1 = glm::vec3(lightPos2.x, lightPos2.y, lightPos2.z);
		break;
		
		// move a luz na direção z
		case '9':
		
			lightM = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, scene_max.z * lMove));
			lightPos2 = glm::vec4(lightPos1, 1.0);
			lightPos2 = lightM * lightPos2;
			lightPos1 = glm::vec3(lightPos1.x, lightPos1.y, lightPos2.z);
		break;

		
		// move a luz na direção -z
		case '0':
		
			lightM = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, scene_max.z * -lMove));;
			lightPos2 = glm::vec4(lightPos1, 1.0);
			lightPos2 = lightM * lightPos2;
			lightPos1 = glm::vec3(lightPos1.x, lightPos1.y, lightPos2.z);
		break;
		

		case 'T':
		case 't':
			turntable = !turntable;
		break;

		//reset ViewMat1, lightpos1
		case 'r':
		case 'R':
			// posição da luz
			lightPos1 = glm::vec3(Max, Max, 0.0);
			//posição da camera
			camPos1 = glm::vec3(1.5f*Max, 1.5f*Max, 1.5f*Max);
			// ponto de vista
			lookAt1 = glm::vec3(scene_center.x, scene_center.y, scene_center.z);
			//matriz de projeção
			ProjMat1 = glm::perspective(70.0, 1.0, 0.01, 100.0);
			//matriz de modelo
			ModelMat1 = glm::mat4(1.0);
			//matriz de vista
			ViewMat1 = glm::lookAt(camPos1, lookAt1, up1);
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


/* ************************************************************************* */

int main(int argc, char *argv[]) {

	GLFWwindow* window;

	char meshFilename[] = "bunny4.obj";

	window = initGLFW(argv[0], winWidth, winHeight);

	initASSIMP();
	initGL(window);
	initShaders();

	if (argc > 1)
		loadMesh(argv[1]);
	else
		loadMesh(meshFilename);
	//Textura 1d
	make1dtexture();

	glGenTextures(1, &tex1dID);

	glBindTexture(GL_TEXTURE_1D, tex1dID);

	glTexImage1D(
		GL_TEXTURE_1D,      // Specifies the target texture. Must be GL_TEXTURE_1D or GL_PROXY_TEXTURE_1D.
		0,                  // Specifies the level-of-detail number. Level 0 is the base image level. Level n is the nth mipmap reduction image.
		GL_RGBA,
		64,  
		0,                  // border: This value must be 0.
		GL_RGBA,
		GL_FLOAT,
		tex1dData);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);

	

	Max = max(scene_max.x, max(scene_max.y, scene_max.z));
	// posição da luz
	lightPos1 = glm::vec3(Max, Max, 0.0);
	//posição da camera
	camPos1= glm::vec3(1.5f*Max, 1.5f*Max, 1.5f*Max);
	// ponto de vista
	lookAt1 = glm::vec3(scene_center.x, scene_center.y, scene_center.z);
	//matriz de projeção
	ProjMat1 = glm::perspective(70.0, 1.0, 0.01, 100.0);
	//matriz de modelo
	ModelMat1 = glm::mat4(1.0);


	//matriz de vista
	ViewMat1 = glm::lookAt(camPos1, 
		lookAt1,
		up1);

	//loop principal
	GLFW_MainLoop(window);

	// fechando a janela
	glfwDestroyWindow(window);

	glfwTerminate();

	aiReleaseImport(scene);
	aiDetachAllLogStreams();

	exit(EXIT_SUCCESS);
}

//