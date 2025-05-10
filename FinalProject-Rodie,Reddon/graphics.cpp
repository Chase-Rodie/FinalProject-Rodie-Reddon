#include "graphics.h"
std::vector<CelestialBody> planets;
std::vector<Sphere*> planetSpheres;
std::vector<Moon> moons;



Graphics::Graphics()
{

}

Graphics::~Graphics()
{

}

bool Graphics::Initialize(int width, int height)
{
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

	// Starship
	m_mesh = new Mesh(glm::vec3(0.0f), "assets\\SpaceShip-1.obj", "assets\\SpaceShip-1.png");
	glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -20.0f)) *
		glm::scale(glm::vec3(0.025f));
	m_mesh->Update(model);


	// The Sun
	m_sphere = new Sphere(64, "assets\\2k_sun.jpg");

	// The Earth
	//m_sphere2 = new Sphere(48, "assets\\2k_earth_daymap.jpg");

	// The moon
	//m_sphere3 = new Sphere(48, "assets\\2k_moon.jpg");

	planets = {
		{ "Mercury", 2.0f, 4.74f, 10.83f, 0.2f, 0.01f, "assets/Mercury.jpg" },
		{ "Venus",   3.0f, 3.5f, -6.52f, 0.45f, 177.4f, "assets/Venus.jpg" },
		{ "Earth",   4.0f, 2.98f, 15.0f, 0.5f, 23.5f, "assets/2k_earth_daymap.jpg" },
		{ "Mars",    5.0f, 2.41f, 14.6f, 0.35f, 25.0f, "assets/Mars.jpg" },
		{ "Jupiter", 7.0f, 1.31f, 25.0f, 1.0f, 3.1f, "assets/Jupiter.jpg" },
		{ "Saturn",  9.0f, 0.97f, 22.0f, 0.9f, 26.7f, "assets/Saturn.jpg" },
		{ "Uranus",  11.0f, 0.68f, -17.2f, 0.7f, 97.8f, "assets/Uranus.jpg" },
		{ "Neptune", 13.0f, 0.54f, 16.1f, 0.65f, 28.3f, "assets/Neptune.jpg" }
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


	//enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	return true;
}

void Graphics::HierarchicalUpdate2(double dt) {
	modelStack = std::stack<glm::mat4>(); // clear stack
	glm::mat4 identity = glm::mat4(1.0f);
	modelStack.push(identity);

	// 1. Sun: rotate in place at origin
	glm::mat4 sunRotation = glm::rotate(identity, (float)(dt * 0.2f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 sunScale = glm::scale(glm::vec3(1.5f)); // slightly bigger sun
	glm::mat4 sunModel = modelStack.top() * sunRotation * sunScale;
	m_sphere->Update(sunModel);

	for (size_t i = 0; i < planets.size(); ++i) {
		const CelestialBody& p = planets[i];

		glm::mat4 orbit = glm::rotate(glm::mat4(1.0f), (float)(dt * p.orbitSpeed), glm::vec3(0, 1, 0));
		glm::mat4 translate = glm::translate(glm::vec3(p.orbitRadius, 0.0f, 0.0f));
		glm::mat4 tilt = glm::rotate(glm::mat4(1.0f), glm::radians(p.axialTilt), glm::vec3(0, 0, 1));
		glm::mat4 spin = glm::rotate(glm::mat4(1.0f), (float)(dt * p.rotationSpeed), glm::vec3(0, 1, 0));
		glm::mat4 scale = glm::scale(glm::vec3(p.scale));

		glm::mat4 model = modelStack.top() * orbit * translate * tilt * spin * scale;
		planetSpheres[i]->Update(model);
	}

	for (Moon& m : moons) {
		glm::mat4 planetModel = planetSpheres[m.parentPlanetIndex]->GetModel();
		glm::mat4 moonTilt = glm::rotate(glm::mat4(1.0f), glm::radians(m.tilt), glm::vec3(1.0f, 0.0f, 0.0f));
		glm::mat4 moonOrbit = glm::rotate(glm::mat4(1.0f), (float)(dt * m.orbitSpeed), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 moonTranslate = glm::translate(glm::vec3(m.orbitRadius, 0.0f, 0.0f));
		glm::mat4 moonScale = glm::scale(glm::vec3(m.scale));
		glm::mat4 moonModel = planetModel * moonTilt * moonOrbit * moonTranslate * moonScale;
		m.sphere->Update(moonModel);
	}


	//// 2. Planet: orbit sun in x-z plane
	//modelStack.push(modelStack.top());
	//glm::mat4 planetOrbit = glm::rotate(identity, (float)(dt), glm::vec3(0.0f, 1.0f, 0.0f)); // orbiting
	//glm::mat4 planetTranslate = glm::translate(glm::vec3(5.0f, 0.0f, 0.0f)); // orbit radius
	//glm::mat4 planetRotate = glm::rotate(identity, (float)(dt * 2.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // self-rotation
	//glm::mat4 planetScale = glm::scale(glm::vec3(0.6f));
	//glm::mat4 planetModel = modelStack.top() * planetOrbit * planetTranslate * planetRotate * planetScale;
	//m_sphere2->Update(planetModel);

	// 3. Moon: orbit planet at a tilted angle
	//modelStack.push(planetSpheres[2]->GetModel()); 
	//glm::mat4 moonTilt = glm::rotate(identity, glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // tilt
	//glm::mat4 moonOrbit = glm::rotate(identity, (float)(dt * 3.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // orbiting
	//glm::mat4 moonTranslate = glm::translate(glm::vec3(1.5f, 0.0f, 0.0f));
	//glm::mat4 moonScale = glm::scale(glm::vec3(0.2f));
	//glm::mat4 moonModel = modelStack.top() * moonTilt * moonOrbit * moonTranslate * moonScale;
	//m_sphere3->Update(moonModel);
	//modelStack.pop(); // pop moon

	modelStack.pop(); // pop planet

	// 4. Starship: orbit in Y-Z plane and point toward sun
	float starshipAngle = static_cast<float>(dt * 0.8f);
	float orbitRadius = 14.0f; 

	// Position in Y-Z plane (X = 0)
	glm::vec3 shipPos = glm::vec3(0.0f, orbitRadius * sin(starshipAngle), orbitRadius * cos(starshipAngle));
	glm::vec3 sunPos = glm::vec3(0.0f);

	// Forward direction from ship to sun
	glm::vec3 forward = glm::normalize(sunPos - shipPos);

	// Fixed right direction in X, since we're orbiting Y-Z plane
	glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	// Recompute right to ensure full orthonormal basis
	right = glm::normalize(glm::cross(up, forward));

	// Build orientation matrix (forward = Z axis)
	glm::mat4 orientation = glm::mat4(1.0f);
	orientation[0] = glm::vec4(right, 0.0f);
	orientation[1] = glm::vec4(up, 0.0f);
	orientation[2] = glm::vec4(forward, 0.0f);  
	orientation[3] = glm::vec4(0, 0, 0, 1);

	// Final model matrix
	glm::mat4 model = glm::translate(glm::mat4(1.0f), shipPos) * orientation * glm::scale(glm::vec3(0.3f));


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
	//clear the screen
	glClearColor(0.5, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Start the correct program
	m_shader->Enable();

	// Send in the projection and view to the shader (stay the same while camera intrinsic(perspective) and extrinsic (view) parameters are the same
	glUniformMatrix4fv(m_projectionMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetProjection()));
	glUniformMatrix4fv(m_viewMatrix, 1, GL_FALSE, glm::value_ptr(m_camera->GetView()));

	// Render the objects
	/*if (m_cube != NULL){
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_cube->GetModel()));
		m_cube->Render(m_positionAttrib,m_colorAttrib);
	}*/

	if (m_mesh != NULL) {
		glUniform1i(m_hasTexture, false);
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_mesh->GetModel()));

		if (m_mesh->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_mesh->getTextureID());  // FIXED: use m_mesh not m_sphere

			GLuint sampler = m_shader->GetUniformLocation("sp");
			if (sampler == INVALID_UNIFORM_LOCATION) {
				printf("Sampler not found\n");
			}
			glUniform1i(sampler, 0);
		}

		m_mesh->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	}


	/*if (m_pyramid != NULL) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_pyramid->GetModel()));
		m_pyramid->Render(m_positionAttrib, m_colorAttrib);
	}*/

	if (m_sphere != NULL) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_sphere->GetModel()));
		if (m_sphere->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m_sphere->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			if (sampler == INVALID_UNIFORM_LOCATION)
			{
				printf("Sampler Not found not found\n");
			}
			glUniform1i(sampler, 0);
			m_sphere->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
		}
	}

	//if (m_sphere2 != NULL) {
	//	glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_sphere2->GetModel()));
	//	if (m_sphere2->hasTex) {
	//		glActiveTexture(GL_TEXTURE0);
	//		glBindTexture(GL_TEXTURE_2D, m_sphere2->getTextureID());
	//		GLuint sampler = m_shader->GetUniformLocation("sp");
	//		if (sampler == INVALID_UNIFORM_LOCATION)
	//		{
	//			printf("Sampler Not found not found\n");
	//		}
	//		glUniform1i(sampler, 0);
	//		m_sphere2->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	//	}
	//}


	// Render Moon
	//if (m_sphere3 != NULL) {
	//	glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m_sphere3->GetModel()));
	//	if (m_sphere3->hasTex) {
	//		glActiveTexture(GL_TEXTURE0);
	//		glBindTexture(GL_TEXTURE_2D, m_sphere3->getTextureID());
	//		GLuint sampler = m_shader->GetUniformLocation("sp");
	//		if (sampler == INVALID_UNIFORM_LOCATION)
	//		{
	//			printf("Sampler Not found not found\n");
	//		}
	//		glUniform1i(sampler, 0);
	//		m_sphere3->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	//	}
	//}

	for (Sphere* planet : planetSpheres) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(planet->GetModel()));
		if (planet->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, planet->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			if (sampler == INVALID_UNIFORM_LOCATION) {
				printf("Sampler Not found\n");
			}
			glUniform1i(sampler, 0);
		}
		planet->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	}

	for (Moon& m : moons) {
		glUniformMatrix4fv(m_modelMatrix, 1, GL_FALSE, glm::value_ptr(m.sphere->GetModel()));
		if (m.sphere->hasTex) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, m.sphere->getTextureID());
			GLuint sampler = m_shader->GetUniformLocation("sp");
			glUniform1i(sampler, 0);
		}
		m.sphere->Render(m_positionAttrib, m_colorAttrib, m_tcAttrib, m_hasTexture);
	}



	// Get any errors from OpenGL
	auto error = glGetError();
	if (error != GL_NO_ERROR)
	{
		string val = ErrorString(error);
		std::cout << "Error initializing OpenGL! " << error << ", " << val << std::endl;
	}
}


bool Graphics::collectShPrLocs() {
	bool anyProblem = true;
	// Locate the projection matrix in the shader
	m_projectionMatrix = m_shader->GetUniformLocation("projectionMatrix");
	if (m_projectionMatrix == INVALID_UNIFORM_LOCATION)
	{
		printf("m_projectionMatrix not found\n");
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

	// Locate the position 
	// 
	// 
	// attribute
	m_positionAttrib = m_shader->GetAttribLocation("v_position");
	if (m_positionAttrib == -1)
	{
		printf("v_position attribute not found\n");
		anyProblem = false;
	}

	// Locate the color vertex attribute
	m_colorAttrib = m_shader->GetAttribLocation("v_color");
	if (m_colorAttrib == -1)
	{
		printf("v_color attribute not found\n");
		anyProblem = false;
	}

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

