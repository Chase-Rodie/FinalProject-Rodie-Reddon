#include "shader.h"

Shader::Shader()
{
    m_shaderProg = 0;
}

Shader::~Shader()
{
    for (auto shader : m_shaderObjList)
        glDeleteShader(shader);

    if (m_shaderProg != 0)
        glDeleteProgram(m_shaderProg);
}

bool Shader::Initialize()
{
    m_shaderProg = glCreateProgram();
    if (m_shaderProg == 0)
    {
        std::cerr << "Error creating shader program\n";
        return false;
    }
    return true;
}

bool Shader::AddShader(GLenum ShaderType)
{
    std::string s;

    if (ShaderType == GL_VERTEX_SHADER)
    {
        s = R"(
            #version 460
            layout (location = 0) in vec3 v_position;
            layout (location = 1) in vec3 v_normal;
            layout (location = 2) in vec2 v_tc;

            out vec3 fragPos;
            out vec3 normal;
            out vec2 tc;

            uniform mat4 projectionMatrix;
            uniform mat4 viewMatrix;
            uniform mat4 modelMatrix;

            void main()
            {
                fragPos = vec3(modelMatrix * vec4(v_position, 1.0));
                normal = mat3(transpose(inverse(modelMatrix))) * v_normal;
                tc = v_tc;
                gl_Position = projectionMatrix * viewMatrix * vec4(fragPos, 1.0);
            }
        )";
    }
    else if (ShaderType == GL_FRAGMENT_SHADER)
    {
        s = R"(
#version 460

in vec3 fragPos;
in vec3 normal;
in vec2 tc;

uniform sampler2D sp;
uniform bool hasTexture;
uniform vec3 overrideColor;

uniform vec3 lightColor;
uniform vec3 lightDir;

uniform vec3 nightColor;
uniform vec3 ambientColor;

uniform bool isEmissive;

out vec4 frag_color;

void main()
{
    if (isEmissive) {
        vec3 baseColor = hasTexture ? texture(sp, tc).rgb : vec3(1.0);
        frag_color = vec4(baseColor * 5.0, 1.0); // Glowing Sun
        return;
    }

    vec3 norm = normalize(normal);

    // Light facing factor
    float NdotL = max(dot(norm, -lightDir), 0.0);

    // Estimate distance-based attenuation
    float distance = length(fragPos); // distance from origin (Sun)
    float attenuation = 1.0 / (distance * distance); // inverse square falloff

    // Clamp to avoid too bright or too dim results
    attenuation = clamp(attenuation * 20.0, 0.0, 1.0); // scale factor is tweakable

    vec3 blendedLight = mix(nightColor, lightColor, NdotL);
    vec3 lighting = ambientColor + blendedLight * attenuation;


    // Texture or fallback color
    vec3 baseColor = hasTexture ? texture(sp, tc).rgb : vec3(1.0);
    vec3 finalColor = (overrideColor != vec3(0.0)) ? overrideColor : baseColor;

    frag_color = vec4(finalColor * lighting, 1.0);
}
)";
    }

    GLuint ShaderObj = glCreateShader(ShaderType);
    if (ShaderObj == 0)
    {
        std::cerr << "Error creating shader type " << ShaderType << std::endl;
        return false;
    }

    m_shaderObjList.push_back(ShaderObj);

    const GLchar* p[1] = { s.c_str() };
    GLint Lengths[1] = { (GLint)s.size() };

    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        std::cerr << "Error compiling: " << InfoLog << std::endl;
        return false;
    }

    glAttachShader(m_shaderProg, ShaderObj);
    return true;
}

bool Shader::Finalize()
{
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(m_shaderProg);
    glGetProgramiv(m_shaderProg, GL_LINK_STATUS, &Success);
    if (Success == 0)
    {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
        return false;
    }

    glValidateProgram(m_shaderProg);
    glGetProgramiv(m_shaderProg, GL_VALIDATE_STATUS, &Success);
    if (!Success)
    {
        glGetProgramInfoLog(m_shaderProg, sizeof(ErrorLog), NULL, ErrorLog);
        std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
        return false;
    }

    for (auto shader : m_shaderObjList)
        glDeleteShader(shader);
    m_shaderObjList.clear();

    return true;
}

void Shader::Enable()
{
    glUseProgram(m_shaderProg);
}

GLint Shader::GetUniformLocation(const char* pUniformName)
{
    GLuint Location = glGetUniformLocation(m_shaderProg, pUniformName);
    if (Location == INVALID_UNIFORM_LOCATION) {
        fprintf(stderr, "Warning! Unable to get the location of uniform '%s'\n", pUniformName);
    }
    return Location;
}

GLint Shader::GetAttribLocation(const char* pAttribName)
{
    GLuint Location = glGetAttribLocation(m_shaderProg, pAttribName);
    if (Location == -1) {
        fprintf(stderr, "Warning! Unable to get the location of attribute '%s'\n", pAttribName);
    }
    return Location;
}

bool Shader::AddShader(GLenum ShaderType, const char* shaderSource)
{
    GLuint ShaderObj = glCreateShader(ShaderType);
    if (ShaderObj == 0)
    {
        std::cerr << "Error creating shader type " << ShaderType << std::endl;
        return false;
    }

    m_shaderObjList.push_back(ShaderObj);

    glShaderSource(ShaderObj, 1, &shaderSource, NULL);
    glCompileShader(ShaderObj);

    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        std::cerr << "Error compiling shader: " << InfoLog << std::endl;
        return false;
    }

    glAttachShader(m_shaderProg, ShaderObj);
    return true;
}