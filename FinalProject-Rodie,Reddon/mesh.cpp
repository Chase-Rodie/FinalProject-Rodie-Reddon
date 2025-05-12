#include "mesh.h"

Mesh::Mesh()
{
	// Vertex Set Up
	// No mesh

	// Model Set Up
	angle = 0.0f;
	pivotLocation = glm::vec3(0.f, 0.f, 0.f);
	model = glm::translate(glm::mat4(1.0f), pivotLocation);

	// Buffer Set Up
	if (!InitBuffers()) {
		printf("Some buffers not initialized.\n");
	}

}

Mesh::Mesh(glm::vec3 pivot, const char* fname)
{
	// Vertex Set Up
	loadModelFromFile(fname);

	// Model Set Up
	angle = 0.0f;
	pivotLocation = pivot;
	model = glm::translate(glm::mat4(1.0f), pivotLocation);

	// Buffer Set Up
	if (!InitBuffers()) {
		printf("some buffers not initialized.\n");
	}

	hasTex = false;
}

Mesh::Mesh(glm::vec3 pivot, const char* fname, const char* tname)
{
	// Vertex Set Up
	loadModelFromFile(fname);

	// Model Set Up
	angle = 0.0f;
	pivotLocation = pivot;
	model = glm::translate(glm::mat4(1.0f), pivotLocation);

	// Buffer Set Up
	if (!InitBuffers()) {
		printf("some buffers not initialized.\n");
	}

	// load texture from file
	m_texture = new Texture(tname);
	if (m_texture)
		hasTex = true;
	else
		hasTex = false;
}


Mesh::~Mesh()
{
	Vertices.clear();
	Indices.clear();
}

void Mesh::Update(glm::mat4 inmodel)
{
	model = inmodel;

}

glm::mat4 Mesh::GetModel()
{
	return model;
}


void Mesh::Render(GLint posAttribLoc, GLint normAttribLoc, GLint tcAttribLoc, GLint hasTextureLoc)
{
	glBindVertexArray(vao);
	// Enable vertex attibute arrays for each vertex attrib
	glEnableVertexAttribArray(posAttribLoc);
	glEnableVertexAttribArray(normAttribLoc);
	glEnableVertexAttribArray(tcAttribLoc);

	// Bind your VBO
	glBindBuffer(GL_ARRAY_BUFFER, VB);

	// Set vertex attribute pointers to the load correct data
	glVertexAttribPointer(posAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, vertex));
	glVertexAttribPointer(normAttribLoc, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glVertexAttribPointer(tcAttribLoc, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texcoord));

	// If has texture, set up texture unit(s) Update here to activate and assign texture unit
	if (m_texture != NULL) {
		glUniform1i(hasTextureLoc, true);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_texture->getTextureID());
	}
	else {
		glUniform1i(hasTextureLoc, false);
	}



	// Bind your Element Array
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);

	// Render
	glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);

	// Disable vertex arrays
	glDisableVertexAttribArray(posAttribLoc);
	glDisableVertexAttribArray(normAttribLoc);
	glDisableVertexAttribArray(tcAttribLoc);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


bool Mesh::InitBuffers() {

	// For OpenGL 3
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &VB);
	glBindBuffer(GL_ARRAY_BUFFER, VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * Vertices.size(), &Vertices[0], GL_STATIC_DRAW);


	glGenBuffers(1, &IB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * Indices.size(), &Indices[0], GL_STATIC_DRAW);

	return true;
}

bool Mesh::loadModelFromFile(const char* path) {
	Assimp::Importer importer;

	// Add aiProcess_GenNormals to ensure normals are generated if missing
	const aiScene* scene = importer.ReadFile(path,
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices);

	if (!scene) {
		printf("couldn't open the .obj file.\n");
		return false;
	}

	int iTotalVerts = 0;

	for (int i = 0; i < scene->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[i];

		if (!mesh->HasNormals()) {
			std::cout << "Warning: Mesh " << i << " has no normals!" << std::endl;
		}

		int iMeshFaces = mesh->mNumFaces;
		for (int j = 0; j < iMeshFaces; j++) {
			const aiFace& face = mesh->mFaces[j];
			for (int k = 0; k < 3; k++) {
				unsigned int index = face.mIndices[k];

				aiVector3D pos = mesh->mVertices[index];
				aiVector3D norm = mesh->mNormals ? mesh->mNormals[index] : aiVector3D(0, 0, 0);
				aiVector3D tex = mesh->HasTextureCoords(0) ? mesh->mTextureCoords[0][index] : aiVector3D(0, 0, 0);

				glm::vec3 position(pos.x, pos.y, pos.z);
				glm::vec3 normal(norm.x, norm.y, norm.z);
				glm::vec2 texCoord(tex.x, tex.y);  // Discard tex.z

				Vertices.push_back(Vertex(position, normal, texCoord));

				
				if (iTotalVerts + Vertices.size() < 5) {
					std::cout << "Vertex " << index
						<< " | Pos: (" << pos.x << ", " << pos.y << ", " << pos.z << ")"
						<< " | Normal: (" << norm.x << ", " << norm.y << ", " << norm.z << ")"
						<< std::endl;
				}
			}
		}

		iTotalVerts += mesh->mNumVertices;
	}

	for (int i = 0; i < Vertices.size(); i++) {
		Indices.push_back(i);
	}

	return true;
}


void Mesh::Rotate(float pitch, float yaw, float roll)
{
	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(roll), glm::vec3(0, 0, 1));
	rotation = glm::rotate(rotation, glm::radians(pitch), glm::vec3(1, 0, 0));
	rotation = glm::rotate(rotation, glm::radians(yaw), glm::vec3(0, 1, 0));

	glm::vec3 position = glm::vec3(model[3]);
	model[3] = glm::vec4(0, 0, 0, 1);
	model = rotation * model;
	model[3] = glm::vec4(position, 1.0f);
}


void Mesh::MoveForward(float amount) {
	glm::vec3 forward = glm::normalize(glm::vec3(model[2]));
	model = glm::translate(model, forward * amount);

}
