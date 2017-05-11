/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Icosphere.h
 * Author: strelok
 *
 * Created on 10 мая 2017 г., 23:12
 */

#include <vector>
#include <map>
#include "math/Vec3.h"

#ifndef ICOSPHERE_H
#define ICOSPHERE_H

struct Triangle
{
  uint32_t vertex[3];
};


using TriangleList=std::vector<Triangle>;
using VertexList=std::vector<cocos2d::Vec3>;
using IndexedMesh=std::pair<VertexList, TriangleList>;
using Lookup=std::map<std::pair<uint32_t, uint32_t>, uint32_t>;
class Icosphere {
public:
    static IndexedMesh make_icosphere(int subdivisions);
private:

};

#endif /* ICOSPHERE_H */

