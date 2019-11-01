// #include "Prereq.h"
#include "ShaderPrecacher.h"
#include <iostream>

int main(int argc, char **argv)
{
    CShaderPrecacher *pShaderPrecacher = new CShaderPrecacher();
    if (pShaderPrecacher->Precache())
    {
        std::cout << "Shader precaching end" << std::endl;
        return 0;
    }
    else
    {
        std::cout << "Shader precaching error" << std::endl;
        return -1;
    }
}