//
// Created by alex_ on 10/20/2017.
//

#ifndef TESTCAMERARENDER_RENDERABLE_H
#define TESTCAMERARENDER_RENDERABLE_H

#include <VrApi.h>
#include <memory>
#include "VertexData.h"

struct Renderable
{
    ovrMatrix4f ModelMatrix;
    std::shared_ptr<VertexData> Vertices;
};


#endif //TESTCAMERARENDER_RENDERABLE_H
