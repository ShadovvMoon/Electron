//
//  render.cpp
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "render.h"

milliseconds now() {
    return duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
}

// Setup
void ERenderer::setup(const char *resources) {
    printf("setup\n");
#ifdef _WINDOWS
    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        printf ("An error occurred with glew %d: %s \n", error, (char *) glewGetErrorString(error));
    }
#endif
    
    // Initilisation
    camera    = new Camera;
    shaders   = new ShaderManager(resources);
    options   = new shader_options;
    bsp       = new BSP(shaders);
    objects   = new ObjectManager(camera, bsp);
    skies     = new SkyManager;
    //interface = new GUI();
    
    // Start the tick
    tick = now();
    forward_tick = now();
    strafe_tick = now();
    
    // Set up the render frame buffer
    glGenFramebuffers(1, &mFBO);
    glGenRenderbuffers(1, &mDiffuse);
    glGenRenderbuffers(1, &mPosition);
    glGenRenderbuffers(1, &mNormals);
    glGenRenderbuffers(1, &mDepthBuffer);
    
    m_width  = 4096;
    m_height = 2048;
    
    // Bind the FBO so that the next operations will be bound to it
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    
    // Bind the diffuse render target
    glBindRenderbuffer(GL_RENDERBUFFER, mDiffuse);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mDiffuse);
    
    // Bind the position render target
    glBindRenderbuffer(GL_RENDERBUFFER, mPosition);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F_ARB, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, mPosition);
    
    // Bind the normal render target
    glBindRenderbuffer(GL_RENDERBUFFER, mNormals);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F_ARB, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_RENDERBUFFER, mNormals);
    
    // Bind the depth buffer
    glBindRenderbuffer(GL_RENDERBUFFER, mDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDepthBuffer);
    
    // Generate and bind the OGL texture for diffuse
    glGenTextures(1, &mDiffuseTexture);
    glBindTexture(GL_TEXTURE_2D, mDiffuseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mDiffuseTexture, 0);
    
    // Generate and bind the OGL texture for positions
    glGenTextures(1, &mPositionTexture);
    glBindTexture(GL_TEXTURE_2D, mPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mPositionTexture, 0);
    
    // Generate and bind the OGL texture for normals
    glGenTextures(1, &mNormalsTexture);
    glBindTexture(GL_TEXTURE_2D, mNormalsTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F_ARB, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, mNormalsTexture, 0);
    
    // Generate and bind the OGL texture for depth
    glGenTextures(1, &mDepthTexture);
    glBindTexture(GL_TEXTURE_2D, mDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // Attach the texture to the FBO
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, mDepthTexture, 0);
    
    // Generate the SSAO texture
    glGenTextures(1, &mSSAOTexture);
    glBindTexture(GL_TEXTURE_2D, mSSAOTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    
    // Check if all worked fine and unbind the FBO
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( status != GL_FRAMEBUFFER_COMPLETE)
        fprintf(stderr, "Can't initialize an FBO render texture. FBO initialization failed.");
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Start
    ready = false;
    printf("ready\n");
}

// Load a map
void ERenderer::write() {
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        objects->write(map, scenarioTag);
    }
}

GLuint reflect_texture;
void ERenderer::read(ProtonMap *map) {
    printf("set map\n");
    this->map = map;

    // Load the map
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        bsp->setup(map, scenarioTag);
        objects->read(shaders, map, scenarioTag);
        skies->read(objects, map, scenarioTag);
        
        // Set the fog
        skies->options(options, map, scenarioTag);
    }
    ready = true;
}

double cotan(double i) { return(1 / tan(i)); }

void ERenderer::resize(float width, float height) {

    // Viewport
    glViewport(0,0,width,height);
    
    double fovy = 45.0;
    double aspect = width / height;
    double znear = 0.1f;
    double zfar = 4000000.0f;
    double f = cotan((fovy / 2.0)); //tan(M_PI_2 - (fovy / 2.0));
    float ymax = znear * tan(fovy * M_PI/360.0);
    float ymin = -ymax;
    float xmax = ymax * aspect;
    float xmin = ymin * aspect;
    
    width  = xmax - xmin;
    height = ymax - ymin;
    
    float depth = zfar - znear;
    float q = -(zfar + znear) / depth;
    float qn = -2 * (zfar * znear) / depth;
    float w = 2 * znear / width;
    float h = 2 * znear / height;
    
    options->perspective[0] = w;
    options->perspective[1] = 0.0;
    options->perspective[2] = 0.0;
    options->perspective[3] = 0.0;
    options->perspective[4] = 0.0;
    options->perspective[5] = h;
    options->perspective[6] = 0.0;
    options->perspective[7] = 0.0;
    options->perspective[8] = 0.0;
    options->perspective[9] = 0.0;
    options->perspective[10] = q;
    options->perspective[11] = -1.0;
    options->perspective[12] = -0.0;
    options->perspective[13] = -0.0;
    options->perspective[14] = qn;
    options->perspective[15] = -0.0;

    #ifndef RENDER_CORE_32
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, (width / height), 0.1f, 4000000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    #endif
}

void errorCheck() {
    // Print any errors
    GLenum error = glGetError();
    if( error != GL_NO_ERROR ) {
        fprintf(stderr, "Opengl error %s\n", gluErrorString( error ));
    }
}

void ERenderer::rightMouseDrag(float dx, float dy) {
    camera->drag(dx, dy);
}

void ERenderer::mouseDrag(float dx, float dy) {
    if (objects->selection.size() > 0) {
        int i;
        for (i=0; i < objects->selection.size(); i++) {
            ObjectInstance *instance = objects->selection[i];
            vector3d *coord = new vector3d(instance->x, instance->y, instance->z);
            vector3d *move = new vector3d(dx/200, -dy/200, 0);
            vector3d *viewDirection = new vector3d(0,0,0);
            if (controller->control) {
                move->x = 0;
                move->y = 0;
                move->z = dy/10;
            }
            viewDirection->set(camera->position);
            viewDirection->sub(coord);
            float s_acceleration = 1.0;
            float mx;
            float my;
            float mz;
            if (controller->control) {
                mx = 0.0;
                my = 0.0;
                mz = s_acceleration * move->z;
            } else {
                mx = (s_acceleration * move->y * viewDirection->x);
                my = (s_acceleration * move->y * viewDirection->y);
                mz = 0.0;
                mx += (s_acceleration * move->x * ((0 * viewDirection->z) - (1 * viewDirection->y)));
                my += (s_acceleration * move->x * ((1 * viewDirection->x) - (0 * viewDirection->z)));
            }
            instance->x += mx;
            instance->y += my;
            instance->z += mz;
            delete coord;
            delete move;
            delete viewDirection;
        }
    }
}

void ERenderer::mouseDown(float dx, float dy) {
    if (controller == nullptr) {
        return;
    }
    
    objects->select(controller->shift, dx, dy);
}

void ERenderer::applyControl(Control *control){
    if (!ready) {
        return;
    }
    
    milliseconds current = now();
    double seconds = (current.count() - tick.count()) / 1000.0;
    double forward_seconds = (current.count() - forward_tick.count()) / 1000.0;
    double  strafe_seconds = (current.count() -  strafe_tick.count()) / 1000.0;
    
    float speed = 1;
    float fspeed = speed * forward_seconds;
    float sspeed = speed * strafe_seconds;
    if (control->forward && !control->back) {
        camera->move(fspeed);
    } else if (control->back) {
        camera->move(-fspeed);
    } else {
        forward_tick = now();
    }
    
    if (control->left && !control->right) {
        camera->strafe(-sspeed);
    } else if (control->right) {
        camera->strafe(+sspeed);
    } else {
        strafe_tick = now();
    }
    controller = control;
    tick = now();
}

void ERenderer::renderScene(bool fast) {
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();
        HaloScenarioTag *scenario = (HaloScenarioTag *)(scenarioTag->Data());
        
        // Render the sky
        float cut = options->fogcut;
        options->fogcut = 0.0;
        shader *scex = shaders->get_shader(shader_SCEX);
        scex->start(options);
        skies->render(shader_SCEX);
        scex->stop();
        shader *schi = shaders->get_shader(shader_SCHI);
        schi->start(options);
        skies->render(shader_SCHI);
        schi->stop();
        options->fogcut = cut;

        // Render everything else
        //glAlphaFunc(GL_GREATER, 0.1);
        for (int pass = ShaderStart; pass <= ShaderEnd; pass++ )
        {
            if (fast && (/*pass == shader_SCEX || pass == shader_SCHI || pass == shader_SGLA ||*/pass == shader_SWAT)) continue;
            #ifdef RENDER_CORE_32
            if (pass != shader_SENV && pass != shader_SOSO && pass != shader_SCEX && pass != shader_SCHI) continue;
            #endif
            
            ShaderType type = static_cast<ShaderType>(pass);
            shader *shader = shaders->get_shader(type);
            shader->start(options);
            bsp->render(type);
            if (render_objects) {
                GLuint number = 0;
                objects->render(&number, nullptr, type, options);
            }
            shader->stop();
        }
    }
}

int NextHighestPowerOf2(int n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}


void ERenderer::start() {
    // Bind our FBO and set the viewport to the proper size
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    glPushAttrib(GL_VIEWPORT_BIT);
    glViewport(0, 0, m_width, m_height);
    
    // Clear the render targets
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_2D);
    
    // Specify what to render an start acquiring
    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
        GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, buffers);
}

void ERenderer::stop(){
    // Stop acquiring and unbind the FBO
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glPopAttrib();
}


// Main rendering loop
void ERenderer::render() {
    if (!ready) {
        return;
    }
    errorCheck();

    // Setup the current viewport
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(options->fogr, options->fogg, options->fogb,1.0);
    glColor4f(1.0,1.0,1.0,1.0);
    
    // Enable GL states
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    #ifdef RENDER_CORE_32
    camera->look(options);
    renderScene(false);
    #else
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera->look(options);
    
    
    if (false) {//shaders->needs_reflection()) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        // Render a reflection of the scenario
        GLint anViewport[4];
        glGetIntegerv(GL_VIEWPORT, anViewport);
        
		float reflectionHeight = shaders->reflection_height();
        glPushMatrix();
        glScalef(1.0, 1.0, -1.0);
        glTranslatef(0.0f, 0.0f, -2*reflectionHeight);
        double plane[4] = {0.0, 0.0, 1.0, reflectionHeight}; //water at y=0
        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, plane);
        renderScene(true);
        glPopMatrix();
        glDisable(GL_CLIP_PLANE0);
        
        // Export the reflection into the reflection texture
        glBindTexture(GL_TEXTURE_2D, shaders->get_reflection(reflection_z));
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0,0, NextHighestPowerOf2(anViewport[2]), NextHighestPowerOf2(anViewport[3]), 0);
        
        // Clear previous frame values
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(options->fogr, options->fogg, options->fogb, 1.0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopAttrib();
    }

    if (!useSSAO) {
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        renderScene(false);
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopAttrib();
    } else {
        // Render the scene into the FBO
        this->start();
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        renderScene(false);
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopAttrib();
        this->stop();
        

        
        
        // Render the quad
        // Projection setup
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0,m_width,0,m_height,0.1f,2);
        
        // Model setup
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        
        
        
        
        
        
        
        // Render the SSAO into a texture
        shader *ssao = shaders->get_shader(shader_SSAO);
        ssao->start(options);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mDiffuseTexture);
        
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mPositionTexture);
        
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mNormalsTexture);
        
        glActiveTexture(GL_TEXTURE3);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mDepthTexture);

        // Render the quad
        glLoadIdentity();
        glColor3f(1,0,0);
        glTranslatef(0,0,-1.0);
        
        glBegin(GL_QUADS);
        glTexCoord2f( 0, 0 );
        glVertex3f(    0.0f, 0.0f, 0.0f);
        glTexCoord2f( 1, 0 );
        glVertex3f(   (float) m_width, 0.0f, 0.0f);
        glTexCoord2f( 1, 1 );
        glVertex3f(   (float) m_width, (float) m_height, 0.0f);
        glTexCoord2f( 0, 1 );
        glVertex3f(    0.0f,  (float) m_height, 0.0f);
        glEnd();
        
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE3);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        ssao->stop();
        
        glBindTexture(GL_TEXTURE_2D, mSSAOTexture);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0,0, m_width, m_height, 0);
        

        // Blur (Y)
        shader *blur = shaders->get_shader(shader_BLUR);
        blur->start(options);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mDiffuseTexture);
        
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mPositionTexture);
        
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mNormalsTexture);
        
        glActiveTexture(GL_TEXTURE3);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mSSAOTexture);
        
        // Render the quad
        glLoadIdentity();
        glColor3f(1,0,0);
        glTranslatef(0,0,-1.0);
        
        glBegin(GL_QUADS);
        glTexCoord2f( 0, 0 );
        glVertex3f(    0.0f, 0.0f, 0.0f);
        glTexCoord2f( 1, 0 );
        glVertex3f(   (float) m_width, 0.0f, 0.0f);
        glTexCoord2f( 1, 1 );
        glVertex3f(   (float) m_width, (float) m_height, 0.0f);
        glTexCoord2f( 0, 1 );
        glVertex3f(    0.0f,  (float) m_height, 0.0f);
        glEnd();
        
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE3);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        blur->stop();
        
        glBindTexture(GL_TEXTURE_2D, mSSAOTexture);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0,0, m_width, m_height, 0);
        
        
        
        
        
        
        
        
        
        
        
        
        
        
        // Render the final scene
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(options->fogr, options->fogg, options->fogb, 1.0);
        
        shader *deferred = shaders->get_shader(shader_DEFF);
        deferred->start(options);
        
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mDiffuseTexture);

        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mPositionTexture);

        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mNormalsTexture);

        glActiveTexture(GL_TEXTURE3);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mSSAOTexture);

        // Render the quad
        glLoadIdentity();
        glColor3f(1,0,0);
        glTranslatef(0,0,-1.0);
        
        glBegin(GL_QUADS);
        glTexCoord2f( 0, 0 );
        glVertex3f(    0.0f, 0.0f, 0.0f);
        glTexCoord2f( 1, 0 );
        glVertex3f(   (float) m_width, 0.0f, 0.0f);
        glTexCoord2f( 1, 1 );
        glVertex3f(   (float) m_width, (float) m_height, 0.0f);
        glTexCoord2f( 0, 1 );
        glVertex3f(    0.0f,  (float) m_height, 0.0f);
        glEnd();
        
        // Reset OpenGL state
        glActiveTexture(GL_TEXTURE0);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        glActiveTexture(GL_TEXTURE3);
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        deferred->stop();
        
        
        //Reset to the matrices	
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
    }
    #endif

    
    // Render the shadow map
    /*
     GLuint ShadowMap;
     glBindFramebuffer(GL_FRAMEBUFFER, FBO);
     glDrawBuffers(0, NULL); glReadBuffer(GL_NONE);
     glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMap, 0);
     
     glClear(GL_DEPTH_BUFFER_BIT);
     glEnable(GL_DEPTH_TEST);
     glEnable(GL_CULL_FACE);
     glCullFace(GL_FRONT);
     renderScene(false);
     glDisable(GL_CULL_FACE);
     glDisable(GL_DEPTH_TEST);
     
     return;
     */
    
    // Render the GUI
    //GLint anViewport[4];
    //glGetIntegerv(GL_VIEWPORT, anViewport);
    // interface->render(anViewport[2], anViewport[3]);
    
    // Intersection debugging
    /*
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenarioTag = map->tags.at(map->principal_tag).get();

        glUseProgram(0);
        glLineWidth(20.0);
        glColor4f(1.0, 0.0, 0.0, 1.0);
        glDisable(GL_ALPHA_TEST);
        glDisable(GL_TEXTURE_2D);
        glBegin(GL_LINES);
        vector3d *temp1 = new vector3d(camera->view);
        temp1->sub(camera->position);
        temp1->mul(100);
        temp1->add(camera->position);
        glVertex3f(camera->position->x, camera->position->y, camera->position->z);
        glVertex3f(temp1->x, temp1->y, temp1->z);
        glEnd();
        
        vector3d *intersect = bsp->intersect(camera->position, temp1, map, scenarioTag);
        if (intersect != nullptr) {
            printf("intersect %f %f %f\n", intersect->x, intersect->y, intersect->z);
            
            // Render something at the intersection point
            glPushMatrix();
            glTranslatef(intersect->x, intersect->y, intersect->z);
            GLUquadric *sphere=gluNewQuadric();
            gluQuadricDrawStyle( sphere, GLU_FILL);
            gluQuadricNormals( sphere, GLU_SMOOTH);
            gluQuadricOrientation( sphere, GLU_OUTSIDE);
            gluQuadricTexture( sphere, GL_TRUE);
            gluSphere(sphere,0.1,30,30);
            gluDeleteQuadric ( sphere );
            glPopMatrix();
        }
        delete temp1;
    }
    */
}

// Cleanup
ERenderer::~ERenderer() {
    printf("deleting renderer\n");
    delete camera;
    delete shaders;
    delete bsp;
    delete objects;
}