# Solar System Exploration Game

## Overview
This project implements a third-person space exploration game that allows users to navigate through the solar system and observe celestial bodies from orbit. The game features realistic planetary orbits, dynamic lighting, textured models, and two distinct camera/gameplay modes: **Exploration Mode** and **Planetary Observation Mode**.

---

## Controls & Interactions

### Exploration Mode (Third Person)
- `W / S`: Accelerate / Decelerate
- `A / D`: Yaw left / right
- `Arrow Keys` or `Mouse Movement`: Pitch/steer
- `TAB`: Toggle to Planetary Observation Mode

### Planetary Observation Mode (First Person)
- `Mouse`: Look around the planet
- `Scroll`: Zoom in/out
- `Arrow Keys`: Pan camera around the target planet
- `TAB`: Toggle back to Exploration Mode

---

## Dependencies

This project uses the following **required** libraries, most of which were introduced in the programming assignments:

### ✅ Libraries Used in Class Assignments:
- **GLFW**  
  For window management and input handling.  
  URL: https://www.glfw.org/

- **GLM**  
  OpenGL Mathematics Library (vec3, mat4, transforms).  
  URL: https://github.com/g-truc/glm



## Setup Instructions

### Prerequisites
- C++17 compiler
- CMake 3.14+
- OpenGL 3.3+ supported GPU
- Git 

### Folder Structure
project/
├── assets/ # Textures, models, skyboxes
├── shaders/ # Vertex and fragment shaders
├── src/ # Source code (.cpp/.h files)
├── external/ # External libraries (GLFW, GLAD, etc.)
├── CMakeLists.txt
├── README.md
└── ...

### Compilation Steps 
1. Clone or download the project.
2. Ensure the following directories are inside `external/`: `glfw`, `glad`, `glm`, and `stb_image.h`
3. Create a build directory:
    ```bash
    mkdir build && cd build
    cmake ..
    make
    ./SolarSystemGame
    ```

### Texture & Model Notes
- Ensure the `assets/` folder contains all textures and models.
- Diffuse, specular, and normal maps must be in the same folder as referenced in code.


## Known Issues
- Camera can clip through objects when too close.
- Performance may degrade with very high asteroid count unless instancing is properly implemented.

## Team Members
| Name         | Contributions                         |
|--------------|----------------------------------------|
| Chase Rodie  | Meshes, pipeline, shaders, gameplay    |
| Lexie Reddon | Texturing, lighting, interactions      |

---

## License
This project is for educational purposes only.
