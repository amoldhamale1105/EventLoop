# C++ Base Project Template
Thinking of starting a brand new C++ project from scratch? Look no further than this template!  
This is a sample C++ project setup that can be used as a template to start a new project in Linux enviroment  
This project template uses the **cmake** build system to build and generate artifacts  
This template can be used to genrate executable binaries or shared libraries to be linked with other applications    

# Project Setup with template
Replace the sample files (`App.hpp` and `App.cpp`) with your own header and source files  
Make sure you have all your application headers inside `include/` directory and source files in `src/` directory  
Finally set up your own `main()` function by including necessary headers in `main.cpp`  
Add your headers in `CMakeLists.txt` under the `INCLUDES` variable  
Add your sources in `CMakeLists.txt` under the `SOURCES` variable   

If you're new to C++ or cmake build system, carefully observe placement of sample files in the directory structure of template project and `CMakeLists.txt` with reference to above instructions

# Build instructions
If you're intending to create a shared library, uncomment the `add_library()` call and comment out the `add_executable()` call in `CMakeLists.txt`  
Follow the ensuing instrcutions depending on your dev environment  

## VS Code
Open the project and click on the **CMake** option in the taskbar  
Configure the project using `Clean Reconfigure All`. Subsequently, build it using `Clean Rebuild All`  

## Terminal/shell
Open terminal window/shell and navigate to project root on the terminal  
Run the following comands once at the root of the project  
```
mkdir -p build
cd build
cmake ..
make
```
Build artifacts will be generated in the `build` directory  
Output Artifacts will be in either `bin` or `lib` directory depending on cmake configuration  
In case of shared library generation, it will be present as `lib<ProjectName>.so` file in the `lib` directory which can be further linked or moved to your target application  

## Output and Testing
If you intended to generate an executable, run the binary/executable from the `bin` directory to launch.  
The sample program in the template can take in an optional string name argument as follows:
```
./bin/BaseProject Amol
```
Alternatively you can run the program without any argument and enter your name when prompted  
The console will then print a random greeting for entered username and exit  
