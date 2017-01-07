#include "Zelda.hpp"
#include "scene_lua.hpp"
using namespace std;

#include "cs488-framework/GlErrorCheck.hpp"
#include "cs488-framework/MathUtils.hpp"
#include "GeometryNode.hpp"
#include "lodepng.hpp"
#include "perlin.hpp"
#include "Particle.hpp"
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/gl.h>
#include <GL/glut.h>
#include <irrklang/irrKlang.h>

using namespace glm;

//-- particles
static const int NUM_PARTICLES = 900;

int last_used = 0;

static int active_particles = 0;
static Particle* Pcontainer;
static GLfloat* particle_position_size;


static float perlin_noise_level = 0.0;
static bool show_gui = true;
static int boat_stall = 0;
static int frame_count = 0;

static int frame_stall =999;


int RNGseed = 0;	//wont change for the whole 60 frames


//---------------MODE----------------------------
static int login_mode = true;
static int shadow_mode = 1;

static bool toggle_shadow = true;
static bool toggle_reflect = false;
static bool toggle_flare = true;
static bool toggle_particle = false;
static bool show_shadow_map = false;
static bool show_normal = true;

static bool show_cel = true;
static bool show_phong = false;


static int w_rotate= 0;
static int w_translate= 0;
GLuint textureID,depthID,postID,flareID,lenColorID,loginID,particleID;
GLuint post_fbo,depth_fbo,post_rbo;

const GLuint SHADOW_WIDTH = 1920, SHADOW_HEIGHT = 1080;
irrklang::ISoundEngine * bSound;


//----------------------------------------------------------------------------//
//																			  //
//										INIT 								  //
//																			  //
//----------------------------------------------------------------------------//

//---------------------------------------------------
// Constructor
Zelda::Zelda(const std::string & luaSceneFile)
	: luaSceneFile(luaSceneFile),
	  positionAttribLocation(0),
	  normalAttribLocation(0),
	  UVAttribLocation(0),
	  vao_meshData(0),
	  vao_particleData(0),
	  vbo_vertexPositions(0),
	  vbo_vertexNormals(0),
	  vbo_vertexUVs(0)
{

}

//---------------------------------------------------
// Destructor
Zelda::~Zelda()
{

}

//---------------------------------------------------
/*
 * Called once, at program start.
 */
void Zelda::init()
{
	// Set the background colour blue sky
	cout << "------ INITIATION START -------" << endl << endl;
	//--- var
	mouse_hold = -1;
	key_hold = -1;
	fov =100.0f;
	srand (time(NULL)); //SEED

	particle_position_size = new GLfloat[NUM_PARTICLES * 4];
	Pcontainer = new Particle[NUM_PARTICLES];


 	createShaderProgram();

	glGenVertexArrays(1, &vao_meshData);
	glGenVertexArrays(1, &vao_particleData);
	glGenVertexArrays(1, &vao_screenData);


	enableVertexShaderInputSlots();

	processLuaSceneFile(luaSceneFile);

	// Load and decode all .obj files at once here.  You may add additional .obj files to
	// this list in order to support rendering additional mesh types.  All vertex
	// positions, and normals will be extracted and stored within the MeshConsolidator
	// class.
	unique_ptr<MeshConsolidator> meshConsolidator (new MeshConsolidator{
			//MA SUN AND MA WATER
			getAssetFilePath("water.obj"),
			getAssetFilePath("sphere.obj"),
			getAssetFilePath("pad.obj"),

			//MA BOAT
			getAssetFilePath("Boat/boathead.obj"),
			getAssetFilePath("Boat/boatbody.obj"),
			getAssetFilePath("Boat/boateye.obj"),
			getAssetFilePath("Boat/boatsail.obj"),
			getAssetFilePath("Boat/sail2.obj"),
			//LINK
			getAssetFilePath("Link/linkbody.obj"),
			getAssetFilePath("Link/linkmouth.obj"),
			getAssetFilePath("Link/lefteye.obj"),
			getAssetFilePath("Link/righteye.obj"),
			getAssetFilePath("Link/swordsheath.obj"),
			

			
	});


	// Acquire the BatchInfoMap from the MeshConsolidator.
	meshConsolidator->getBatchInfoMap(batchInfoMap);

	// Take all vertex data within the MeshConsolidator and upload it to VBOs on the GPU.
	uploadVertexDataToVbos(*meshConsolidator);
	bindBuffer();
	
	initPerspectiveMatrix();
	initViewMatrix();
	initLightSources();

	postFrame();
	shadowFrame();
	createTexture();

	bSound = irrklang::createIrrKlangDevice();
	bSound->play2D("Assets/login.mp3",GL_TRUE); 

	makeParticles(NUM_PARTICLES);
	cout << "postID: " << postID << " ,textureID: " << textureID <<
			" ,depthID: " << depthID << " ,flareID: " << flareID << endl;
	cout << "post_fbo: " << post_fbo << " ,depth_fbo: " << depth_fbo <<
			" ,post_rbo: " << post_rbo <<endl << endl; 
	cout << "------ INITIATION COMPLETE -------" << endl;

}

//---------------------------------------------------
void Zelda::processLuaSceneFile(const std::string & filename) {
	// This version of the code treats the Lua file as an Asset,
	// so that you'd launch the program with just the filename
	// of a puppet in the Assets/ directory.
	std::string assetFilePath = getAssetFilePath(filename.c_str());
	rootNode = std::shared_ptr<SceneNode>(import_lua(assetFilePath));

	// This version of the code treats the main program argument
	// as a straightforward pathname.
	//rootNode = std::shared_ptr<SceneNode>(import_lua(filename));
	if (!rootNode) {
		std::cerr << "Could not open " << filename << std::endl;
	}
}



  	


//---------------------------------------------------
void Zelda::createShaderProgram()
{
	main_shader.generateProgramObject();
	main_shader.attachVertexShader( getAssetFilePath("mainVS.vs").c_str() );
	main_shader.attachFragmentShader( getAssetFilePath("mainFS.fs").c_str() );
	main_shader.link();

	particle_shader.generateProgramObject();
	particle_shader.attachVertexShader( getAssetFilePath("particleVS.vs").c_str() );
	particle_shader.attachFragmentShader( getAssetFilePath("particleFS.fs").c_str() );
	particle_shader.link();

	shadow_shader.generateProgramObject();
	shadow_shader.attachVertexShader( getAssetFilePath("shadowVS.vs").c_str() );
	shadow_shader.attachFragmentShader( getAssetFilePath("shadowFS.fs").c_str() );
	shadow_shader.link();

	screen_shader.generateProgramObject();
	screen_shader.attachVertexShader( getAssetFilePath("screenVS.vs").c_str() );
	screen_shader.attachFragmentShader( getAssetFilePath("screenFS.fs").c_str() );
	screen_shader.link();

	lens_shader.generateProgramObject();
	lens_shader.attachVertexShader( getAssetFilePath("lenflareVS.vs").c_str() );
	lens_shader.attachFragmentShader( getAssetFilePath("lenflareFS.fs").c_str() );
	lens_shader.link();


}



//---------------------------------------------------
void Zelda::enableVertexShaderInputSlots()
{
	//-- Enable input slots for vao_meshData:
	{
		glBindVertexArray(vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		positionAttribLocation = main_shader.getAttribLocation("position");
		glEnableVertexAttribArray(positionAttribLocation);

		positionAttribLocation2 = main_shader.getAttribLocation("position2");
		glEnableVertexAttribArray(positionAttribLocation2);
		// Enable the vertex shader attribute location for "normal" when rendering.
		normalAttribLocation = main_shader.getAttribLocation("normal");
		glEnableVertexAttribArray(normalAttribLocation);

		// Enable the vertex shader attribute location for "uv" when rendering.
		UVAttribLocation = main_shader.getAttribLocation("uv");
		glEnableVertexAttribArray(UVAttribLocation);
		CHECK_GL_ERRORS;
		glBindVertexArray(0);
	}


	//particle 
	{	
		glBindVertexArray(vao_particleData);
		particleVertexLocation = particle_shader.getAttribLocation("vert");
		glEnableVertexAttribArray(particleVertexLocation);

		particlePositionLocation = particle_shader.getAttribLocation("position");
		glEnableVertexAttribArray(particlePositionLocation);

		particleUVLocation = particle_shader.getAttribLocation("uv");
		glEnableVertexAttribArray(particleUVLocation);
		CHECK_GL_ERRORS;
		glBindVertexArray(0);
	}

	{
		glBindVertexArray(vao_screenData);
		screenPositionLocation = screen_shader.getAttribLocation("position");
		glEnableVertexAttribArray(screenPositionLocation);
		screenUVLocation = screen_shader.getAttribLocation("uv");
		glEnableVertexAttribArray(screenUVLocation);
		CHECK_GL_ERRORS;
		glBindVertexArray(0);
	}

	
}

//---------------------------------------------------
void Zelda::uploadVertexDataToVbos (
		const MeshConsolidator & meshConsolidator
) {
	//---------------MESH------------------
	{
		glGenBuffers(1, &vbo_vertexPositions);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexPositions);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes(),
					 meshConsolidator.getVertexPositionDataPtr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;

		glGenBuffers(1, &vbo_vertexPositions2);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexPositions2);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexPositionBytes2(),
					 meshConsolidator.getVertexPositionDataPtr2(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;




		glGenBuffers(1, &vbo_vertexNormals);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexNormals);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexNormalBytes(),
					 meshConsolidator.getVertexNormalDataPtr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;


		glGenBuffers(1, &vbo_vertexUVs);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexUVs);

		glBufferData(GL_ARRAY_BUFFER, meshConsolidator.getNumVertexUVBytes(),
					 meshConsolidator.getVertexUVDataPtr(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		CHECK_GL_ERRORS;
	}



	//---------------------------_PARTICLE

	//data for the partilcle, this one does not involve lua file
	static const GLfloat quads[] = {
 		0.5f, 0.5f, 1.0f,
 		1.5f, 0.5f, 1.0f,
 		0.5f, 1.5f, 1.0f,
 		1.5f, 1.5f, 1.0f,
	};

	static const GLfloat quadsuv[] = {
		0.0f,1.0f,
		1.0f,1.0f,
		0.0f,0.0f,
		1.0f,0.0f,


	};

	{
		glGenBuffers(1, &vbo_particles);

		glBindBuffer(GL_ARRAY_BUFFER,vbo_particles);

		glBufferData(GL_ARRAY_BUFFER, sizeof(quads), 
					 quads, GL_STATIC_DRAW);
		CHECK_GL_ERRORS;

		glGenBuffers(1, &vbo_particles_uv);

		glBindBuffer(GL_ARRAY_BUFFER,vbo_particles_uv);

		glBufferData(GL_ARRAY_BUFFER, sizeof(quadsuv), 
					 quadsuv, GL_STATIC_DRAW);
		CHECK_GL_ERRORS;

		//positions
		glGenBuffers(1, &vbo_particles_position);

		glBindBuffer(GL_ARRAY_BUFFER,vbo_particles_position);

		glBufferData(GL_ARRAY_BUFFER, NUM_PARTICLES * 4 * sizeof(GLfloat), 
					 NULL, GL_STREAM_DRAW);
		CHECK_GL_ERRORS;

	}


	//----- screen
	static const GLfloat screenquads[] = {
		-1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,

        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f,
	};

	static const GLfloat screenUVs[] = {
		0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
	};
		
	{
		glGenBuffers(1, &vbo_screenPosition);

		glBindBuffer(GL_ARRAY_BUFFER,vbo_screenPosition);

		glBufferData(GL_ARRAY_BUFFER, sizeof(screenquads), 
					 screenquads, GL_STATIC_DRAW);
		CHECK_GL_ERRORS;


		glGenBuffers(1, &vbo_screenUV);

		glBindBuffer(GL_ARRAY_BUFFER,vbo_screenUV);

		glBufferData(GL_ARRAY_BUFFER, sizeof(screenUVs), 
					 screenUVs, GL_STATIC_DRAW);
		CHECK_GL_ERRORS;
	}

}


//---------------------------------------------------
void Zelda::bindBuffer()
{
	//meshes
	{
		glBindVertexArray(vao_meshData);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexPositions);
		glVertexAttribPointer(positionAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexPositions2);
		glVertexAttribPointer(positionAttribLocation2, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexNormals);
		glVertexAttribPointer(normalAttribLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_vertexUVs);
		glVertexAttribPointer(UVAttribLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		//-- Unbind target, and restore default values:
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CHECK_GL_ERRORS;
	}

	{
		glBindVertexArray(vao_screenData);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_screenPosition);
		glVertexAttribPointer(screenPositionLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		glBindBuffer(GL_ARRAY_BUFFER, vbo_screenUV);
		glVertexAttribPointer(screenUVLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

		//-- Unbind target, and restore default values:
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		CHECK_GL_ERRORS;

	}
}






//---------------------------------------------------
void Zelda::initPerspectiveMatrix()
{
	float aspect = ((float)m_windowWidth) / m_windowHeight;
	perspective = glm::perspective(degreesToRadians(fov), aspect, 0.1f, 100.0f);
}
//---------------------------------------------------
void Zelda::initViewMatrix() {
	view = glm::lookAt(vec3(0.0f, 3.0f, 9.0f), 		//look from
						 vec3(0.0f, 0.0f, -1.0f),		//look at
						 vec3(0.0f, 1.0f, 0.0f));
}
//---------------------------------------------------
void Zelda::initLightSources() {
	// World-space position
	light.position = vec3(3.0,5.0,-5.0);
	light.lightView =glm::lookAt(light.position, vec3(0.0f, 0.0f, -1.0f),
			vec3(0.0f, 1.0f, 0.0f));
	GLfloat near_plane =-5.0f, far_plane = 15.0f;
	light.projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);  
}


//---------------------------------------------------
void Zelda::uploadCommonSceneUniforms() {
	main_shader.enable();
	{
		//-- Set Perspective matrix uniform for the scene:
		GLint location = main_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(perspective));
		CHECK_GL_ERRORS;


		//-- Set LightSource uniform for the scene:
		{
			location = main_shader.getUniformLocation("light.position");
			glUniform3fv(location, 1, value_ptr(light.position));
			CHECK_GL_ERRORS;

			location = main_shader.getUniformLocation("umbra");
			glUniform1i(location,shadow_mode);
			CHECK_GL_ERRORS;
		}
		
	}
	main_shader.disable();


}

//---------------------------------------------------
/*
 * Called once per frame, before guiLogic().
 */
void Zelda::appLogic()
{
	// Place per frame, application logic here ...
	uploadCommonSceneUniforms();
	frame_count++;

	//-----------MOVEMENT
	stallBoat();

}





























//----------------------------------------------------------------------------//
//																			  //
//									UI 										  //
//																			  //
//----------------------------------------------------------------------------//
//---------------------------------------------------
/*
 * Called once per frame, after appLogic(), but before the draw() method.
 */
void Zelda::guiLogic()
{

	if( !show_gui ) {
		return;
	}

	static bool firstRun(true);
	if (firstRun) {
		ImGui::SetNextWindowPos(ImVec2(50, 50));
		firstRun = false;
	}

	static bool showDebugWindow(true);

	ImGuiWindowFlags windowFlags(ImGuiWindowFlags_AlwaysAutoResize | 
								 ImGuiWindowFlags_MenuBar);
	float opacity(0.5f);

	ImGui::Begin("Properties", &showDebugWindow, ImVec2(100,100), opacity,
			windowFlags);
	if( ImGui::BeginMenuBar()) {
		if(ImGui::BeginMenu("Reder Mode")) {

			if(ImGui::MenuItem("Normal","N", &show_normal)) {
				show_shadow_map = false;
				show_normal = true;
			}
			if(ImGui::MenuItem("Shadow Map","H", &show_shadow_map)) {
				//do stuff
				show_normal = false;
				show_shadow_map = true;
				
			}
			ImGui::EndMenu();
		}

		if(ImGui::BeginMenu("Shading Mode")) {

			if(ImGui::MenuItem("Cel Shading", "C", &show_cel)) {
				show_phong = false;
				show_cel = true;
			}
			if(ImGui::MenuItem("Phong Shading","O", &show_phong)) {
				//do stuff
				show_phong = true;
				show_cel = false;
				
			}
			ImGui::EndMenu();
		}


			
		ImGui::EndMenuBar();
	}


	//bunch of checkboxes
	ImGui::Checkbox("Show Shadow (1)", &toggle_shadow);
	if (toggle_shadow) shadow_mode = 1;
	else shadow_mode = 0;
	ImGui::Checkbox("Show Reflection (2)", &toggle_reflect);

	ImGui::Checkbox("Show Lens Flare (3)", &toggle_flare);
	ImGui::Checkbox("Show Particles System (4)", &toggle_particle);

	ImGui::SliderFloat("Water Noise Level", &perlin_noise_level, 0.0f, 10.0f);



	if( ImGui::Button( "Quit Application (ESC)" ) ) {
			glfwSetWindowShouldClose(m_window, GL_TRUE);
	}
	ImGui::Text( "Framerate: %.1f FPS", ImGui::GetIO().Framerate );

	ImGui::End();
}



































//----------------------------------------------------------------------------//
//																			  //
//									TEXTURE									  //
//																			  //
//----------------------------------------------------------------------------//
Image Zelda::getImage(const std::string& filename) {
	Image ret;
	std::vector<unsigned char> image;
  	unsigned width, height;
  	unsigned error = lodepng::decode(image, width, height, filename);
  	if(error != 0) {
    	std::cout << "error " << error << ": " << lodepng_error_text(error) << std::endl;
  	}else {
  		//make sure no error
  		ret.data = image;
  		ret.width = width;
  		ret.height = height;

  	}
	return ret;
}


//---------------------------------------------------
void Zelda::createTexture() {
	//load texture data
	Image texture_atlas = getImage("Assets/texture.png");

	//assign texture to ID
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F, 
		 		 texture_atlas.width, 
		 		 texture_atlas.height, 
		 		 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 		 &texture_atlas.data[0]);
	//Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS;

    glBindTexture(GL_TEXTURE_2D,0);




    Image lens = getImage("Assets/lenscolor.png");
    	//assign texture to ID
	glGenTextures(1, &lenColorID);
	glBindTexture(GL_TEXTURE_1D, lenColorID);
	glTexImage1D(GL_TEXTURE_1D, 0,GL_RGBA32F, 
		 		 lens.width, 
		 		 0, GL_RGBA,  GL_UNSIGNED_BYTE,
		 		 &lens.data[0]);
	//Parameters
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS;

    glBindTexture(GL_TEXTURE_1D,0);


    Image login_screen = getImage("Assets/login.png");

	//assign texture to ID
	glGenTextures(1, &loginID);
	glBindTexture(GL_TEXTURE_2D, loginID);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F, 
		 		 login_screen.width, 
		 		 login_screen.height, 
		 		 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 		 &login_screen.data[0]);
	//Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS;

    glBindTexture(GL_TEXTURE_2D,0);

    Image particle_img = getImage("Assets/bubble.png");
    	//assign texture to ID
	glGenTextures(1, &particleID);
	glBindTexture(GL_TEXTURE_2D, particleID);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA32F, 
		 		 particle_img.width, 
		 		 particle_img.height, 
		 		 0, GL_RGBA, GL_UNSIGNED_BYTE,
		 		 &particle_img.data[0]);
	//Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS;

    glBindTexture(GL_TEXTURE_2D,0);
}
  	

//-----------
void Zelda::shadowFrame() {
	glGenFramebuffers(1, &depth_fbo);

	glGenTextures(1, &depthID);
	glBindTexture(GL_TEXTURE_2D,depthID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				 m_windowWidth,m_windowHeight,0,
				 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
 	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	CHECK_GL_ERRORS;

	//bind buffer to the depth
	glBindFramebuffer(GL_FRAMEBUFFER,depth_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, 
						   GL_DEPTH_ATTACHMENT, 
						   GL_TEXTURE_2D, depthID, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    CHECK_GL_ERRORS;


    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "WARNING: TROUBLE --- Framebuffer is not complete." << endl;
     glBindFramebuffer(GL_FRAMEBUFFER, 0);


} 











//-----------------
void Zelda::postFrame() {
	glGenFramebuffers(1,&post_fbo);


	//--generate the colour attachment1
	glGenTextures(1,&postID);
	glBindTexture(GL_TEXTURE_2D,postID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 
				m_windowWidth,m_windowHeight, 0, 
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS;


	//--- colour 2
	glGenTextures(1,&flareID);
	glBindTexture(GL_TEXTURE_2D,flareID);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 
				m_windowWidth,m_windowHeight, 0, 
				GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	CHECK_GL_ERRORS;




	//---generate the depth and stencil attachment
	glGenRenderbuffers(1, &post_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, post_rbo); 
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 
    					  m_windowWidth, m_windowHeight); // Use a single renderbuffer object for both a depth AND stencil buffer.
    CHECK_GL_ERRORS;

	//bind the buffer
	glBindFramebuffer(GL_FRAMEBUFFER,post_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, 
						   GL_COLOR_ATTACHMENT0, 
						   GL_TEXTURE_2D, postID, 0);

	glBindFramebuffer(GL_FRAMEBUFFER,post_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, 
						   GL_COLOR_ATTACHMENT1, 
						   GL_TEXTURE_2D, flareID, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
    						  GL_RENDERBUFFER, post_rbo);
    CHECK_GL_ERRORS;



    GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);


    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        cout << "WARNING: TROUBLE --- Framebuffer is not complete." << endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CHECK_GL_ERRORS;

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
}



















//----------------------------------------------------------------------------//
//																			  //
//										DRAW 								  //
//																			  //
//----------------------------------------------------------------------------//




//---------------------------------------------------
/*
 * Called once per frame, after guiLogic().
 */
void Zelda::draw() {
	if (frame_stall < 100) frame_stall++;
	if (login_mode && frame_stall > 70 && frame_stall != 999) {
			login_mode = false;
	}
	if (!login_mode) {


		//draw flag 0 : normal without sun
		//			1 :
		glViewport(0,0,m_windowWidth,m_windowHeight);
		//enable the OPENGL to blend alpha with background to produce tranparency
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
		glEnable( GL_DEPTH_TEST );

		glClearColor(0.529, 0.808, 0.980,1.0);
		
		


		//----------------render to depth map first
		glViewport(0,0,m_windowWidth,m_windowHeight);
		glBindFramebuffer(GL_FRAMEBUFFER,depth_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);

		drawObjects(*rootNode,1);
		

		//render normally
		glClear(GL_STENCIL_BUFFER_BIT);

		

		glBindFramebuffer(GL_FRAMEBUFFER, post_fbo); 
		glViewport(0,0,m_windowWidth,m_windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 

		glActiveTexture(GL_TEXTURE0); // Texture unit 1
		glBindTexture(GL_TEXTURE_2D, textureID);
		glActiveTexture(GL_TEXTURE1); // Texture unit 1
		glBindTexture(GL_TEXTURE_2D, depthID);
		if (toggle_particle) {
			glActiveTexture(GL_TEXTURE0); // Texture unit 1
			glBindTexture(GL_TEXTURE_2D, particleID);
			drawParticles();
			glActiveTexture(GL_TEXTURE0); // Texture unit 1
			glBindTexture(GL_TEXTURE_2D, textureID);
		}

		//sun
		drawObjects(*rootNode,4);

		drawObjects(*rootNode,2);
		
		if (toggle_reflect) {
			glEnable(GL_STENCIL_TEST);
			 // Draw floor
	    	glStencilFunc(GL_ALWAYS, 1, 0xFF); // Set any stencil to 1
	    	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	    	glStencilMask(0xFF); // Write to stencil buffer
	    	glDepthMask(GL_FALSE); // Don't write to depth buffer
	    	glClear(GL_STENCIL_BUFFER_BIT); // Clear stencil buffer (0 by default)
	    	drawObjects(*rootNode,3);


	    	glStencilFunc(GL_EQUAL, 1, 0xFF); // Pass test if stencil value is 1
	    	glStencilMask(0x00); // Don't write anything to stencil buffer
	    	glDepthMask(GL_TRUE); // Write to depth buffer


			drawObjects(*rootNode,5);
		

			glDisable(GL_STENCIL_TEST);
		}else {
			drawObjects(*rootNode,3);
		}
	




		//draw to screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,m_windowWidth,m_windowHeight);
		if (show_normal) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,postID);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D,lenColorID);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D,flareID);
			drawScreen();
		}else {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D,depthID);

			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_1D,lenColorID);

			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D,flareID);
			login_mode = true;
			drawScreen();
			login_mode = false;
		}
		


		drawScreen();
	
	}else {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0,0,m_windowWidth,m_windowHeight);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,loginID);
		drawScreen();

	}




}









//---------------------------------------------------
// Update mesh specific shader uniforms:
static void updateShaderUniforms(
		const ShaderProgram & shader,
		const GeometryNode & node,
		const glm::mat4 & modelMatrix,
		const glm::mat4 & viewMatrix,
		const glm::mat4 & lightMatrix,
		const int what_shader
) {

 	if (what_shader == 0 || what_shader == 5) {	//main shader
 		shader.enable();
		{	



		

			//-- Set ModelView matrix:
			GLint location = shader.getUniformLocation("Model");
			mat4 model = modelMatrix * node.trans;
			glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(model));
			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("View");
			glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(viewMatrix));
			CHECK_GL_ERRORS;



			location = shader.getUniformLocation("tex");
			glUniform1i(location, 0);
			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("shadowMap");
			glUniform1i(location, 1);
			CHECK_GL_ERRORS;




			//-- Set NormMatrix:
			location = shader.getUniformLocation("NormalMatrix");
			mat3 normalMatrix = glm::transpose(glm::inverse(mat3(viewMatrix * modelMatrix)));
			glUniformMatrix3fv(location, 1, GL_FALSE, value_ptr(normalMatrix));
			CHECK_GL_ERRORS;
			
			location = shader.getUniformLocation("lightSpace");
			glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(lightMatrix));
			CHECK_GL_ERRORS;

			{
				//-- Set Material values: (for non texture objects)
				location = shader.getUniformLocation("mat.kd");
				vec3 kd = node.material.kd;
				glUniform3fv(location, 1, value_ptr(kd));
				CHECK_GL_ERRORS;

				location = shader.getUniformLocation("mat.ks");
				vec3 ks= node.material.ks;
				glUniform3fv(location, 1, value_ptr(ks));
				CHECK_GL_ERRORS;


				location = shader.getUniformLocation("mat.shine");
				glUniform1f(location,node.material.shine);
				CHECK_GL_ERRORS;

				//alpha value 
				float opaqueness = node.material.opaque;
				location = shader.getUniformLocation("mat.alpha");
				glUniform1f(location,opaqueness);
				CHECK_GL_ERRORS;

				//alpha value 
				int smode = (show_cel) ? 1 : 0 ;
				location = shader.getUniformLocation("mat.mode");
				glUniform1i(location,smode);
				CHECK_GL_ERRORS;

				//status  value 0.0 for non transparent, 1 for transparent,2 for ignoring texture 
				int stt = node.material.status;
				location = shader.getUniformLocation("mat.status");
				glUniform1i(location,stt);
				CHECK_GL_ERRORS;
			}

			location = shader.getUniformLocation("time");
			glUniform1i(location,frame_count);

			location = shader.getUniformLocation("real_time");
			glUniform1f(location,(sin(glfwGetTime()+ 1)/2));

			location = shader.getUniformLocation("reflect");
			glUniform1i(location,what_shader);
			//1 apply noise for water, 2 apply noise for sail, 0 no noise
			int noise = (node.m_name == "water" || node.m_name == "boatsail") ? 
						 ((node.m_name == "water") ? 1: 2) : 0;
			location = shader.getUniformLocation("noise_level");
			glUniform1i(location,noise);

			CHECK_GL_ERRORS;

			location = shader.getUniformLocation("noiselv");
			glUniform1f(location,perlin_noise_level);


		}
		shader.disable();


	
 	}else if (what_shader == 1) {	//shadow shader
 		shader.enable();
 		{
 			GLint location = shader.getUniformLocation("model");
 			mat4 modelView = modelMatrix * node.trans;
 			glUniformMatrix4fv(location,1,GL_FALSE,value_ptr(modelView));
 			CHECK_GL_ERRORS;

 			location = shader.getUniformLocation("lightSpaceMatrix");
			glUniformMatrix4fv(location,1,GL_FALSE,value_ptr(lightMatrix));
 		}
 		shader.disable();
 	}


}



void Zelda::traverseTree(SceneNode * node,int flag) {

	if (node->m_nodeType == NodeType::GeometryNode) {

		GeometryNode * geometryNode = static_cast<GeometryNode *>(node);	

		//add boat transformation
		if (geometryNode->m_name == "boatbody") {
			if (flag == 5) mat_stack.top() = mat_stack.top() * scale(mat4(),vec3(1.0,-1.0,1.0));
			geometryNode->trans = geometryNode->trans * b_auto * b_rotate;
			b_rotate = mat4();
			b_auto = mat4();
		}




		mat4 lightMatrix = light.projection * light.lightView;
		glBindVertexArray(vao_meshData);
		bool skip_flag = false;	//if this one is turned on ... only draw the water
		//Normal rendering
		if (flag != 1 ) {
        	if (flag == 3) {
        		skip_flag = true;
        		if (geometryNode->m_name == "water") {
        			if (w_translate != 0) {
        				float dir = (w_translate == 1) ? 0.5f : -0.5f;
        				geometryNode->translate(vec3(0,0,dir));
        				w_translate = 0;
        			}

        			if (w_rotate != 0) {
        				float angle = (w_rotate == 1) ? 0.9f : -0.9f;
        				geometryNode->rotate('y',angle);
        				w_rotate = 0;
        			}

        			
        			updateShaderUniforms(main_shader, *geometryNode, mat_stack.top(),view,lightMatrix,0);
					// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
					BatchInfo batchInfo = batchInfoMap[geometryNode->meshId];
					//w_translate = mat4();
					//-- Now render the mesh:
					main_shader.enable();
					glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
					main_shader.disable();
        		}

        	//reflected boat
        	}else if (flag == 5) {
        		if (geometryNode->m_name == "water") {
				}else {
			
					updateShaderUniforms(main_shader, *geometryNode, mat_stack.top(),view,lightMatrix,5);
		
					// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
					BatchInfo batchInfo = batchInfoMap[geometryNode->meshId];

					//-- Now render the mesh:
					main_shader.enable();
					glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
					main_shader.disable();
				}

        	}else {
				if (geometryNode->m_name == "water" && flag == 2) {
				}else {
					updateShaderUniforms(main_shader, *geometryNode, mat_stack.top(),view,lightMatrix,0);

					// Get the BatchInfo corresponding to the GeometryNode's unique MeshId.
					BatchInfo batchInfo = batchInfoMap[geometryNode->meshId];

					//-- Now render the mesh:
					main_shader.enable();
					glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
					main_shader.disable();
				}
        	}
	      


		//render depth test
		}else {
			updateShaderUniforms(shadow_shader, *geometryNode,
								 mat_stack.top(),view,lightMatrix,flag);
			BatchInfo batchInfo = batchInfoMap[geometryNode->meshId];

			shadow_shader.enable();
			glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);
			shadow_shader.disable();
		}

		//if its not skip then draw everythingelse
		if (!skip_flag) {
			mat_stack.push(mat_stack.top() * node->trans );
			for (SceneNode * child: node->children) {
				traverseTree(child,flag);
			}
		mat_stack.pop();
		}
		

		glBindVertexArray(0);
		CHECK_GL_ERRORS;
	}
	
}	


//---------------------------------------------------
void Zelda::drawObjects(const SceneNode & root,int flag) {
	// Bind the VAO once here, and reuse for all GeometryNode rendering below.
	
	//push the parent trans and then apply it to children
	mat_stack.push(root.trans* translate * rotate);
	for (SceneNode * node : root.children) {
		if (node->m_nodeType != NodeType::GeometryNode) {
			continue;
		}
		if (node->m_name == "sun") {

			if (flag == 4) drawSun(node);
		}else {
			if (flag != 4) traverseTree(node,flag);
		}
		

		
	}
	mat_stack.pop();


	
}

















void Zelda::drawSun(SceneNode * node) {
	GeometryNode * gNode = static_cast<GeometryNode *>(node);
	lens_shader.enable();
	{
		glBindVertexArray(vao_meshData);

		// Enable the vertex shader attribute location for "position" when rendering.
		GLuint sunPos = lens_shader.getAttribLocation("position");
		glEnableVertexAttribArray(sunPos);
		CHECK_GL_ERRORS;

		//-- Set ModelView matrix:
		GLint location = lens_shader.getUniformLocation("Model");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(gNode->trans));
		CHECK_GL_ERRORS;

		location = lens_shader.getUniformLocation("Pers");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(perspective));
		CHECK_GL_ERRORS;

		location = lens_shader.getUniformLocation("View");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(view));
		CHECK_GL_ERRORS;

		BatchInfo batchInfo = batchInfoMap[gNode->meshId];
		glDrawArrays(GL_TRIANGLES, batchInfo.startIndex, batchInfo.numIndices);

	}
	lens_shader.disable();
}








void Zelda::drawScreen() {

	screen_shader.enable();
	{	
		GLint location = screen_shader.getUniformLocation("screenTex");
		glUniform1i(location, 0);
		CHECK_GL_ERRORS;

		location = screen_shader.getUniformLocation("lensColor");
		glUniform1i(location, 1);
		CHECK_GL_ERRORS;


		location = screen_shader.getUniformLocation("brightTex");
		glUniform1i(location, 2);
		CHECK_GL_ERRORS;
		
		location = screen_shader.getUniformLocation("login");
		glUniform1i(location,login_mode);

		location = screen_shader.getUniformLocation("showFlare");
		int sf = (toggle_flare) ? 1 : 0;
		glUniform1i(location,sf);


		glBindVertexArray(vao_screenData);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        CHECK_GL_ERRORS;
	}
	screen_shader.disable();
	 

}





void Zelda::drawParticles() {
	makeParticles(10);
	//-------
	int p_count = updateParticles();




	//--- Binding to the buffer
	glBindBuffer(GL_ARRAY_BUFFER,vbo_particles_position);
	glBufferData(GL_ARRAY_BUFFER,NUM_PARTICLES * 4 * sizeof(GLfloat),
				 NULL,GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER,0,
					p_count * sizeof(GLubyte) * 4,particle_position_size);

	
	//draw particles
	particle_shader.enable();

	{
		glBindVertexArray(vao_particleData);

		//bind and update the vertices
		glBindBuffer(GL_ARRAY_BUFFER, vbo_particles);
		glVertexAttribPointer(particleVertexLocation,3,GL_FLOAT,GL_FALSE,0,nullptr);

		//bind and update the vertices
		glBindBuffer(GL_ARRAY_BUFFER, vbo_particles_uv);
		glVertexAttribPointer(particleUVLocation,2,GL_FLOAT,GL_FALSE,0,nullptr);

		//bind and update the position
		glBindBuffer(GL_ARRAY_BUFFER, vbo_particles_position);
		glVertexAttribPointer(particlePositionLocation,4,GL_FLOAT,GL_FALSE,0,nullptr);
		

		//reusing the same set of vertices
		glVertexAttribDivisor(0, 0);
		glVertexAttribDivisor(1, 1); // positions : one per quad (its center)    
		CHECK_GL_ERRORS;
		GLint location = particle_shader.getUniformLocation("tex");
		glUniform1i(location, 0);
		CHECK_GL_ERRORS;

		//up load the P and V
		location = particle_shader.getUniformLocation("ModelView");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(view));
		CHECK_GL_ERRORS;
		
		location = particle_shader.getUniformLocation("Perspective");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(perspective));
		CHECK_GL_ERRORS;


		location = particle_shader.getUniformLocation("Model");
		glUniformMatrix4fv(location, 1, GL_FALSE, value_ptr(b_auto));
		CHECK_GL_ERRORS;
		
		//actually draw
		glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, NUM_PARTICLES);


		//unbind
		glBindVertexArray(0);
	
	}

	particle_shader.disable();

}

//---------------------------------------------------
/*
 * Called once, after program is signaled to terminate.
 */
void Zelda::cleanup()
{

	delete[] particle_position_size;
	delete[] Pcontainer;
	glDeleteBuffers(1, &vbo_vertexUVs);
	glDeleteBuffers(1, &vbo_vertexNormals);
	glDeleteBuffers(1, &vbo_vertexPositions);
	glDeleteTextures(1, &textureID);
	glDeleteVertexArrays(1, &vao_meshData);
	glDeleteVertexArrays(1, &vao_particleData);
}







































//----------------------------------------------------------------------------//
//																			  //
//								GAME LOGIC									  //
//																			  //
//----------------------------------------------------------------------------//

//-----------PARTICLE
int Zelda::findUnused(bool dir) {
	for (int i= last_used;i < NUM_PARTICLES;i++) {
		if ((Pcontainer+i)->life < 0) {
			last_used = i;
			return i;
		}
	}

	for (int i= 0;i < last_used;i++) {
		if ((Pcontainer+i)->life < 0) {
			last_used = i;
			return i;
		}
	}
	
	//there is a higher chance that the one after last used havent used yet
	

	//if all are used do nothing
	return -1;
}

void Zelda::makeParticles(int num) {
	//Genew particle
	float spread = 5.0f;
	for (int i=0;i< num;i++) {
		int index = findUnused(0);

	
		if (index != -1) {
			(Pcontainer + index)->life = 1.0f;	
			(Pcontainer + index)->size = (rand() % 10 + 1)/ 30.5f;
			//starting location
			(Pcontainer + index)->pos = vec3(0.1,2.1,-3.5f);

			vec3 dir = vec3(0.0f,0.0f,-10.0f);

			// randir from -1 to 1
			glm::vec3 spread_dir = glm::vec3(
					(rand()%200 - 100.0f)/100.0f,
					(rand()%200 - 100.0f)/100.0f,
					(rand()%200 - 100.0f)/100.0f
			);

			(Pcontainer + index)->speed = dir + spread_dir*spread;
		}//index2
	}//for loop

}

int Zelda::updateParticles() {
	int p_count = 0;
	for (int i=0;i< NUM_PARTICLES;i++) {
		Particle*  ptc = (Pcontainer+i);
		float delta = 0.005f;



		if (ptc->life > 0.0f) {
			ptc->life -= 0.01;
			if (ptc->life > 0.0f) {
				ptc->speed += vec3(0.0f,-9.8f,0.0f) * delta * 0.9f;
				ptc->pos += (ptc->speed * delta);


				// Fill the GPU buffer
				particle_position_size[4*p_count+0] = ptc->pos.x;
				particle_position_size[4*p_count+1] = ptc->pos.y;
				particle_position_size[4*p_count+2] = ptc->pos.z;
				particle_position_size[4*p_count+3] = ptc->size;
	
			}else {
				//down to oblivion
				ptc->pos = vec3(-999.0f,-999.0f,-999.0f);
			}
			p_count++;
		}
	}
	return p_count;
}

//do the procedure boat movement with based on RNG

void Zelda::stallBoat() {
	//new loop get new seed from 1 to 100
	if (boat_stall == 0) RNGseed = rand() % 100 + 1;

	// if the rng pass the threshold, do the stalling motion for the whole 60 frames
	float speed = (RNGseed < 20) ? 0.0002 : ((RNGseed > 50) ? 0.00045 : -0.00045); 
	int stall_frames = 100;


	if (RNGseed > 80 ) {
		if (boat_stall < stall_frames) {
			b_auto = glm::rotate(b_auto,speed,vec3(1.0,0.0,1.0));
		}else {
			b_auto = glm::rotate(b_auto,-speed,vec3(1.0,0.0,1.0));
		}

	}else if (RNGseed > 60) {

		if (boat_stall < stall_frames) {
			b_auto = glm::rotate(b_auto,speed,vec3(0.0,0.0,1.0));
		}else {
			b_auto = glm::rotate(b_auto,-speed,vec3(0.0,0.0,1.0));
		}

	}else if (RNGseed > 40) {
		if (boat_stall < stall_frames) {
			b_auto = glm::rotate(b_auto,-speed,vec3(0.0,0.0,1.0));
		}else {
			b_auto = glm::rotate(b_auto,speed,vec3(0.0,0.0,1.0));
		}

	}else {
		if (boat_stall < stall_frames) {
			b_auto = glm::rotate(b_auto,speed,vec3(-1.0,0.0,1.0));
		}else {
			b_auto = glm::rotate(b_auto,-speed,vec3(-1.0,0.0,1.0));
		}
	}
		
	boat_stall++;
	//reset 
	if (boat_stall >= stall_frames*2) boat_stall = 0;
	
}

































//----------------------------------------------------------------------------//
//																			  //
//								EVEN HANDLER 								  //
//																			  //
//----------------------------------------------------------------------------//

//---------------------------------------------------
/*
 * Event handler.  Handles cursor entering the window area events.
 */
bool Zelda::cursorEnterWindowEvent (
		int entered
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//---------------------------------------------------
/*
 * Event handler.  Handles mouse cursor movement events.
 */
bool Zelda::mouseMoveEvent (
		double xPos,
		double yPos
) {
	bool eventHandled(false);
	double x_factor = xPos - xOld;
	double y_factor = yPos - yOld;
	double damping = 0.005f;


	if (mouse_hold != -1) {//any of the mouse button is held
		switch(mouse_hold) {
			case GLFW_MOUSE_BUTTON_LEFT:
				b_auto = glm::rotate(b_auto,
									   (float)(x_factor*damping),
									   vec3(0.0,1.0,0.0));
				eventHandled = true;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				view = glm::rotate(view,
									   (float)(x_factor*damping),
									   vec3(0.0,1.0,0.0));
				eventHandled = true;
				break;
		}// switch
	}

	
	//done everything, assign new value to the old coor
	xOld = xPos;
	yOld = yPos;
	return eventHandled;
}

//---------------------------------------------------
/*
 * Event handler.  Handles mouse button events.
 */
bool Zelda::mouseButtonInputEvent (
		int button,
		int actions,
		int mods
) {
	bool eventHandled(false);

	// Fill in with event handling code...
	if (actions == GLFW_PRESS) {
		mouse_hold = button;
		eventHandled = true;
	}else if(actions == GLFW_RELEASE) {
		mouse_hold = -1;
		eventHandled = true;
	}

	return eventHandled;
}

//---------------------------------------------------
/*
 * Event handler.  Handles mouse scroll wheel events.
 */
bool Zelda::mouseScrollEvent (
		double xOffSet,
		double yOffSet
) {
	bool eventHandled(false);

	// Fill in with event handling code...

	return eventHandled;
}

//---------------------------------------------------
/*
 * Event handler.  Handles window resize events.
 */
bool Zelda::windowResizeEvent (
		int width,
		int height
) {
	bool eventHandled(false);
	initPerspectiveMatrix();
	return eventHandled;
}

//---------------------------------------------------
/*
 * Event handler.  Handles key input events.
 */
bool Zelda::keyInputEvent (
		int key,
		int action,
		int mods
) {
	bool eventHandled(false);

	if( action == GLFW_PRESS || action == GLFW_REPEAT ) {
		if (login_mode && frame_stall == 999) {
			frame_stall = 0;
			bSound->drop();
			bSound = irrklang::createIrrKlangDevice();
			bSound->play2D("Assets/theme.mp3",GL_TRUE);
		}

		if (key == GLFW_KEY_W || key ==  GLFW_KEY_S) {

			
		}

		switch (key) {

			case GLFW_KEY_M:
				show_gui = !show_gui;
				eventHandled = true;
				break;
			case GLFW_KEY_ESCAPE:
				glfwSetWindowShouldClose(m_window, GL_TRUE);
				break;
			case GLFW_KEY_1:
				toggle_shadow = !toggle_shadow;
				eventHandled = true;
				break;
			case GLFW_KEY_2:
				toggle_reflect = !toggle_reflect;
				eventHandled = true;
				break;
			case GLFW_KEY_3:
				toggle_flare = !toggle_flare;
				eventHandled = true;
				break;
			case GLFW_KEY_4:
				toggle_particle = !toggle_particle;
				eventHandled = true;
				break;



			case GLFW_KEY_W:
				w_translate = 1;
				eventHandled = true;
				break;
			case GLFW_KEY_A:
				w_rotate = -1;
				eventHandled = true;
				break;
			case GLFW_KEY_D:
				w_rotate = 1;
				eventHandled = true;
				break;
			case GLFW_KEY_S:
				w_translate = -1;
				eventHandled = true;
				break;


			case GLFW_KEY_C:
				show_phong = false;
				show_cel = true;
				break;
			case GLFW_KEY_O:
				show_phong = true;
				show_cel = false;
				break;
			case GLFW_KEY_N:
				show_shadow_map = false;
				show_normal = true;
				break;
			case GLFW_KEY_H:
				show_shadow_map = true;
				show_normal = false;
				break;
		}
	}
	// Fill in with event handling code...

	return eventHandled;
}



