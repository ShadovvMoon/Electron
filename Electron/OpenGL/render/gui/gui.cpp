//
//  render.h
//  
//
//  Created by Samuco on 19/04/2015.
//
//

#include "gui.h"

GUI::GUI() {
    
}

#define SIDE_BAR_COLOUR 0.1
#define SIDE_BAR_ALPHA  0.95
#define SIDE_WIDTH 500.0

#define PADDING 10.0
#define SMALL_PADDING 5.0
#define STROKE_SIZE 2.0
#define TEXT_HEIGHT 18.0

void renderString(char *str, float x, float y) {
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glRasterPos3f(x, y, 0.0);
    int i;
    for (i=0; i < strlen(str); i++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
    }
}

void renderRawBox(float x, float y, float w, float h) {
    glBegin(GL_TRIANGLES);
    glVertex3f(x,  y+h, 0.0);
    glVertex3f(x,  y, 0.0);
    glVertex3f(x+w,y+h, 0.0);
    glVertex3f(x+w,y+h, 0.0);
    glVertex3f(x+w,y, 0.0);
    glVertex3f(x,  y, 0.0);
    glEnd();
}
void renderBox(float x, float y, float w, float h) {
    glColor4f(SIDE_BAR_COLOUR, SIDE_BAR_COLOUR, SIDE_BAR_COLOUR, SIDE_BAR_ALPHA);
    renderRawBox(x,y,w,h);
}
void renderStrokeBox(float x, float y, float w, float h, float width, float height) {
    glColor4f(0.5, 0.5, 0.5, 0.5);
    float xstroke = (STROKE_SIZE/width);
    float ystroke = (STROKE_SIZE/height);
    renderRawBox(x-xstroke,y-ystroke,w+2*xstroke,h+2*ystroke);
    renderBox(x,y,w,h);
}
void renderTextField(char *content, float x, float y, float w, float width, float height) {
    float xpadding = SMALL_PADDING / width;
    float ypadding = SMALL_PADDING / height;
    float h = TEXT_HEIGHT / height;
    
    renderStrokeBox(x, y, w, h + 2 * ypadding, width, height);
    renderString(content, x+xpadding, y+ypadding);
}

void renderCategory(char *name, float width, float height) {
    // Draw a category
    float barWidth = SIDE_WIDTH / width;
    float categoryHeight = 300 / height;
    float xpadding = PADDING / width;
    float ypadding = PADDING / height;
    float categoryOffset = ypadding;
    float stringSize = TEXT_HEIGHT / height;
 
    // Render the background
    renderBox(1.0-barWidth+xpadding, 1.0-categoryOffset-categoryHeight, barWidth, categoryHeight);
    renderBox(1.0-barWidth+xpadding, 1.0-categoryOffset-(stringSize+2*ypadding), barWidth, stringSize+2*ypadding);
    
    // Render the title text
    renderString(name, 1.0-barWidth+2*xpadding, 1.0-categoryOffset-ypadding-stringSize);
    
    // Render the body
    float bodyX = 1.0-barWidth+3*xpadding;
    float fieldX = 50 / width;
    float bodyOffset = 1.0-categoryOffset-(stringSize+2*ypadding)-(stringSize+2*ypadding)-ypadding;
    float textFieldWidth = barWidth - 5*xpadding - fieldX;
    
    renderString((char*)"X: ", bodyX, bodyOffset + SMALL_PADDING / height);
    renderTextField((char*)"128.567", bodyX + fieldX, bodyOffset, textFieldWidth, width, height);
    bodyOffset -= 2 * ypadding + TEXT_HEIGHT / height;
    
    renderString((char*)"Y: ", bodyX, bodyOffset + SMALL_PADDING / height);
    renderTextField((char*)"128.567", bodyX + fieldX, bodyOffset, textFieldWidth, width, height);
    bodyOffset -= 2 * ypadding + TEXT_HEIGHT / height;
    
    renderString((char*)"Z: ", bodyX, bodyOffset + SMALL_PADDING / height);
    renderTextField((char*)"128.567", bodyX + fieldX, bodyOffset, textFieldWidth, width, height);
}


void GUI::render(float width, float height) {
    width  /= 2;
    height /= 2;
    
    // Render the side bar
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glUseProgram(0);
    
    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Draw the bar background
    /*
    float barWidth = SIDE_WIDTH / width;
    glBegin(GL_TRIANGLES);
    glVertex3f(1.0-barWidth,  1.0, 0.0);
    glVertex3f(1.0-barWidth, -1.0, 0.0);
    glVertex3f(1.0,           1.0, 0.0);
    glVertex3f(1.0,           1.0, 0.0);
    glVertex3f(1.0,          -1.0, 0.0);
    glVertex3f(1.0-barWidth, -1.0, 0.0);
    glEnd();
    */
    
    renderCategory((char*)"Selection", width, height);
    
}
