#pragma once

#include "cs488-framework/CS488Window.hpp"
#include "cs488-framework/OpenGLImport.hpp"
#include "cs488-framework/ShaderProgram.hpp"
#include "cs488-framework/MeshConsolidator.hpp"

#include "SceneNode.hpp"

#include <glm/glm.hpp>
#include <cstring>
#include <memory>
#include <stack>
#include <cstdio>
#include <vector>
#include <cstdlib>     /* srand, rand */
#include <ctime>       /* time */
#include <algorithm>
struct LightSource {
	glm::vec3 position;
	glm::mat4 projection;
	glm::mat4 lightView;
};

struct Image {
	std::vector<unsigned char> data;
  	unsigned width, height;
};

class Zelda : public CS488Window {
public:
	Zelda(const std::string & luaSceneFile);
	virtual ~Zelda();

protected:
	virtual void init() override;
	virtual void appLogic() override;
	virtual void guiLogic() override;
	virtual void draw() override;
	virtual void cleanup() override;

	//-- Virtual callback methods
	virtual bool cursorEnterWindowEvent(int entered) override;
	virtual bool mouseMoveEvent(double xPos, double yPos) override;
	virtual bool mouseButtonInputEvent(int button, int actions, int mods) override;
	virtual bool mouseScrollEvent(double xOffSet, double yOffSet) override;
	virtual bool windowResizeEvent(int width, int height) override;
	virtual bool keyInputEvent(int key, int action, int mods) override;



	//-- One time initialization methods:
	void processLuaSceneFile(const std::string & filename);
	void createShaderProgram();
	void enableVertexShaderInputSlots();
	void uploadVertexDataToVbos(const MeshConsolidator & meshConsolidator);
	void bindBuffer();
	void initViewMatrix();
	void initLightSources();
	void initPerspectiveMatrix();
	void uploadCommonSceneUniforms();
	void createTexture();
	void shadowFrame();
	void postFrame();
	void flareFrame();
	Image getImage(const std::string& filename);



	//----- MOVEMENT
	void stallBoat();
	int findUnused(bool dir);
	void sortParticles();
	
	//------ Draw
//-all objects
	void drawObjects(const SceneNode &node,int flag);
	void drawParticles();
	void drawScreen();
	void drawSun(SceneNode * node);
	void traverseTree(SceneNode * node,int flag);
//-partiles
	void makeParticles(int num);
	int updateParticles();
	

	//----VAR
//-matrices
	glm::mat4 perspective;
	glm::mat4 view;
	glm::mat4 translate;
	glm::mat4 rotate;
	glm::mat4 b_auto;	//boat
	glm::mat4 b_rotate;
//-vao and vbo
	GLuint vao_meshData;
	GLuint vao_particleData;
	GLuint vao_screenData;

	GLuint vbo_vertexPositions;
	GLuint vbo_vertexPositions2;
	
	GLuint vbo_vertexNormals;
	GLuint vbo_vertexUVs;


	GLuint vbo_particles;
	GLuint vbo_particles_position;
	GLuint vbo_particles_uv;

	GLuint vbo_screenPosition;
	GLuint vbo_screenUV;

//-lua processing data
	GLint UVAttribLocation;
	GLint positionAttribLocation;
	GLint positionAttribLocation2;
	GLint normalAttribLocation;



	std::string luaSceneFile;
	std::shared_ptr<SceneNode> rootNode;
	BatchInfoMap batchInfoMap;

//-particles
	GLint particleVertexLocation;
	GLint particlePositionLocation;
	GLint particleUVLocation;

//-screen
	GLint screenPositionLocation;
	GLint screenUVLocation;

//-shader
	ShaderProgram main_shader, particle_shader, 
				  shadow_shader, screen_shader,lens_shader;

//-Misc
	LightSource light;
	float fov;
	double xOld,yOld; 					//previous location of the mouse
	int mouse_hold,key_hold;
	std::stack<glm::mat4> mat_stack;	//transformation matrix stack

};
