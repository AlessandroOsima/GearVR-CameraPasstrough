//
// Created by alex_ on 10/10/2017.
//

#ifndef TESTCAMERARENDER_RESOURCES_H
#define TESTCAMERARENDER_RESOURCES_H

#include <string>

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    static const std::string & GetShadersForlder()
    {
        return Shaders;
    }


private:
    static const std::string Shaders;
};

#endif //TESTCAMERARENDER_RESOURCES_H
