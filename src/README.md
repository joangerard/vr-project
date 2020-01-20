# VR Project with OpenGL
This repository is based on a fork from https://learnopengl.com. So it contains the same structure.
This project was based on https://learnopengl.com and http://www.opengl-tutorial.org 
examples and tutorials so it contains some portions of code from them.

# Building
First copy the resources from [this url](https://drive.google.com/drive/folders/18LIBy6WQD28gwQvQ5YUa9vdAtrDreN0S?usp=sharing) to the resources files. They were too heavy to be 
upload to git. That is why they are in another website.

```
brew install cmake assimp glm glfw
mkdir build
cd build
cmake ../.
make -j8
```

## Project Details

It is a sample project where it exemplifies how to load models, use the camera, use of lights, 
shadows and dynamic environment mapping.

![Example](https://github.com/joangerard/vr-project/blob/master/src/ex.png)

