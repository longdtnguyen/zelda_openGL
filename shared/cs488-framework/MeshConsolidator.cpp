#include "MeshConsolidator.hpp"
using namespace glm;
using namespace std;
#include <iostream>
#include <cstring>
#include "cs488-framework/Exception.hpp"
#include "cs488-framework/ObjFileDecoder.hpp"


//----------------------------------------------------------------------------------------
// Default constructor
MeshConsolidator::MeshConsolidator()
{

}

//----------------------------------------------------------------------------------------
// Destructor
MeshConsolidator::~MeshConsolidator()
{

}

//----------------------------------------------------------------------------------------
template <typename T>
static void appendVector (
		std::vector<T> & dest,
		const std::vector<T> & source
) {
	// Increase capacity to hold source.size() more elements
	dest.reserve(dest.size() + source.size());

	dest.insert(dest.end(), source.begin(), source.end());
}


//----------------------------------------------------------------------------------------
MeshConsolidator::MeshConsolidator(
		std::initializer_list<ObjFilePath> objFileList
) {

	MeshId meshId;
	vector<vec3> positions;
	vector<vec3> normals;
	vector<vec2> uvs;
	BatchInfo batchInfo;
	unsigned long indexOffset(0);
	float width = 1024.0f;
	float height = 512.0f;

    for(const ObjFilePath & objFile : objFileList) {
    	float sx,sy,ex,ey;
    	string fpath = objFile.c_str();
	   	string name = fpath.substr(fpath.find_last_of("/\\") + 1);

	   	//maping to the texture atlas
    	if (name == "boatbody.obj") {
    		sx = 768.0f;
    		sy = 96.0f;
    		ex = 128.0f;
    		ey = 64.0f;
    	}else if(name == "boathead.obj") {
    		sx = 512.0f;
    		sy = 352.0f;
    		ex = 128.0f;
    		ey = 64.0f;
    	}else if(name == "boateye.obj") {
    		sx = 960.0f;
    		sy = 0.0f;
    		ex = 32.0f;
    		ey = 32.0f;
    	}else if(name == "linkbody.obj") {
    		sx = 768.0f;
    		sy = 0.0f;
    		ex = 160.0f;
    		ey = 96.0f;
    	}else if(name == "linkmouth.obj") {
    		sx = 512.0f;
    		sy = 256.0f;
    		ex = 192.0f;
    		ey = 96.0f;
    	}else if(name == "swordsheath.obj") {
    		sx = 928.0f;
    		sy = 0.0f;
    		ex = 32.0f;
    		ey = 128.0f;
    	}else if(name == "lefteye.obj") {
    		sx = 768.0f;
    		sy = 160.0f;
    		ex = 96.0f;
    		ey = 96.0f;
    	}else if(name == "righteye.obj") {
    		sx = 512.0f;
    		sy = 416.0f;
    		ex = 96.0f;
    		ey = 96.0f;
    	}else if(name =="boatsail.obj") {
            sx = 512.0f;
            sy = 0.0f;
            ex = 256.0f;
            ey = 256.0f;
        }else if(name == "water.obj") {
            sx = 0.0f;
            sy = 0.0f;
            ex = 512.0f;
            ey = 512.0f;
        }else {
            sx = 0.0f;
            sy = 0.0f;
            ex = width;
            ey = height;
        }


	    ObjFileDecoder::decode(objFile.c_str(), meshId, positions, normals,uvs);
	    uint numIndices = positions.size();
	    if (numIndices != normals.size()) {
		    throw Exception("Error within MeshConsolidator: "
					"positions.size() != normals.size()\n");
	    }
		//remaking the uv coor with texture atlas - before it was uv of each png
	   	for (int i=0;i< uvs.size();i++) {
	   		uvs[i].x = (sx + ex*uvs[i].x)/width;
	   		uvs[i].y = 1.0 - uvs[i].y;
	   		uvs[i].y = (sy + ey*uvs[i].y)/height;
	   	}

	    batchInfo.startIndex = indexOffset;
	    batchInfo.numIndices = numIndices;
        if (name == "boatsail.obj") {
            sail_start = indexOffset;
            sail_length = numIndices;
        }
	    m_batchInfoMap[meshId] = batchInfo;
        if(name == "sail2.obj") secondSail = positions;

	    appendVector(m_vertexPositionData, positions);
	    appendVector(m_vertexNormalData, normals);
	    appendVector(m_vertexUVData, uvs);
	    indexOffset += numIndices;
    }
    m_vertexPositionData2 = m_vertexPositionData;
    mixData(sail_start,sail_length);
}

//----------------------------------------------------------------------------------------
void MeshConsolidator::getBatchInfoMap (
		BatchInfoMap & batchInfoMap
) const {
	batchInfoMap = m_batchInfoMap;
}

void MeshConsolidator::mixData(uint start,uint length) {
    for (int i=start;i <start+ length;i++) {
        m_vertexPositionData2[i].x = secondSail[i-start].x;
        m_vertexPositionData2[i].y = secondSail[i-start].y;
        m_vertexPositionData2[i].z = secondSail[i-start].z;
    }
}
//----------------------------------------------------------------------------------------
// Returns the starting memory location for vertex position data.
const float * MeshConsolidator::getVertexPositionDataPtr() const {
	return &(m_vertexPositionData[0].x);
}

const float * MeshConsolidator::getVertexPositionDataPtr2() const {
    return &(m_vertexPositionData2[0].x);
}
//----------------------------------------------------------------------------------------
// Returns the starting memory location for vertex normal data.
const float * MeshConsolidator::getVertexNormalDataPtr() const {
    return &(m_vertexNormalData[0].x);
}

//----------------------------------------------------------------------------------------
// Returns the starting memory location for vertex UV data.
const float * MeshConsolidator::getVertexUVDataPtr() const {
    return &(m_vertexUVData[0].x);
}

//----------------------------------------------------------------------------------------
// Returns the total number of bytes of all vertex position data.
size_t MeshConsolidator::getNumVertexPositionBytes() const {
	return m_vertexPositionData.size() * sizeof(vec3);
}
//----------------------------------------------------------------------------------------
// Returns the total number of bytes of all vertex position data.
size_t MeshConsolidator::getNumVertexPositionBytes2() const {
    return m_vertexPositionData2.size() * sizeof(vec3);
}

//----------------------------------------------------------------------------------------
// Returns the total number of bytes of all vertex normal data.
size_t MeshConsolidator::getNumVertexNormalBytes() const {
	return m_vertexNormalData.size() * sizeof(vec3);
}

//----------------------------------------------------------------------------------------
// Returns the total number of bytes of all vertex UV data.
size_t MeshConsolidator::getNumVertexUVBytes() const {
	return m_vertexUVData.size() * sizeof(vec2);
}
