#include "graphics.h"
#include <glm/gtx/string_cast.hpp> 
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::vector<CelestialBody> planets;
std::vector<Sphere*> planetSpheres;
std::vector<Moon> moons;
Comet halleysComet;


Graphics::Graphics()
{

}

Graphics::~Graphics()
{

}

bool Graphics::Initialize(int width, int height)
{
	currentMode = GameMode::Exploration; // Default mode

	// Used for the linux OS
#if !defined(__APPLE__) && !defined(MACOSX)
  // cout << glewGetString(GLEW_VERSION) << endl;
	glewExperimental = GL_TRUE;

	auto status = glewInit();

	// This is here to grab the error that comes from glew init.
	// This error is an GL_INVALID_ENUM that has no effects on the performance
	glGetError();

	//Check for error
	if (status != GLEW_OK)
	{
		std::cerr << "GLEW Error: " << glewGetErrorString(status) << "\n";
		return false;
	}
#endif



	// Init Camera
	m_camera = new Camera();
	if (!m_camera->Initialize(width, height))
	{
		printf("Camera Failed to Initialize\n");
		return false;
	}

	// Set up the shaders
	m_shader = new Shader();
	if (!m_shader->Initialize())
	{
		printf("Shader Failed to Initialize\n");
		return false;
	}

	// Add the vertex shader
	if (!m_shader->AddShader(GL_VERTEX_SHADER))
	{
		printf("Vertex Shader failed to Initialize\n");
		return false;
	}

	// Add the fragment shader
	if (!m_shader->AddShader(GL_FRAGMENT_SHADER))
	{
		printf("Fragment Shader failed to Initialize\n");
		return false;
	}

	// Connect the program
	if (!m_shader->Finalize())
	{
		printf("Program to Finalize\n");
		return false;
	}

	// Populate location bindings of the shader uniform/attribs
	if (!collectShPrLocs()) {
		printf("Some shader attribs not located!\n");
	}

	// Skybox Shader
	skyboxShader = new Shader();
	skyboxShader->Initialize();
	const char* skyboxVertexShader = R"(
#version 460
layout (location = 0) in vec3 aPos;
out vec3 TexCoords;
uniform mat4 projection;
uniform mat4 view;
void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}
)";

	const char* skyboxFragmentShader = R"(
#version 460
in vec3 TexCoords;
out vec4 FragColor;
uniform samplerCube skybox;
void main()
{
    FragColor = texture(skybox, TexCoords);
}
)";

	skyboxShader->AddShader(GL_VERTEX_SHADER, skyboxVertexShader);
	skyboxShader->AddShader(GL_FRAGMENT_SHADER, skyboxFragmentShader);

	skyboxShader->Finalize();

	float skyboxVertices[] = {
		-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f
	};
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	std::vector<std::string> faces = {
		"assets/skybox_right.jpg",   // POSITIVE_X
		"assets/skybox_left.jpg",    // NEGATIVE_X
		"assets/skybox_top.jpg",     // POSITIVE_Y
		"assets/skybox_bottom.jpg",  // NEGATIVE_Y
		"assets/skybox_front.jpg",   // POSITIVE_Z
		"assets/skybox_back.jpg"     // NEGATIVE_Z
	};
	cubemapTexture = loadCubemap(faces);


	// Starship
	m_mesh = new Mesh(glm::vec3(0.0f), "assets\\SpaceShip-1.obj", "assets\\SpaceShip-1.png");
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) *
		glm::scale(glm::vec3(0.025f));
	m_mesh->Update(model);


	// The Sun
	m_sphere = new Sphere(64, "assets\\2k_sun.jpg");

	// Create a single asteroid mesh
	m_asteroid = new Mesh(glm::vec3(0.0f), "assets\\asteroid.obj", "assets\\asteroid.jpg");
	GenerateAsteroidBelts();
	SetupAsteroidInstancing();

	
	planets = {
		{ "Mercury", 2.0f, 4.74f, 10.83f, 0.2f, 0.01f, "assets/Mercury.jpg", glm::vec3(10.0f, 5.0f, 0.0f), glm::vec3(0.01f) },
		{ "Venus",   3.0f, 3.5f, -6.52f, 0.45f, 177.4f, "assets/Venus.jpg", glm::vec3(10.0f, 5.0f, 0.0f), glm::vec3(0.01f) },
		{ "Earth",   4.0f, 2.98f, 15.0f, 0.5f, 23.5f, "assets/2k_earth_daymap.jpg", glm::vec3(10.0f, 5.0f, 0.0f), glm::vec3(0.01f) },
		{ "Mars",    5.0f, 2.41f, 14.6f, 0.35f, 25.0f, "assets/Mars.jpg", glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.05f, 0.2f) },
		{ "Jupiter", 7.0f, 1.31f, 25.0f, 1.0f, 3.1f, "assets/Jupiter.jpg", glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.05f, 0.2f) },
		{ "Saturn",  9.0f, 0.97f, 22.0f, 0.9f, 26.7f, "assets/Saturn.jpg", glm::vec3(0.0f, 10.0f, 0.0f), glm::vec3(0.0f, 0.05f, 0.2f) },
		{ "Uranus",  11.0f, 0.68f, -17.2f, 0.7f, 97.8f, "assets/Uranus.jpg", glm::vec3(0.0f, 1.0f, 10.0f), glm::vec3(0.3f, 0.3f, 1.0f) },
		{ "Neptune", 13.0f, 0.54f, 16.1f, 0.65f, 28.3f, "assets/Neptune.jpg", glm::vec3(0.0f, 1.0f, 10.0f), glm::vec3(0.3f, 0.3f, 1.0f) }
	};


	for (const auto& p : planets) {
		Sphere* s = new Sphere(48, p.texturePath.c_str());
		planetSpheres.push_back(s);
	}

	// Earth's Moon
	moons.push_back({ 2, new Sphere(32, "assets/2k_moon.jpg"), 1.5f, 3.0f, 0.2f, 30.f, "" });


	// Mars: Phobos & Deimos
	moons.push_back({ 3, new Sphere(32, "assets/Mercury.jpg"), 0.6f, 4.5f, 0.05f, 20.f, "" });  // Phobos-like
	moons.push_back({ 3, new Sphere(32, "assets/Mercury.jpg"), 1.0f, 3.2f, 0.07f, -10.f, "" }); // Deimos-like

	// Jupiter: Europa & Ganymede
	moons.push_back({ 4, new Sphere(32, "assets/Mercury.jpg"), 1.5f, 3.0f, 0.1f, 15.f, "" });   // Europa
	moons.push_back({ 4, new Sphere(32, "assets/Mercury.jpg"), 2.2f, 2.2f, 0.12f, -15.f, "" });  // Ganymede

	halleysComet = {
	new Sphere(32, "assets/2k_moon.jpg"),
	20.0f,  
	6.0f,   
	0.2f,   
	0.15f,  
	1.0f    
	};


	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return true;
}

void Graphics::HierarchicalUpdate2(double dt) {
	totalTime += dt;  
	modelStack = std::stack<glm::mat4>();
	glm::mat4 identity = glm::mat4(1.0f);
	modelStack.push(identity);

	// 1. Sun: rotate in place at origin
	glm::mat4 sunRotation = glm::rotate(identity, (float)(totalTime * 0.2f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 sunScale = glm::scale(glm::vec3(1.5f)); // slightly bigger sun
	glm::mat4 sunModel = modelStack.top() * sunRotation * sunScale;
	m_sphere->Update(sunModel);

	for (size_t i = 0; i < planets.size(); ++i) {
		const CelestialBody& p = planets[i];

		glm::mat4 orbit = glm::rotate(glm::mat4(1.0f), (float)(totalTime * p.orbitSpeed), glm::vec3(0, 1, 0));
		glm::mat4 translate = glm::translate(glm::vec3(p.orbitRadius, 0.0f, 0.0f));
		glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), glm::radians(p.axialTilt), glm::vec3(0, 0, 1));
		glm::mat4 spin = glm::rotate(glm::mat4(1.0f), (float)(totalTime * p.rotationSpeed), glm::vec3(0, 1, 0));
		glm::mat4 scale = glm::scale(glm::vec3(p.scale));

		glm::mat4 model = modelStack.top() * orbit * translate * tilt * spin * scale;
		planetSpheres[i]->Update(model);
	}

	for (Moon& m : moons) {
		glm::mat4 planetModel = planetSpheres[m.parentPlanetIndex]->GetModel();
		glm::mat4 moonTilt = glm::rotate(glm::mat4(1.0f), glm::radians(m.tilt), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 moonOrbit = glm::rotate(glm::mat4(1.0f), (float)(totalTime * m.orbitSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 moonTranslate = glm::translate(glm::vec3(m.orbitRadius, 0.0f, 0.0f));
		glm::mat4 moonScale = glm::scale(glm::vec3(m.scale));
		glm::mat4 moonModel = planetModel * moonTilt * moonOrbit * moonTranslate * moonScale;
		m.sphere->Update(moonModel);
	}


	float flySpeed = halleysComet.speed;
	float radiusX = 20.0f;
	float radiusZ = 35.0f;
	float angle = totalTime * 0.3f;
	cometVelocity = glm::normalize(currentCometPosition - previousCometPosition);
	float x = radiusX * cos(angle);
	float z = radiusZ * sin(angle);


	glm::vec3 cometPos = glm::vec3(x, 0.0f, z);


	// Comet faces away from the Sun
	glm::vec3 sunToComet = glm::normalize(cometPos - glm::vec3(0.0f));
	glm::vec3 cometForward = -sunToComet;  // tail points away
	glm::vec3 right = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cometForward));
	glm::vec3 up = glm::normalize(glm::cross(cometForward, right));

	glm::mat4 orientation = glm::mat4(1.0f);
	orientation[0] = glm::vec4(right, 0.0f);
	orientation[1] = glm::vec4(up, 0.0f);
	orientation[2] = glm::vec4(cometForward, 0.0f);

glm::vec3 cometRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), cometForward));
glm::vec3 cometUp = glm::normalize(glm::cross(cometForward, cometRight));

glm::mat4 cometOrientation = glm::mat4(1.0f);
cometOrientation[0] = glm::vec4(cometRight, 0.0f);
cometOrientation[1] = glm::vec4(cometUp, 0.0f);
cometOrientation[2] = glm::vec4(cometForward, 0.0f);
cometOrientation[3] = glm::vec4(0, 0, 0, 1);
currentCometPosition = glm::vec3(x, 0.0f, z);  

glm::mat4 cometModel = glm::translate(glm::mat4(1.0f), cometPos) *
cometOrientation *
glm::scale(glm::vec3(1.0f)); 
halleysComet.body->Update(cometModel);


	modelStack.pop(); // pop planet

	previousCometPosition = currentCometPosition;

	// Update the comet trail
	cometTrailPositions.push_front(currentCometPosition);  // new head of the trail

	// Keep only the most recent N positions
	if (cometTrailPositions.size() > maxTrailLength)
		cometTrailPositions.pop_back();

}




void Graphics::ComputeTransforms(double dt, std::vector<float> speed, std::vector<float> dist,
	std::vector<float> rotSpeed, glm::vec3 rotVector, std::vector<float> scale, glm::mat4& tmat, glm::mat4& rmat, glm::mat4& smat) {
	tmat = glm::translate(glm::mat4(1.f),
		glm::vec3(cos(speed[0] * dt) * dist[0], sin(speed[1] * dt) * dist[1], sin(speed[2] * dt) * dist[2])
	);
	rmat = glm::rotate(glm::mat4(1.f), rotSpeed[0] * (float)dt, rotVector);
	smat = glm::scale(glm::vec3(scale[0], scale[1], scale[2]));
}

void Graphics::Render()
{

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_shader->Enable();  

	glm::vec3 lightDir = glm::normalize(glm::vec3(1.0, -1.0, -1.0));
	glm::vec3 nightDir = -lightDir;


	glm::vec3 ambientColor = glm::vec3(0.3f);  
	glm::vec3 overrideColor = glm::vec3(0.0f); 

	if (m_lightDir != -1) glUniform3fv(m_lightDir, 1, glm::value_ptr(lightDir));
	if (m_ambientColor != -1) glUniform3fv(m_ambientColor, 1, glm::value_ptr(ambientColor));
	if (m_overrideColor != -1) glUniform3fv(m_overrideColor, 1, glm::value_ptr(overrideColor));

	GLint locNightColor = m_shader->GetUniformLocation("nightColor");
	GLint locLightDir = m_shader->GetUniformLocation("lightDir");
	GLint locNightDir = m_shader->GetUniformLocation("nightDir");

	if (locLightDir != -1) glUniform3fv(locLightDir, 1, glm::value_ptr(lightDir));
	if (locNightDir != -1) glUniform3fv(locNightDir, 1, glm::value_ptr(nightDir));


	// --- SKYBOX ---
	glDepthFunc(GL_LEQUAL);
	skyboxShader->Enable();

	glm::mat4 view = glm::mat4(glm::mat3(m_camera->GetView())); // remove translation
	glm::mat4 projection = m_camera->GetProjection();

	glUniformMatrix4fv(skyboxShader->GetUniformLocation("view"), 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(skyboxShader->GetUniformLocation("projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform1i(skyboxShader->GetUniformLocation("skybox"), 0);

	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS);  // reset depth


	
	m_shader->Enable();

	glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
	glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

	
	if (m_mesh != NULL) {
		glUniform1i(m_hasTexture, false);
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_mesh->GetModel()));

		if (m_mesh->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mesh->getTextureID()); 

			GLuint sampler = m_shader->GetUniformLocation("sp");
			if (sampler == INVALID_UNIFORM_LOCATION) {
				printf("Sampler not found\n");
			}
			glUniform1i(sampler, 0);
		}

		m_mesh->Render(m_positionAttrib, m_normalAttrib, m_tcAttrib, m_hasTexture);
	}

	
	int count = std::min(100, static_cast<int>(innerAsteroidTransforms.size()));
	for (int i = 0; i < count; ++i) {
		m_asteroid->Update(innerAsteroidTransforms[i]);

		glUniform1i(m_shader->GetUniformLocation("isInstanced"), false);
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_asteroid->GetModel()));

		if (m_asteroid->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_asteroid->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			glUniform1i(sampler, 0);
		}

		m_asteroid->Render(m_positionAttrib, m_normalAttrib, m_tcAttrib, m_hasTexture);
	}


	
	std::cout << "lightDir = " << glm::to_string(lightDir) << std::endl;

	
	if (!innerAsteroidTransforms.empty()) {
		
		glUniform1i(m_hasTexture, true);
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_asteroid->getTextureID());
		GLuint sampler = m_shader->GetUniformLocation("sp");
		glUniform1i(sampler, 0);

		glBindVertexArray(m_asteroid->getVAO());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_asteroid->getIBO());

		glDrawElementsInstanced(GL_TRIANGLES, m_asteroid->GetIndexCount(), GL_UNSIGNED_INT, 0, innerAsteroidTransforms.size());

	}

	
	
	int outerCount = std::min(10000, static_cast<int>(outerAsteroidTransforms.size()));
	for (int i = 0; i < outerCount; ++i) {
		glm::mat4 scaled = outerAsteroidTransforms[i] * glm::scale(glm::vec3(1.0f)); // or whatever size you want
		m_asteroid->Update(scaled);

		glUniform1i(m_shader->GetUniformLocation("isInstanced"), false);
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_asteroid->GetModel()));

		if (m_asteroid->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_asteroid->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			glUniform1i(sampler, 0);
		}

		m_asteroid->Render(m_positionAttrib, m_normalAttrib, m_tcAttrib, m_hasTexture);
	}

	
	if (m_sphere != NULL) {
		GLint emissiveLoc = m_shader->GetUniformLocation("isEmissive");
		glUniform1i(emissiveLoc, true); // make Sun emissive

		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_sphere->GetModel()));
		if (m_sphere->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_sphere->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			glUniform1i(sampler, 0);
		}
		m_sphere->Render(m_positionAttrib, m_normalAttrib, m_tcAttrib, m_hasTexture);

		glUniform1i(emissiveLoc, false); // reset for other objects
	}



	glm::vec3 sunPos = glm::vec3(0.0f); // Sun is at origin

	for (size_t i = 0; i < planetSpheres.size(); ++i) {
		Sphere* planet = planetSpheres[i];
		const std::string& name = planets[i].name;
		glm::mat4 model = planet->GetModel();
		glm::vec3 objPos = glm::vec3(model[3]);
		glm::vec3 lightDir = glm::normalize(objPos - sunPos);
		glm::vec3 nightDir = -lightDir;

		glm::vec3 lightColor, nightColor;

		if (name == "Mercury" || name == "Venus" || name == "Earth") {
			lightColor = glm::vec3(1.0f, 0.8f, 0.4f);       // warm white
			nightColor = glm::vec3(0.05f);                 // soft ambient
		}
		else if (name == "Mars" || name == "Jupiter" || name == "Saturn") {
			lightColor = glm::vec3(0.6f, 0.6f, 0.5f);       
			nightColor = glm::vec3(0.02f, 0.05f, 0.08f);
		}
		else if (name == "Uranus" || name == "Neptune") {
			lightColor = glm::vec3(0.2f, 0.4f, 1.0f);       // soft blue
			nightColor = glm::vec3(0.1f, 0.1f, 0.2f);
		}


		glUniform3fv(m_lightColor, 1, glm::value_ptr(lightColor));
		glUniform3fv(m_nightColor, 1, glm::value_ptr(nightColor));
		glUniform3fv(m_ambientColor, 1, glm::value_ptr(ambientColor));
		glUniform3fv(m_lightDir, 1, glm::value_ptr(lightDir));
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(model));

		if (planet->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, planet->getTextureID());
			glUniform1i(m_shader->GetUniformLocation("sp"), 0);
		}

		planet->Render(m_positionAttrib, m_normalAttrib, m_tcAttrib, m_hasTexture);
	}






	for (Moon& m : moons) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m.sphere->GetModel()));
		if (m.sphere->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m.sphere->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			glUniform1i(sampler, 0);
		}
		m.sphere->Render(m_positionAttrib, m_normalAttrib, m_tcAttrib, m_hasTexture);
	}

	glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(halleysComet.body->GetModel()));
	if (halleysComet.body->hasTex) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, halleysComet.body->getTextureID());
		GLuint sampler = m_shader->GetUniformLocation("sp");
		glUniform1i(sampler, 0);
	}
	
	RenderCometTail(currentCometPosition, glm::vec3(0.0f)); 

	
	auto error = glGetError();
	if (error != GL_NO_ERROR)
	{
		string val = ErrorString(error);
		
	}
}


bool Graphics::collectShPrLocs() {

	m_lightColor = m_shader->GetUniformLocation("lightColor");
	m_lightDir = m_shader->GetUniformLocation("lightDir");
	m_ambientColor = m_shader->GetUniformLocation("ambientColor");
	m_overrideColor = m_shader->GetUniformLocation("overrideColor");
	m_nightColor = m_shader->GetUniformLocation("nightColor");
	m_nightDir = m_shader->GetUniformLocation("nightDir");




	bool anyProblem = true;
	// Locate the projection matrix in the shader
	m_projectionMatrix = m_shader->GetUniformLocation("projectionMatrix");
	if (m_projectionMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_projectionMatrix not found\n");
		anyProblem = false;
	}

	m_overrideColor = m_shader->GetUniformLocation("overrideColor");
	if (m_overrideColor == INVALID_UNIFORM_LOCATION)
	{
		printf("overrideColor uniform not found\n");
		anyProblem = false;
	}

	// Locate the view matrix in the shader
	m_viewMatrix = m_shader->GetUniformLocation("viewMatrix");
	if (m_viewMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_viewMatrix not found\n");
		anyProblem = false;
	}

	// Locate the model matrix in the shader
	m_modelMatrix = m_shader->GetUniformLocation("modelMatrix");
	if (m_modelMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_modelMatrix not found\n");
		anyProblem = false;
	}

	m_positionAttrib = m_shader->GetAttribLocation("v_position");
	if (m_positionAttrib == -1)
	{
		printf("v_position attribute not found\n");
		anyProblem = false;
	}

	// Locate the color vertex attribute
	m_normalAttrib = m_shader->GetAttribLocation("v_normal");


	// Locate the color vertex attribute
	m_tcAttrib = m_shader->GetAttribLocation("v_tc");
	if (m_tcAttrib == -1)
	{
		printf("v_texcoord attribute not found\n");
		anyProblem = false;
	}

	m_hasTexture = m_shader->GetUniformLocation("hasTexture");
	if (m_hasTexture == INVALID_UNIFORM_LOCATION) {
		printf("hasTexture uniform not found\n");
		anyProblem = false;
	}

	return anyProblem;
}

std::string Graphics::ErrorString(GLenum error)
{
	if (error == GL_INVALID_ENUM)
	{
		return "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
	}

	else if (error == GL_INVALID_VALUE)
	{
		return "GL_INVALID_VALUE: A numeric argument is out of range.";
	}

	else if (error == GL_INVALID_OPERATION)
	{
		return "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
	}

	else if (error == GL_INVALID_FRAMEBUFFER_OPERATION)
	{
		return "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
	}

	else if (error == GL_OUT_OF_MEMORY)
	{
		return "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
	}
	else
	{
		return "None";
	}
}

glm::mat4 Graphics::GetStarshipModelMatrix() const {
	return m_mesh->GetModel();
}

void Graphics::RenderCometTail(const glm::vec3& cometPos, const glm::vec3& sunPos)
{
	if (cometTrailPositions.size() < 2) return;

	glDisable(GL_DEPTH_TEST); 

	glLineWidth(6.0f);  


	for (size_t i = 0; i < cometTrailPositions.size() - 1; ++i)
	{
		const glm::vec3& start = cometTrailPositions[i];
		const glm::vec3& end = cometTrailPositions[i + 1];

		// Fade from yellow to transparent black
		float alpha = 1.0f - static_cast<float>(i) / cometTrailPositions.size();
		glm::vec3 color = glm::mix(glm::vec3(1.0f, 1.0f, 0.2f), glm::vec3(0.0f), 1.0f - alpha);

		GLfloat tailVertices[] = {
			start.x, start.y, start.z,
			end.x,   end.y,   end.z
		};

		GLuint tailVAO, tailVBO;
		glGenVertexArrays(1, &tailVAO);
		glGenBuffers(1, &tailVBO);

		glBindVertexArray(tailVAO);
		glBindBuffer(GL_ARRAY_BUFFER, tailVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tailVertices), tailVertices, GL_STATIC_DRAW);

		glEnableVertexAttribArray(m_positionAttrib);
		glVertexAttribPointer(m_positionAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

		glUniform1i(m_hasTexture, false);
		glUniform3f(m_overrideColor, color.r, color.g, color.b);
		// faded color

		glDrawArrays(GL_LINES, 0, 2);

		glDisableVertexAttribArray(m_positionAttrib);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glDeleteBuffers(1, &tailVBO);
		glDeleteVertexArrays(1, &tailVAO);
	}

	glUniform3f(m_overrideColor, 0.0f, 0.0f, 0.0f);
	// reset
	glEnable(GL_DEPTH_TEST);  // restore
}


GLuint Graphics::loadCubemap(std::vector<std::string> faces) {
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	for (GLuint i = 0; i < faces.size(); i++) {
		int width, height;
		unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		if (data) {
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				width,
				height,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				data
			);
			SOIL_free_image_data(data);
			std::cout << "Loaded cubemap face: " << faces[i] << std::endl;
		}
		else {
			std::cerr << " Failed to load cubemap texture at: " << faces[i] << std::endl;

			// Fallback: make an empty black texture instead of passing null
			unsigned char black[] = { 0, 0, 0 };
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				GL_RGB,
				1,
				1,
				0,
				GL_RGB,
				GL_UNSIGNED_BYTE,
				black
			);
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}
void Graphics::GenerateAsteroidBelts() {
	const int numInner = 800, numOuter = 800;
	float innerMin = 6.5f, innerMax = 7.0f;
	float outerMin = 16.0f, outerMax = 17.0f;

	auto generateBelt = [](int count, float minRadius, float maxRadius, std::vector<glm::mat4>& transforms) {
		for (int i = 0; i < count; ++i) {
			float angle = glm::radians((float)(rand() % 360));
			float radius = minRadius + static_cast <float>(rand()) / (static_cast <float>(RAND_MAX / (maxRadius - minRadius)));
			float height = ((rand() % 100) / 100.0f - 0.5f) * 0.5f; // small Y offset

			glm::vec3 position = glm::vec3(cos(angle) * radius, height, sin(angle) * radius);
			glm::mat4 model = glm::translate(glm::mat4(1.0f), position);
			model = glm::scale(model, glm::vec3(0.05f));

			transforms.push_back(model);
		}
		};

	generateBelt(numInner, innerMin, innerMax, innerAsteroidTransforms);
	generateBelt(numOuter, outerMin, outerMax, outerAsteroidTransforms);
	if (!innerAsteroidTransforms.empty()) {
		std::cout << "First inner asteroid matrix:\n";
		const glm::mat4& mat = innerAsteroidTransforms[0];
		for (int i = 0; i < 4; ++i) {
			std::cout << glm::to_string(mat[i]) << "\n";
		}
	}
}


void Graphics::SetupAsteroidInstancing() {
	// Generate buffers
	glGenBuffers(1, &innerAsteroidVBO);
	glBindBuffer(GL_ARRAY_BUFFER, innerAsteroidVBO);
	glBufferData(GL_ARRAY_BUFFER, innerAsteroidTransforms.size() * sizeof(glm::mat4), innerAsteroidTransforms.data(), GL_STATIC_DRAW);

	// Bind VAO for the asteroid mesh
	glBindVertexArray(m_asteroid->getVAO());

	// Bind the instance buffer before defining attributes
	glBindBuffer(GL_ARRAY_BUFFER, innerAsteroidVBO);

	// Set up mat4 as 4 vec4s
	for (int i = 0; i < 4; ++i) {
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(float) * i * 4));
		glVertexAttribDivisor(3 + i, 1);  // Advance per instance
	}

	// Unbind VAO to avoid accidental overwrites
	glBindVertexArray(0);
}


glm::vec3 Graphics::GetPlanetPosition(const std::string& name) {
	for (size_t i = 0; i < planets.size(); ++i) {
		if (planets[i].name == name) {
			return glm::vec3(planetSpheres[i]->GetModel()[3]);
		}
	}
	return glm::vec3(0.0f); // fallback
}

std::string Graphics::GetClosestPlanetName(const glm::vec3& position) {
	float minDist = std::numeric_limits<float>::max();
	std::string closestPlanet;

	for (size_t i = 0; i < planetSpheres.size(); ++i) {
		glm::vec3 planetPos = glm::vec3(planetSpheres[i]->GetModel()[3]);
		float dist = glm::distance(position, planetPos);

		if (dist < minDist) {
			minDist = dist;
			closestPlanet = planets[i].name;
		}
	}

	return closestPlanet;
}
