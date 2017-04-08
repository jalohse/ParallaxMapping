#define _USE_MATH_DEFINES

#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "cyTriMesh.h"
#include "cyMatrix.h"
#include "cyGL.h"
#include "lodepng.h"


#define width 300
#define far_plane 500.0f
#define inital_z -70.0f
#define fov_degrees 45.0f
#define flip_degree 135.1f
#define y_trans -7

std::vector<cyPoint3f> plane_vertices = {
	cyPoint3f(-20.5f, -20.5f, 0.0f),
	cyPoint3f(20.5f, -20.5f, 0.0f),
	cyPoint3f(-20.5f, 20.5f, 0.0f),
	cyPoint3f(-20.5f, 20.5f, 0.0f),
	cyPoint3f(20.5f, -20.5f, 0.0f),
	cyPoint3f(20.5f, 20.5f, 0.0f)
};

std::vector<cyPoint3f> planeTextureVertices = {
	cyPoint3f(0, 0, 0),
	cyPoint3f(1, 0, 0),
	cyPoint3f(0, 1, 0),
	cyPoint3f(0, 1, 0),
	cyPoint3f(1, 0, 0),
	cyPoint3f(1, 1, 0)
};

cy::Matrix4<float> totalPlaneRotationMatrix = cyMatrix4f::MatrixIdentity();
cyPoint3f lightPos = cyPoint3f(18, 60, 20);
cyPoint3f cameraPos = cyPoint3f(40, 110, 40);

cyTriMesh mesh;

GLuint vertexArrayObj;
GLuint planeVertexArrayObj;
GLuint cubeVertexArrayObj;
GLuint depthVertexArrayObj;

cy::GLSLProgram teapot_shaders;
cy::GLSLProgram depth_shaders;
cy::GLSLProgram plane_shaders;

cy::Matrix4<float> cameraTransformationMatrix;
cy::Matrix4<float> planeCameraTransformationMatrix;
cy::Matrix4<float> lightCameraTransformationMatrix;
cy::Matrix4<float> perspectiveMatrix;
cy::Matrix4<float> totalRotationMatrix;
cy::Matrix4<float> translationMatrix;
cy::Matrix4<float> planeTransformationMatrix;
cy::Matrix4<float> lightTransformationMatrix;
cy::Matrix4<float> lightRotationMatrix;
int selected;
int selectedKey;
bool zoom_in = true;
int movement = 0;
std::vector<cyPoint3f> vertices;
std::vector<cyPoint3f> textureVertices;
std::vector<cyPoint3f> normals;
std::vector<cyPoint3f> lightVertices;
cy::GLRenderDepth<GL_TEXTURE_2D> buffer;
GLuint textureID[2];
unsigned diffWidth, diffHeight, specHeight, specWidth;
cyPoint3f upVec = cyPoint3f(0, 1, 0);
cyMatrix4f view = cyMatrix4f::MatrixView(cameraPos, cyPoint3f(0, 0, 0), upVec);
cyMatrix4f lightView = cyMatrix4f::MatrixView(lightPos, cyPoint3f(0, 0, 0), upVec);
cyMatrix4f lightProj = cyMatrix4f::MatrixPerspective(M_PI / 8, 1, 20, 200);
cyMatrix4f bias = cyMatrix4f::MatrixTrans(cyPoint3f(0.5f, 0.5f, 0.495f)) * cyMatrix4f::MatrixScale(0.5f, 0.5f, 0.5f);
cyMatrix4f teapotLightMVP;
cyMatrix4f planeLightMVP;



void setInitialRotationAndTranslation() {
	cyPoint3f max = mesh.GetBoundMax();
	cyPoint3f min = mesh.GetBoundMin();
	cyPoint3f mid = max + min;
	cy::Matrix4<float> rotationZ = cyMatrix4f::MatrixRotationZ(-10);
	cy::Matrix4<float> rotationX = cyMatrix4f::MatrixRotationX(40.2);
	cyPoint3f translation = cyPoint3f(0, 0, 0);
	translationMatrix = cyMatrix4f::MatrixTrans(translation);

	totalRotationMatrix = rotationX * rotationZ;
	cameraTransformationMatrix = translationMatrix * totalRotationMatrix;
	planeCameraTransformationMatrix = cyMatrix4f::MatrixTrans(cyPoint3f(0, -5, -10));

	lightRotationMatrix = cyMatrix4f::MatrixRotationX(0);
	teapotLightMVP = lightProj * lightView * cameraTransformationMatrix;
	planeLightMVP = lightProj * lightView * planeCameraTransformationMatrix;
}

void display() {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	teapot_shaders.Bind();
	glBindVertexArray(vertexArrayObj);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	plane_shaders.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glUniform1i(glGetUniformLocation(plane_shaders.GetID(), "diffuseMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glUniform1i(glGetUniformLocation(plane_shaders.GetID(), "normalMap"), 1);
	glBindVertexArray(planeVertexArrayObj);
	glDrawArrays(GL_TRIANGLES, 0, plane_vertices.size());

	glutSwapBuffers();
}


void zoomPlane() {
	plane_shaders.Bind();
	int data = planeTransformationMatrix.data[14];
	cyPoint3f translation = cyPoint3f(0.0f, 0.0f, 0.05f);
	if (data > -25 || !zoom_in) {
		zoom_in = false;
		translation = cyPoint3f(0.0f, 0.0f, -0.05f);
	}
	planeTransformationMatrix = cyMatrix4f::MatrixTrans(translation) * planeTransformationMatrix;
	planeCameraTransformationMatrix = planeTransformationMatrix * totalPlaneRotationMatrix;
	plane_shaders.SetUniform(1, planeCameraTransformationMatrix);
	glutPostRedisplay();
}

void zoom() {
	teapot_shaders.Bind();
	int data = translationMatrix.data[14];
	cyPoint3f translation = cyPoint3f(0.0f, 0.0f, 1.0f);
	if (data > -25 || !zoom_in) {
		zoom_in = false;
		translation = cyPoint3f(0.0f, 0.0f, -1.0f);
	}
	translationMatrix = cyMatrix4f::MatrixTrans(translation) * translationMatrix;
	cameraTransformationMatrix = translationMatrix * totalRotationMatrix;
	teapot_shaders.SetUniform(1, cameraTransformationMatrix);
	teapot_shaders.SetUniform(3, cameraTransformationMatrix.GetInverse().GetTranspose());
	glutPostRedisplay();
}

void rotatePlane() {
	plane_shaders.Bind();
	totalPlaneRotationMatrix = cyMatrix4f::MatrixRotationX(0.5) * totalPlaneRotationMatrix;
	planeCameraTransformationMatrix = planeTransformationMatrix * totalPlaneRotationMatrix;
	plane_shaders.SetUniform(1, planeCameraTransformationMatrix);
	glutPostRedisplay();
}

void rotate() {
	teapot_shaders.Bind();
	totalRotationMatrix = cyMatrix4f::MatrixRotationY(0.5) * totalRotationMatrix;
	cameraTransformationMatrix = translationMatrix * totalRotationMatrix;
	teapot_shaders.SetUniform(1, cameraTransformationMatrix);
	teapot_shaders.SetUniform(3, cameraTransformationMatrix.GetInverse().GetTranspose());
	glutPostRedisplay();
}


void updateLightPosition() {
	lightView = cyMatrix4f::MatrixView(lightPos, cyPoint3f(0, 0, 0), cyPoint3f(0, 1, 0));
	teapotLightMVP = lightProj * lightView * cameraTransformationMatrix;
	planeLightMVP = lightProj * lightView * planeCameraTransformationMatrix;
	depth_shaders.Bind();
	depth_shaders.SetUniform(1, teapotLightMVP);
	teapot_shaders.Bind();
	teapot_shaders.SetUniform(6, bias * teapotLightMVP);
	teapot_shaders.SetUniform(7, lightPos);
	plane_shaders.Bind();
	plane_shaders.SetUniform(3, bias * planeLightMVP);
	lightTransformationMatrix = cyMatrix4f::MatrixTrans(lightPos);
	lightCameraTransformationMatrix = lightTransformationMatrix * cyMatrix4f::MatrixRotationY(-10);
}

void moveLight() {
	if (movement != 10) {
		lightPos = cyPoint3f(lightPos.x + 2, lightPos.y, lightPos.z - 1);
		movement = movement + 1;
	}
	else {
		lightPos = cyPoint3f(lightPos.x - 2, lightPos.y, lightPos.z + 1);
	}

	updateLightPosition();
	glutPostRedisplay();
}

void onClick(int button, int state, int x, int y) {

	int mod = glutGetModifiers();
	if (mod == GLUT_ACTIVE_CTRL) {
		selectedKey = mod;
		moveLight();
	}
	else if (mod == GLUT_ACTIVE_ALT) {
		selectedKey = mod;
		if (button == GLUT_RIGHT_BUTTON) {
			if (state == GLUT_DOWN) {
				selected = GLUT_RIGHT_BUTTON;
				zoomPlane();
			}
			else {
				selected = NULL;
			}
		}
		else if (button == GLUT_LEFT_BUTTON) {
			if (state == GLUT_DOWN) {
				selected = GLUT_LEFT_BUTTON;
				rotatePlane();
			}
			else {
				selected = NULL;
			}
		}
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		if (state == GLUT_DOWN) {
			selected = GLUT_RIGHT_BUTTON;
			zoom();
		}
		else {
			selected = NULL;
		}
	}
	else if (button == GLUT_LEFT_BUTTON) {
		if (state == GLUT_DOWN) {
			selected = GLUT_LEFT_BUTTON;
			rotate();
		}
		else {
			selected = NULL;
		}
	}
}

void move(int x, int y) {
	if (selectedKey == GLUT_ACTIVE_CTRL) {
		moveLight();
	}
	else if (selectedKey == GLUT_ACTIVE_ALT && selected == GLUT_RIGHT_BUTTON) {
		zoomPlane();
	}
	else if (selectedKey == GLUT_ACTIVE_ALT && selected == GLUT_LEFT_BUTTON) {
		rotatePlane();
	}
	else if (selected == GLUT_RIGHT_BUTTON) {
		zoom();
	}
	else if (selected == GLUT_LEFT_BUTTON) {
		rotate();
	}
}


void reset(int key, int x, int y) {
	if (key == GLUT_KEY_F6) {
		setInitialRotationAndTranslation();
		teapot_shaders.Bind();
		teapot_shaders.SetUniform(1, cameraTransformationMatrix);
		teapot_shaders.SetUniform(3, cameraTransformationMatrix.GetInverse().GetTranspose());
		glutPostRedisplay();
	}
}

void populateVerticesAndNormals() {
	vertices = {};
	for (int i = 0; i < mesh.NF(); i = i + 1) {
		cy::TriMesh::TriFace face = mesh.F(i);
		vertices.push_back(mesh.V(face.v[0]));
		vertices.push_back(mesh.V(face.v[1]));
		vertices.push_back(mesh.V(face.v[2]));
		cy::TriMesh::TriFace nface = mesh.FN(i);
		normals.push_back(mesh.VN(nface.v[0]).GetNormalized());
		normals.push_back(mesh.VN(nface.v[1]).GetNormalized());
		normals.push_back(mesh.VN(nface.v[2]).GetNormalized());
	}
}


void createObj(char* fileName) {
	mesh = cyTriMesh();
	mesh.LoadFromFileObj(fileName);
	mesh.ComputeBoundingBox();
	mesh.ComputeNormals();

	populateVerticesAndNormals();
	setInitialRotationAndTranslation();

	teapot_shaders = cy::GLSLProgram();
	teapot_shaders.BuildFiles("teapot_vertex_shader.glsl", "teapot_fragment_shader.glsl");
	teapot_shaders.Bind();
	teapot_shaders.RegisterUniform(1, "cameraTransformation");
	teapot_shaders.SetUniform(1, cameraTransformationMatrix);
	teapot_shaders.RegisterUniform(2, "perspective");
	teapot_shaders.SetUniform(2, perspectiveMatrix);
	teapot_shaders.RegisterUniform(3, "inverseCamera");
	teapot_shaders.SetUniform(3, cameraTransformationMatrix.GetInverse().GetTranspose());
	teapot_shaders.RegisterUniform(4, "lightRotation");
	teapot_shaders.SetUniform(4, lightRotationMatrix);
	teapot_shaders.RegisterUniform(5, "view");
	teapot_shaders.SetUniform(5, view);
	teapot_shaders.RegisterUniform(6, "shadowMatrix");
	teapot_shaders.SetUniform(6, bias * teapotLightMVP);
	teapot_shaders.RegisterUniform(7, "lightPos");
	teapot_shaders.SetUniform(7, lightPos);

	GLuint vertexBufferObj[2];

	glGenVertexArrays(1, &vertexArrayObj);
	glBindVertexArray(vertexArrayObj);

	glGenBuffers(1, vertexBufferObj);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * normals.size(), &normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindVertexArray(0);

	depth_shaders = cy::GLSLProgram();
	depth_shaders.BuildFiles("depth_map_vertex_shader.glsl", "depth_map_fragment_shader.glsl");
	depth_shaders.Bind();
	depth_shaders.RegisterUniform(1, "shadowMatrix");
	depth_shaders.SetUniform(1, teapotLightMVP);

	GLuint depthVertexBufferObj[2];

	glGenVertexArrays(1, &depthVertexArrayObj);
	glBindVertexArray(depthVertexArrayObj);

	glGenBuffers(1, depthVertexBufferObj);

	glBindBuffer(GL_ARRAY_BUFFER, depthVertexBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindVertexArray(0);
}

std::vector<cyPoint3f> calculateTangentsOfPlane()
{
	cyPoint3f edge1 = plane_vertices.at(1) - plane_vertices.at(0);
	cyPoint3f edge2 = plane_vertices.at(2) - plane_vertices.at(0);
	cyPoint3f tex_change1 = planeTextureVertices.at(1) - planeTextureVertices.at(0);
	cyPoint3f tex_change2 = planeTextureVertices.at(2) - planeTextureVertices.at(0);
	float determinant_tri1 = 1.0f / (tex_change1.x * tex_change2.y - tex_change2.x * tex_change1.y);
	cyPoint3f tangent_tri1 = cyPoint3f(determinant_tri1 * (tex_change2.y * edge1.x - tex_change1.y * edge2.x),
		determinant_tri1 * (tex_change2.y * edge1.y - tex_change1.y * edge2.y),
		determinant_tri1 * (tex_change2.y * edge1.z - tex_change1.y * edge2.z));
	tangent_tri1 = tangent_tri1.GetNormalized();

	cyPoint3f bitangent1 = cyPoint3f(determinant_tri1 * (-tex_change2.x * edge1.x + tex_change1.x * edge2.x),
	determinant_tri1 * (-tex_change2.x * edge1.y + tex_change1.x * edge2.y),
		determinant_tri1 * (-tex_change2.x * edge1.z + tex_change1.x * edge2.z));
	bitangent1 = bitangent1.GetNormalized();

	return std::vector<cyPoint3f>{tangent_tri1, bitangent1};

}

std::vector<unsigned char> flipImage(int total, std::vector<unsigned char> imageVector) {
	std::vector<unsigned char> image(total + 1);
	for (int i = 0; i <= total; i += 4) {
		char r = imageVector[total - i - 3];
		char g = imageVector[total - i - 2];
		char b = imageVector[total - i - 1];
		char a = imageVector[total - i];
		image[i] = r;
		image[i + 1] = g;
		image[i + 2] = b;
		image[i + 3] = a;
	}
	return image;
}

std::vector<unsigned char> generateImage(std::string image) {
	std::vector<unsigned char> diffuseVector;
	int total;
	if (diffWidth == 0) {
		lodepng::decode(diffuseVector, diffWidth, diffHeight, image);
		total = diffHeight * diffWidth * 4 - 1;
	}
	else {
		lodepng::decode(diffuseVector, specWidth, specHeight, image);
		total = specHeight * specWidth * 4 - 1;
	}
	return flipImage(total, diffuseVector);
}

void loadTextures() {
	std::vector<unsigned char> diffuseImage = generateImage("bricks2.png");
	std::vector<unsigned char> normalImage = generateImage("bricks2_normal.png");

	glGenTextures(2, textureID);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, diffWidth, diffHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &diffuseImage[0]);

	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, specWidth, specHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &normalImage[0]);

	glBindTexture(GL_TEXTURE_2D, 0);

}


void createPlane() {
	planeTransformationMatrix = cyMatrix4f::MatrixTrans(cyPoint3f(0, 0, 0));
	planeCameraTransformationMatrix = planeTransformationMatrix * cyMatrix4f::MatrixRotationX(80);
	cyPoint3f normal = plane_vertices.at(0).Cross(plane_vertices.at(1)).GetNormalized();

	plane_shaders = cy::GLSLProgram();
	plane_shaders.BuildFiles("plane_vertex_shader.glsl", "plane_fragment_shader.glsl");
	plane_shaders.Bind();
	plane_shaders.RegisterUniform(1, "cameraTransformation");
	plane_shaders.SetUniform(1, planeCameraTransformationMatrix);
	plane_shaders.RegisterUniform(2, "perspective");
	plane_shaders.SetUniform(2, perspectiveMatrix);
	plane_shaders.RegisterUniform(3, "normal");
	plane_shaders.SetUniform(3, normal);
	plane_shaders.RegisterUniform(4, "view");
	plane_shaders.SetUniform(4, view);
	plane_shaders.RegisterUniform(5, "lightPos");
	plane_shaders.SetUniform(5, lightPos);
	plane_shaders.RegisterUniform(6, "cameraPos");
	plane_shaders.SetUniform(6, cameraPos);

	loadTextures();

	GLuint planeVertexBufferObj[1];
	GLuint planeTextureBufferObj[1];
	GLuint planeTangentBufferObj[1];
	GLuint planeBitangentBufferObj[1];

	glGenVertexArrays(1, &planeVertexArrayObj);
	glBindVertexArray(planeVertexArrayObj);

	glGenBuffers(1, planeVertexBufferObj);
	glBindBuffer(GL_ARRAY_BUFFER, planeVertexBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * plane_vertices.size(), &plane_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);


	glGenBuffers(1, planeTextureBufferObj);
	glBindBuffer(GL_ARRAY_BUFFER, planeTextureBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * planeTextureVertices.size(), &planeTextureVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	std::vector<cyPoint3f> tangentsBitangents = calculateTangentsOfPlane();

	cyPoint3f tangent = tangentsBitangents.at(0);
	std::vector<cyPoint3f> tangents = { tangent, tangent, tangent, tangent, tangent, tangent };

	glGenBuffers(1, planeTangentBufferObj);
	glBindBuffer(GL_ARRAY_BUFFER, planeTangentBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * tangents.size(), &tangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	cyPoint3f bitangent = tangentsBitangents.at(1);
	std::vector<cyPoint3f> bitangents = { bitangent, bitangent, bitangent, bitangent, bitangent, bitangent };

	glGenBuffers(1, planeBitangentBufferObj);
	glBindBuffer(GL_ARRAY_BUFFER, planeBitangentBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * tangents.size(), &bitangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindVertexArray(0);
}


int main(int argc, char* argv[])
{

	glutInit(&argc, argv);
	glutInitContextFlags(GLUT_DEBUG);
	glutInitWindowSize(width, width);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Shadow Mapping");
	cyGL::PrintVersion();
	GLenum res = glewInit();
	if (res != GLEW_OK)
	{
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}

	float fov = fov_degrees * (M_PI / 180.0f);
	perspectiveMatrix = cyMatrix4f::MatrixPerspective(fov, 1.0f, 0.1f, far_plane);
	buffer.Initialize(true, width, width);
	createObj(argv[1]);
	createPlane();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEBUG_OUTPUT);

	glutDisplayFunc(display);
	glutMouseFunc(onClick);
	glutMotionFunc(move);
	glutSpecialFunc(reset);

	glutMainLoop();

	return 0;

}