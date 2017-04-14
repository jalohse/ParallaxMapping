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
#define fov_degrees 90.0f
#define flip_degree 135.1f
#define y_trans -7

#define cube_front "front.png"
#define cube_back "back.png"
#define cube_left "left.png"
#define cube_right "right.png"
#define cube_top "top.png"
#define color_suffix "_COLOR.png"
#define disp_suffix "_DISP.png"
#define normal_suffix "_NRM.png"
#define spec_suffix "_SPEC.png"

std::vector<cyPoint3f> plane_vertices = {
	cyPoint3f(-1.5f, -1.5f, 0.0f),
	cyPoint3f(1.5f, -1.5f, 0.0f),
	cyPoint3f(-1.5f, 1.5f, 0.0f),
	cyPoint3f(-1.5f, 1.5f, 0.0f),
	cyPoint3f(1.5f, -1.5f, 0.0f),
	cyPoint3f(1.5f, 1.5f, 0.0f)
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
cyPoint3f cameraPos = cyPoint3f(0, -3, 5);
cyPoint3f upVec = cyPoint3f(0, 1, 0);
cyPoint3f lookAt = cyPoint3f(0, -3, 0);

GLuint vertexArrayObj;
GLuint planeVertexArrayObj;
GLuint cubeVertexArrayObj;
GLuint depthVertexArrayObj;

cy::GLSLProgram teapot_shaders;
cy::GLSLProgram plane_shaders;
cy::GLSLProgram cube_shaders;

cy::Matrix4<float> cameraTransformationMatrix;
cy::Matrix4<float> planeCameraTransformationMatrix;
cy::Matrix4<float> lightCameraTransformationMatrix;
cy::Matrix4<float> perspectiveMatrix;
cy::Matrix4<float> totalRotationMatrix;
cy::Matrix4<float> translationMatrix;
cy::Matrix4<float> cubeTranslationMatrix;
cy::Matrix4<float> planeTransformationMatrix;
cy::Matrix4<float> lightTransformationMatrix;
cy::Matrix4<float> lightRotationMatrix;
cyMatrix4f teapotLightMVP;
cyMatrix4f planeLightMVP;

int selected;
int selectedKey;

std::vector<cyPoint3f> vertices;
std::vector<cyPoint3f> lightVertices;

std::vector<cyPoint3f> cube_vertices;
std::vector<cyPoint3f> cube_normals;
std::vector<cyPoint3f> cube_texture_vertices;
std::vector<unsigned char> cube_front_img;
std::vector<unsigned char> cube_back_img;
std::vector<unsigned char> cube_left_img;
std::vector<unsigned char> cube_right_img;
std::vector<unsigned char> cube_top_img;

GLuint textureID[3];
GLuint cubeTexId[3];

unsigned diffWidth, diffHeight, specHeight, specWidth;
unsigned cubeWidth, cubeHeight;

cyMatrix4f view = cyMatrix4f::MatrixView(cameraPos, lookAt, upVec);
cyMatrix4f lightView = cyMatrix4f::MatrixView(lightPos, lookAt, upVec);
cyMatrix4f lightProj = cyMatrix4f::MatrixPerspective(M_PI / 8, 1, 20, 200);
cyMatrix4f bias = cyMatrix4f::MatrixTrans(cyPoint3f(0.5f, 0.5f, 0.495f)) * cyMatrix4f::MatrixScale(0.5f, 0.5f, 0.5f);
bool zoom_in = true;
int movement = 0;




void setInitialRotationAndTranslation() {
	cy::Matrix4<float> rotationZ = cyMatrix4f::MatrixRotationZ(0);
	cy::Matrix4<float> rotationX = cyMatrix4f::MatrixRotationX(0);
	cyPoint3f translation = cyPoint3f(0,-5,0);
	translationMatrix = cyMatrix4f::MatrixTrans(translation);

	totalRotationMatrix = rotationX * rotationZ;
	cameraTransformationMatrix = translationMatrix * totalRotationMatrix;
	planeCameraTransformationMatrix = cyMatrix4f::MatrixTrans(cyPoint3f(0, -7, -10));

	lightRotationMatrix = cyMatrix4f::MatrixRotationX(0);
	teapotLightMVP = lightProj * lightView * cameraTransformationMatrix;
	planeLightMVP = lightProj * lightView * planeCameraTransformationMatrix;
}

void display() {


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cube_shaders.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexId[0]);
	glUniform1i(glGetUniformLocation(cube_shaders.GetID(), "diffuseMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexId[1]);
	glUniform1i(glGetUniformLocation(cube_shaders.GetID(), "normalMap"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexId[2]);
	glUniform1i(glGetUniformLocation(cube_shaders.GetID(), "depthMap"), 2);
	glDepthMask(false);
	glBindVertexArray(cubeVertexArrayObj);
	glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());
	glDepthMask(true);

	teapot_shaders.Bind();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID[0]);
	glUniform1i(glGetUniformLocation(teapot_shaders.GetID(), "diffuseMap"), 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureID[1]);
	glUniform1i(glGetUniformLocation(teapot_shaders.GetID(), "normalMap"), 1);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureID[2]);
	glUniform1i(glGetUniformLocation(teapot_shaders.GetID(), "depthMap"), 2);
	glBindVertexArray(vertexArrayObj);
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

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
	cubeTranslationMatrix = cyMatrix4f::MatrixTrans(translation) * cubeTranslationMatrix * totalRotationMatrix;
	cameraTransformationMatrix = translationMatrix * totalRotationMatrix;
	teapot_shaders.SetUniform(1, cameraTransformationMatrix);
	teapot_shaders.SetUniform(3, cameraTransformationMatrix.GetInverse().GetTranspose());
	cube_shaders.Bind();
	cube_shaders.SetUniform(1, cubeTranslationMatrix * totalRotationMatrix);
	glutPostRedisplay();
}

void rotatePlane() {
	plane_shaders.Bind();
	totalPlaneRotationMatrix = cyMatrix4f::MatrixRotationX(0.1) * totalPlaneRotationMatrix;
	planeCameraTransformationMatrix = planeTransformationMatrix * totalPlaneRotationMatrix;
	plane_shaders.SetUniform(1, planeCameraTransformationMatrix);
	glutPostRedisplay();
}

void rotate() {
	totalRotationMatrix = cyMatrix4f::MatrixRotationY(0.5) * totalRotationMatrix;
	cameraTransformationMatrix = translationMatrix * totalRotationMatrix;
	teapot_shaders.Bind();
	teapot_shaders.SetUniform(1, cameraTransformationMatrix);
	teapot_shaders.SetUniform(3, cameraTransformationMatrix.GetInverse().GetTranspose());
	cube_shaders.Bind();
	cube_shaders.SetUniform(1, cubeTranslationMatrix * totalRotationMatrix);
	glutPostRedisplay();
}


void updateLightPosition() {
	lightView = cyMatrix4f::MatrixView(lightPos, cyPoint3f(0, 0, 0), cyPoint3f(0, 1, 0));
	teapotLightMVP = lightProj * lightView * cameraTransformationMatrix;
	planeLightMVP = lightProj * lightView * planeCameraTransformationMatrix;
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


std::vector<cyPoint3f> calculateTangentsOfPlane(std::vector<cyPoint3f> vertices, std::vector<cyPoint3f> texture_vertices)
{
	cyPoint3f edge1 = vertices.at(1) - vertices.at(0);
	cyPoint3f edge2 = vertices.at(2) - vertices.at(0);
	cyPoint3f tex_change1 = texture_vertices.at(1) - texture_vertices.at(0);
	cyPoint3f tex_change2 = texture_vertices.at(2) - texture_vertices.at(0);
	float det = tex_change1.x * tex_change2.y - tex_change2.x * tex_change1.y;
	cyPoint3f tangent;
	cyPoint3f bitangent;
	if (det == 0)
	{
		tangent = cyPoint3f(1.0f, 0.0f, 0.0f);
		bitangent = cyPoint3f(0.0f, 1.0f, 0.0f);
	}
	else {
		float determinant_tri1 = 1.0f / det;
		tangent = cyPoint3f(determinant_tri1 * (tex_change2.y * edge1.x - tex_change1.y * edge2.x),
			determinant_tri1 * (tex_change2.y * edge1.y - tex_change1.y * edge2.y),
			determinant_tri1 * (tex_change2.y * edge1.z - tex_change1.y * edge2.z));
		tangent = tangent.GetNormalized();

		bitangent = cyPoint3f(determinant_tri1 * (-tex_change2.x * edge1.x + tex_change1.x * edge2.x),
			determinant_tri1 * (-tex_change2.x * edge1.y + tex_change1.x * edge2.y),
			determinant_tri1 * (-tex_change2.x * edge1.z + tex_change1.x * edge2.z));
		bitangent = bitangent.GetNormalized();
	}

	return std::vector<cyPoint3f>{tangent, bitangent};
}

void calculateTangents(std::vector<cyPoint3f> vertices, std::vector<cyPoint3f> texture_vertices, std::vector<cyPoint3f> &tangents, std::vector<cyPoint3f> &bitangents)
{
	for (int i = 0; i < vertices.size(); i = i + 3)
	{
		std::vector<cyPoint3f> points = { vertices.at(i), vertices.at(i + 1), vertices.at(i + 2) };
		std::vector<cyPoint3f> tex_points = { texture_vertices.at(i), texture_vertices.at(i + 1), texture_vertices.at(i + 2) };
		std::vector<cyPoint3f> tangentsBitangents = calculateTangentsOfPlane(points, tex_points);
		for (int j = 0; j < 3; j++)
		{
			tangents.push_back(tangentsBitangents.at(0));
			bitangents.push_back(tangentsBitangents.at(1));
		}

	}
}

std::vector<unsigned char> generateImage(std::string image, unsigned &im_width, unsigned &im_height) {
	std::vector<unsigned char> diffuseVector;
	lodepng::decode(diffuseVector, im_width, im_height, image);
	return diffuseVector;
}

void loadTextures(GLuint id[], std::string img_name) {
	std::string color_img = img_name + color_suffix;
	std::string normal_img = img_name + normal_suffix;
	std::string disp_img = img_name + disp_suffix;
	std::vector<unsigned char> diffuseImage = generateImage(color_img, diffWidth, diffHeight);
	std::vector<unsigned char> normalImage = generateImage(normal_img, specWidth, specHeight);
	std::vector<unsigned char> displacementImage = generateImage(disp_img, specWidth, specHeight);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, id[0]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, diffWidth, diffHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &diffuseImage[0]);

	glBindTexture(GL_TEXTURE_2D, id[1]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, specWidth, specHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &normalImage[0]);

	glBindTexture(GL_TEXTURE_2D, id[2]);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_NEAREST = no smoothing
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 4, specWidth, specHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &displacementImage[0]);

	glBindTexture(GL_TEXTURE_2D, 0);

}

void populateVerticesAndNormals(cyTriMesh mesh, std::vector<cyPoint3f> &normals, std::vector<cyPoint3f> &textureVertices) {
	
	for (int i = 0; i < mesh.NF(); i = i + 1) {
		cy::TriMesh::TriFace face = mesh.F(i);
		vertices.push_back(mesh.V(face.v[0]));
		vertices.push_back(mesh.V(face.v[1]));
		vertices.push_back(mesh.V(face.v[2]));
		cy::TriMesh::TriFace nface = mesh.FN(i);
		normals.push_back(mesh.VN(nface.v[0]).GetNormalized());
		normals.push_back(mesh.VN(nface.v[1]).GetNormalized());
		normals.push_back(mesh.VN(nface.v[2]).GetNormalized());
		cy::TriMesh::TriFace texface = mesh.FT(i);
		textureVertices.push_back(mesh.VT(texface.v[0]));
		textureVertices.push_back(mesh.VT(texface.v[1]));
		textureVertices.push_back(mesh.VT(texface.v[2]));
	}
}


void createFountain() {
	cyTriMesh mesh = cyTriMesh();
	mesh.LoadFromFileObj("fountain.obj");
	mesh.ComputeBoundingBox();
	mesh.ComputeNormals();

	std::vector<cyPoint3f> normals;
	std::vector<cyPoint3f> textureVertices;

	populateVerticesAndNormals(mesh, normals, textureVertices);
	setInitialRotationAndTranslation();

	teapot_shaders = cy::GLSLProgram();
	teapot_shaders.BuildFiles("plane_vertex_shader.glsl", "plane_fragment_shader.glsl");
	teapot_shaders.Bind();
	teapot_shaders.RegisterUniform(1, "cameraTransformation");
	teapot_shaders.SetUniform(1, cameraTransformationMatrix);
	teapot_shaders.RegisterUniform(2, "perspective");
	teapot_shaders.SetUniform(2, perspectiveMatrix);
	teapot_shaders.RegisterUniform(3, "view");
	teapot_shaders.SetUniform(3, view);
	teapot_shaders.RegisterUniform(4, "lightPos");
	teapot_shaders.SetUniform(4, lightPos);
	teapot_shaders.RegisterUniform(5, "cameraPos");
	teapot_shaders.SetUniform(5, cameraPos);

	GLuint vertexBufferObj[2];
	GLuint textureVertexBufferObj[1];
	GLuint vertexTangentObj[1];
	GLuint vertexBitangentObj[1];


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

	glGenBuffers(1, textureVertexBufferObj);
	glBindBuffer(GL_ARRAY_BUFFER, textureVertexBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * textureVertices.size(), &textureVertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	std::vector<cyPoint3f> tangents;
	std::vector<cyPoint3f> bitangents;

	calculateTangents(vertices, textureVertices, tangents, bitangents);

	glGenBuffers(1, vertexTangentObj);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTangentObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * tangents.size(), &tangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glGenBuffers(1, vertexBitangentObj);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBitangentObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * bitangents.size(), &bitangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindVertexArray(0);

	glGenTextures(3, textureID);
	loadTextures(textureID, "stone");
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

	glGenTextures(3, textureID);

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

	std::vector<cyPoint3f> tangentsBitangents = calculateTangentsOfPlane(plane_vertices, planeTextureVertices);

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
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindVertexArray(0);
}


void loadCubeTextures(int num) {
	if(cube_front_img.empty())
	{
		cube_front_img = generateImage(cube_front, cubeWidth, cubeHeight);
	}
	if (cube_back_img.empty())
	{
		cube_back_img = generateImage(cube_back, cubeWidth, cubeHeight);
	}
	if (cube_left_img.empty())
	{
		cube_left_img = generateImage(cube_left, cubeWidth, cubeHeight);
	}
	if (cube_right_img.empty())
	{
		cube_right_img = generateImage(cube_right, cubeWidth, cubeHeight);
	}
	if (cube_top_img.empty())
	{
		cube_top_img = generateImage(cube_top, cubeWidth, cubeHeight);
	}
	std::string img_name = "floor";
	if(num == 0)
	{
		img_name += color_suffix;
	} else if(num == 1)
	{
		img_name += normal_suffix;
	} else if(num == 2)
	{
		img_name += disp_suffix;
	}
	std::vector<unsigned char> floor = generateImage(img_name, cubeWidth, cubeHeight);


	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeTexId[num]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, 4, cubeWidth, cubeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &cube_left_img[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, 4, cubeWidth, cubeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &cube_right_img[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, 4, cubeWidth, cubeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &cube_top_img[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, 4, cubeWidth, cubeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &floor[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, 4, cubeWidth, cubeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &cube_back_img[0]);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, 4, cubeWidth, cubeHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &cube_front_img[0]);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

void populateCubeVertices(cyTriMesh box) {
	cube_vertices = {};
	for (int i = 0; i < box.NF(); i = i + 1) {
		cy::TriMesh::TriFace face = box.F(i);
		cube_vertices.push_back(box.V(face.v[0]));
		cube_vertices.push_back(box.V(face.v[1]));
		cube_vertices.push_back(box.V(face.v[2]));
		cy::TriMesh::TriFace nface = box.FN(i);
		cube_normals.push_back(box.VN(nface.v[0]).GetNormalized());
		cube_normals.push_back(box.VN(nface.v[1]).GetNormalized());
		cube_normals.push_back(box.VN(nface.v[2]).GetNormalized());
	}
}

void createSceneBox()
{
	cyTriMesh box = cyTriMesh();
	box.LoadFromFileObj("cube.obj");
	box.ComputeBoundingBox();
	box.ComputeNormals();

	cubeTranslationMatrix = cyMatrix4f::MatrixTrans(cyPoint3f(0, 0, 0));

	cube_shaders = cy::GLSLProgram();
	cube_shaders.BuildFiles("cube_vertex_shader.glsl", "cube_fragment_shader.glsl");
	cube_shaders.Bind();
	cube_shaders.RegisterUniform(1, "cameraTransformation");
	cube_shaders.SetUniform(1, cubeTranslationMatrix * totalRotationMatrix);
	cube_shaders.RegisterUniform(2, "perspective");
	cube_shaders.SetUniform(2, perspectiveMatrix);
	cube_shaders.RegisterUniform(3, "view");
	cube_shaders.SetUniform(3, view);
	cube_shaders.RegisterUniform(4, "lightPos");
	cube_shaders.SetUniform(4, lightPos);
	cube_shaders.RegisterUniform(5, "cameraPos");
	cube_shaders.SetUniform(5, cameraPos);

	populateCubeVertices(box);

	GLuint vertexBufferObj[2];
	GLuint vertexTangentObj[1];
	GLuint vertexBitangentObj[1];

	glGenVertexArrays(1, &cubeVertexArrayObj);
	glBindVertexArray(cubeVertexArrayObj);

	glGenBuffers(1, vertexBufferObj);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * cube_vertices.size(), &cube_vertices[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * cube_normals.size(), &cube_normals[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	std::vector<cyPoint3f> tangents;
	std::vector<cyPoint3f> bitangents;

	calculateTangents(cube_vertices, cube_vertices, tangents, bitangents);

	glGenBuffers(1, vertexTangentObj);
	glBindBuffer(GL_ARRAY_BUFFER, vertexTangentObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * tangents.size(), &tangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glGenBuffers(1, vertexBitangentObj);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBitangentObj[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cyPoint3f) * bitangents.size(), &bitangents[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, 0, sizeof(cyPoint3f), NULL);

	glBindVertexArray(0);

	glGenTextures(3, cubeTexId);
	loadCubeTextures(0);
	loadCubeTextures(1);
	loadCubeTextures(2);
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
	createFountain();
	createSceneBox();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEBUG_OUTPUT);

	glutDisplayFunc(display);
	glutMouseFunc(onClick);
	glutMotionFunc(move);
	glutSpecialFunc(reset);

	glutMainLoop();

	return 0;

}