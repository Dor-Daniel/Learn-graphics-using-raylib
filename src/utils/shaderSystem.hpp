#pragma once 
#include "raylib.h"
#include <map>
#include <string>
namespace ShaderSystem{

    std::map<std::string, Shader> shaders;

    void load(std::string shader_name, std::string vert_path, std::string frag_path){
        if(shaders.find(shader_name) == shaders.end()) return;
        Shader s = LoadShader(TextFormat(vert_path.c_str()), TextFormat(frag_path.c_str()));
        shaders.insert({shader_name, s});   
    }

    void cleanup(){
        for (const auto& pair : shaders){
            UnloadShader(pair.second);
        }
        shaders.clear(); 
    }


}