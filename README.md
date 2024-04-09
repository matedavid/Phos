# Phos 

Simple Game Engine built using ImGui and Vulkan.

![Editor](https://github.com/matedavid/Phos/assets/42835499/a7d4dd1a-4f3c-4af7-ad4f-94844a4371a3)

### Features

#### Rendering
- Deferred Rendering
- Bloom
- Directional shadow mapping, with multiple directional lights
- PBR Materials
- Skybox

#### Engine
- C# scripting
- C# scripting / C++ engine interaction with C# PhosEngine library
- Asset management
- ECS system, with entity hirearchy in editor

### Limitations

Apart from various bugs that can appear, the project currently only works on Linux, specially because mono C# libraries have only been tested for linux builds.

### Requirements
- [glslc](https://github.com/google/shaderc) compiler installed on the system on the system

### How to build

You can build the project using cmake and make/ninja. Just create a `build/` directory and run the following commands:

```
cmake ..
make
```

This will create, by default, the editor executable under  `build/apps/editor`, and the tests under `build/tests`.

### How to run the Sample project

The project comes with a sample project you can run out of the box. This project is under `projects/Sample/`. To execute the project, you first need to compile the C# code.

You can do this by navigating to `projects/Sample/` and running:

```
msbuild Sample.csproj
```

Or compiling the C# code with your preferred editor, be it Visual Studio or JetBrains Rider, among others.

Then, you just need to navigate to `build/apps/editor` and run the PhosEditor executable from that location. This will open up the editor with the Sample project loaded.

```
./PhosEditor
```
