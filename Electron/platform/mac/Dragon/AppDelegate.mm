//
//  AppDelegate.m
//  Dragon
//
//  Created by Samuco on 9/05/2015.
//  Copyright (c) 2015 Samuco. All rights reserved.
//

#import "AppDelegate.h"
#import "bitmaps.h"
#import "ERenderView.h"
#import "../../OpenGL/render/render.h"
#include <ctime>
#include <queue>

@interface AppDelegate ()

@property IBOutlet NSWindow *window;
@end

@implementation AppDelegate

NSDate *start = [NSDate date];
int tick = 0;
ERenderView *renderView;

/*
struct NavigationNode;
struct NavigationLink {
    NavigationNode *start;
    NavigationNode *end;
    float cost;
};
struct NavigationNode {
    uint32_t surface;
    std::vector<NavigationLink*> links;
};
*/
/*
-(void)generateNavigationMesh {
    
}

void renderSurface(ProtonTag *bspTag, CollisionBSP *collision, uint16_t s) {
    Surface *surf = surface(bspTag, collision, s);
    uint32_t startEdge = surf->edge;
    Edge *start = edge(bspTag, collision, startEdge);
    Edge *current = start;
    do {
        CollVert *v1 = vert(bspTag, collision, current->startVert);
        CollVert *v2 = vert(bspTag, collision, current->endVert);
        glVertex3f(v1->x, v1->y, v1->z);
        glVertex3f(v2->x, v2->y, v2->z);
        if (current->leftSurface == s) {
            current = edge(bspTag, collision, current->forwardEdge);
        } else {
            current = edge(bspTag, collision, current->prevEdge);
        }
    } while (current != start);
}

vector3d *surfaceCenter(ProtonTag *bspTag, CollisionBSP *collision, uint32_t s) {
    Surface *surf = surface(bspTag, collision, s);
    uint32_t startEdge = surf->edge;
    Edge *start = edge(bspTag, collision, startEdge);
    Edge *current = start;
    
    float mx = 0.0;
    float my = 0.0;
    float mz = 0.0;
    int total = 0;
    do {
        total++;
        if (current->leftSurface == s) {
            CollVert *v1 = vert(bspTag, collision, current->startVert);
            mx += v1->x;
            my += v1->y;
            mz += v1->z;
            current = edge(bspTag, collision, current->forwardEdge);
        } else {
            CollVert *v2 = vert(bspTag, collision, current->endVert);
            mx += v2->x;
            my += v2->y;
            mz += v2->z;
            current = edge(bspTag, collision, current->prevEdge);
        }
    } while (current != start);
    mx /= total;
    my /= total;
    mz /= total;
    return new vector3d(mx, my, mz);
}

float surfaceWeight(ProtonTag *bspTag, CollisionBSP *collision, uint32_t s1, uint32_t s2) {
    vector3d *m1 = surfaceCenter(bspTag, collision, s1);
    vector3d *m2 = surfaceCenter(bspTag, collision, s2);
    float distance = m1->distance(m2);
    free(m1);
    free(m2);
    return distance;
}

uint32_t surfaceId(ProtonTag *bspTag, CollisionBSP *collision, intersection *intersect, vector3d *pos) {
    float closest = 10000.0;
    uint32_t outsurface = -1;
    while (intersect->next != NULL) {
        uint32_t s   = intersect->surface;
        if (s != -1) {
            
            intersect->accurate ? glColor4f(1.0, 0.0, 0.0, 1.0) : glColor4f(1.0, 1.0, 0.0, 1.0);
            glLineWidth(10.0);
            glBegin(GL_LINES);
            renderSurface(bspTag, collision, s);
            glEnd();
            
            Surface *surf = surface(bspTag, collision, s);
            uint32_t startEdge = surf->edge;
            Edge *start = edge(bspTag, collision, startEdge);
            Edge *current = start;
            
            float mx = 0.0;
            float my = 0.0;
            float mz = 0.0;
            int total = 0;
            do {
                total++;
                if (current->leftSurface == s) {
                    CollVert *v1 = vert(bspTag, collision, current->startVert);
                    mx += v1->x;
                    my += v1->y;
                    mz += v1->z;
                    current = edge(bspTag, collision, current->forwardEdge);
                } else {
                    CollVert *v2 = vert(bspTag, collision, current->endVert);
                    mx += v2->x;
                    my += v2->y;
                    mz += v2->z;
                    current = edge(bspTag, collision, current->prevEdge);
                }
            } while (current != start);
            mx /= total;
            my /= total;
            mz /= total;
            
            float distance = sqrtf(powf(mx - pos->x, 2) + powf(my - pos->y, 2));
            if (distance < closest) {
                closest = distance;
                outsurface = s;
            }
        }
        intersect = intersect->next;
    }
    return outsurface;
}

struct surfaceExtra {
    uint32_t closest;
    bool visited;
    float weight;
};

inline float vdistsqr(const float* a, const float* b) {
    return sqrtf(powf(a[0]-b[0], 2) + powf(a[1]-b[1], 2));
}
inline float vdistsqr3(const float* a, const float* b) {
    return sqrtf(powf(a[0]-b[0], 2) + powf(a[1]-b[1], 2) + powf(a[2]-b[2], 2));
}

inline float triarea3(const float* a, const float* b, const float* c)
{
    const float ax = b[0] - a[0];
    const float ay = b[1] - a[1];
    const float bx = c[0] - a[0];
    const float by = c[1] - a[1];
    return bx*ay - ax*by;
    
    const float x1 = b[0] - a[0];
    const float x2 = b[1] - a[1];
    const float x3 = b[2] - a[2];
    const float y1 = c[0] - a[0];
    const float y2 = c[1] - a[1];
    const float y3 = c[2] - a[2];
    return 0.5*sqrt(pow(x2*y3-x3*y2, 2)+pow(x3*y1-x1*y3, 2)+pow(x1*y2-x2*y1, 2));
}
inline float triarea2(const float* a, const float* b, const float* c)
{
    const float ax = b[0] - a[0];
    const float ay = b[1] - a[1];
    const float bx = c[0] - a[0];
    const float by = c[1] - a[1];
    return bx*ay - ax*by;
}

inline bool vequal(const float* a, const float* b)
{
    static const float eq = 0.001f*0.001f;
    return vdistsqr(a, b) < eq;
}
inline bool vequal3(const float* a, const float* b)
{
    static const float eq = 0.001f*0.001f;
    return vdistsqr3(a, b) < eq;
}
bool collequal(CollVert *a, CollVert *b) {
    static const float eq = 0.001f*0.001f;
    return sqrtf(powf(a->x-b->x, 2) + powf(a->y-b->y, 2) + powf(a->z-b->z, 2)) < eq;
}

void vcpy(float *a, const float *b) {
    a[0] = b[0];
    a[1] = b[1];
    a[2] = b[2];
}

int stringPull(const float* portals, int nportals,
               float* pts, const int maxPts)
{
    // Find straight path.
    int npts = 0;
    // Init scan state
    float portalApex[3], portalLeft[3], portalRight[3];
    int apexIndex = 0, leftIndex = 0, rightIndex = 0;
    vcpy(portalApex, &portals[0]);
    vcpy(portalLeft, &portals[0]);
    vcpy(portalRight, &portals[3]);
    
    // Add start point.
    vcpy(&pts[npts*3], portalApex);
    npts++;
    
    for (int i = 1; i < nportals && npts < maxPts; ++i)
    {
        const float* left = &portals[i*6+0];
        const float* right = &portals[i*6+3];
        
        // Update right vertex.
        if (triarea3(portalApex, portalRight, right) <= 0.0f)
        {
            if (vequal3(portalApex, portalRight) || triarea3(portalApex, portalLeft, right) > 0.0f)
            {
                // Tighten the funnel.
                vcpy(portalRight, right);
                rightIndex = i;
            }
            else
            {
                // Right over left, insert left to path and restart scan from portal left point.
                vcpy(&pts[npts*3], portalLeft);
                npts++;
                // Make current left the new apex.
                vcpy(portalApex, portalLeft);
                apexIndex = leftIndex;
                // Reset portal
                vcpy(portalLeft, portalApex);
                vcpy(portalRight, portalApex);
                leftIndex = apexIndex;
                rightIndex = apexIndex;
                // Restart scan
                i = apexIndex;
                continue;
            }
        }
        
        // Update left vertex.
        if (triarea3(portalApex, portalLeft, left) >= 0.0f)
        {
            if (vequal3(portalApex, portalLeft) || triarea3(portalApex, portalRight, left) < 0.0f)
            {
                // Tighten the funnel.
                vcpy(portalLeft, left);
                leftIndex = i;
            }
            else
            {
                // Left over right, insert right to path and restart scan from portal right point.
                vcpy(&pts[npts*3], portalRight);
                npts++;
                // Make current right the new apex.
                vcpy(portalApex, portalRight);
                apexIndex = rightIndex;
                // Reset portal
                vcpy(portalLeft, portalApex);
                vcpy(portalRight, portalApex);
                leftIndex = apexIndex;
                rightIndex = apexIndex;
                // Restart scan
                i = apexIndex;
                continue;
            }
        }
    }
    // Append last point to path.
    if (npts < maxPts)
    {
        vcpy(&pts[npts*3], &portals[(nportals-1)*6+0]);
        npts++;
    }
    
    return npts;
}
*/

//surfaceExtra *visited = NULL;
-(void)render {
    
    NSTimeInterval timeInterval = -[start timeIntervalSinceNow];
    start = [NSDate date];
    tick++;
    
    ERenderer *render = (ERenderer *)[renderView renderer];
    render->render_objects = false;
    
    
    // BSP pathfinding
    //render->bsp->
    
    
    
    
    vector3d *start = new vector3d(92.47775, -156.4127 , 1.704486);
    
    Client *main = clients[0];
    Game *game = main->game;
    if (game != nullptr) {
        int i;
        for (i=0; i < kMaxPlayers; i++) {
            Player *p = game->getPlayer(i);
            if (p != nullptr && p->alive) {
                Biped *bipd = game->getBiped(p->biped_number);
                if (bipd != nullptr && bipd->alive) {
                    start->set(bipd->position);
                }
            }
        }
    }
    
    vector3d *end   = new vector3d(43.1904, -82.22294, 1.704486);
    
    // Render the spheres
    glUseProgram(0);
    glColor4f(1.0, 1.0, 0.0, 1.0);
    glPushMatrix();
    glTranslatef(start->x, start->y, start->z);
    GLUquadric *sphere=gluNewQuadric();
    gluQuadricDrawStyle( sphere, GLU_FILL);
    gluQuadricNormals( sphere, GLU_SMOOTH);
    gluQuadricOrientation( sphere, GLU_OUTSIDE);
    gluQuadricTexture( sphere, GL_TRUE);
    gluSphere(sphere,0.2,10,10);
    gluDeleteQuadric ( sphere );
    glPopMatrix();
    glColor4f(0.0, 1.0, 1.0, 1.0);
    glPushMatrix();
    glTranslatef(end->x, end->y, end->z);
    sphere=gluNewQuadric();
    gluQuadricDrawStyle( sphere, GLU_FILL);
    gluQuadricNormals( sphere, GLU_SMOOTH);
    gluQuadricOrientation( sphere, GLU_OUTSIDE);
    gluQuadricTexture( sphere, GL_TRUE);
    gluSphere(sphere,0.2,10,10);
    gluDeleteQuadric ( sphere );
    glPopMatrix();
    glColor4f(1.0, 1.0, 1.0, 1.0);

    /*
    Edge *edgeCache[20];
    uint16_t scenarioTag = render->map->principal_tag;
    if (scenarioTag != NULLED_TAG_ID) {
        ProtonTag *scenario = render->map->tags.at(render->map->principal_tag).get();
        CollisionBSP *collision = nullptr;
        ProtonTag *bspTag = nullptr;
        HaloTagReflexive bsp = ((HaloScenarioTag*)scenario->Data())->bsp;
        int i, m;
        for (i=0; i < bsp.count; i++) {
            BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bsp.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
            bspTag = render->map->tags.at((uint16_t)(chunk->tagId)).get();
            uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
            BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
            HaloTagReflexive collisionBSP = mesh->collBSP;
            for (m=0; m < bsp.count; m++) {
                collision = (CollisionBSP *)(map2mem(bspTag, collisionBSP.address) + 96 * m);
                break;
            }
            break;
        }
        if (collision == nullptr) {
            return;
        }
        
        // Find the start surface
        vector3d *startm = new vector3d(start);
        vector3d *endm   = new vector3d(end);
        start ->z += 0.5;
        end   ->z += 0.5;
        startm->z -= 0.5;
        endm  ->z -= 0.5;
        intersection *start_report = new intersection;
        start_report->next = NULL;
        vector3d *start_loc = render->bsp->intersect_report(start, startm, render->map, scenario, start_report);
        intersection *end_report   = new intersection;
        end_report->next = NULL;
        vector3d *end_loc   = render->bsp->intersect_report(end  , endm  , render->map, scenario, end_report);
        uint32_t start_surface = -1;
        if (start_loc != nullptr) {
            start_surface = surfaceId(bspTag, collision, start_report, start_loc);
        }
        
        uint32_t end_surface  = -1;
        if (end_loc != nullptr) {
            end_surface = surfaceId(bspTag, collision, end_report, end_loc);
        }
        
        // Render the surfaces as purple
        glColor4f(1.0, 0.0, 1.0, 1.0);
        glLineWidth(5.0);
        glBegin(GL_LINES);
        if (start_surface != 0xFFFFFFFF) {
            renderSurface(bspTag, collision, start_surface);
        }
        if (end_surface   != 0xFFFFFFFF) {
            renderSurface(bspTag, collision,   end_surface);
        }
        glEnd();
        
        if (start_surface < collision->Surfaces.count && end_surface < collision->Surfaces.count) {

            // Find the shortest path using A*
            // Generate the edge list
            std::clock_t start;
            start = std::clock();
            HaloTagReflexive surfaces = collision->Surfaces;
            HaloTagReflexive verts    = collision->Vertices;

            visited = (surfaceExtra *)malloc(surfaces.count * sizeof(surfaceExtra));
            for (int s = 0; s < surfaces.count; s++) {
                visited[s].visited = false;
                visited[s].closest = -1;
                visited[s].weight  = 10000.0;
            }
            visited[start_surface].weight = 0.0;
            
            struct Comp
            {
                bool operator()(const uint32_t& s1, const uint32_t& s2)
                {
                    return visited[s1].weight > visited[s2].weight;
                }
            };

            //fprintf(stderr, "pathfinding...\n");
            std::priority_queue<uint32_t, std::vector<uint32_t>, Comp> heap = std::priority_queue<uint32_t, std::vector<uint32_t>, Comp>();
            heap.push(start_surface);

            while (!heap.empty()) {
                uint32_t s = heap.top();
                heap.pop();
                
                // Loop over the connected surfaces
                visited[s].visited = true;
                
                Surface *surf = surface(bspTag, collision, s);
                uint32_t startEdge = surf->edge;
                Edge *start = edge(bspTag, collision, startEdge);
                Edge *current = start;
                Edge *previous;
                do {
                    previous = current;
                    uint32_t neighbour = -1;
                    if (current->leftSurface == s) {
                        neighbour = current->rightSurface;
                        current = edge(bspTag, collision, current->forwardEdge);
                    } else {
                        neighbour = current->leftSurface;
                        current = edge(bspTag, collision, current->prevEdge);
                    }
                    
                    // Compute the distance between these two surfaces
                    if (neighbour == -1) continue;
                    if (visited[neighbour].visited) continue;
                    
                    // Can we walk on this plane?
                    Surface *neighsurf = surface(bspTag, collision, neighbour);
                    uint32_t pla = (neighsurf->plane & 0x7FFFFFFF);
                    Plane *p = plane(bspTag, collision, pla);
                    float a = p->a;
                    float b = p->b;
                    float c = p->c;
                    
                    // Is this division possible?
                    if (sqrt(a*a+ b*b+ c*c) == 0) {
                        continue;
                    }
                    
                    float angle = acosf(c / sqrt(a*a+ b*b+ c*c));
                    if (angle > 45.0/180.0 * M_PI) {
                        continue;
                    }

                    float weight = surfaceWeight(bspTag, collision, s, neighbour);
                    float nweight = visited[s].weight + weight;
                    if (nweight < visited[neighbour].weight) {
                        visited[neighbour].weight = nweight;
                        visited[neighbour].closest = s;

                        if (neighbour == end_surface) {
                            std::priority_queue<uint32_t, std::vector<uint32_t>, Comp>empty;
                            std::swap(heap, empty);
                            break;
                        }
                        heap.push(neighbour);
                    }
                } while (current != start);
            }
            
            glColor4f(1.0, 0.0, 1.0, 1.0);
            glLineWidth(1.0);
            
            // Render the triangles
            glBegin(GL_LINES);
            uint32_t next = end_surface;
            while (next != -1) {
                renderSurface(bspTag, collision, next);
                next = visited[next].closest;
            }
            glEnd();
            glLineWidth(2.0);
            glColor4f(0.0, 0.0, 1.0, 1.0);
  
            struct vertexMarker {
                bool visited;
                bool left;
             };
             vertexMarker *marked = (vertexMarker *)malloc(verts.count * sizeof(vertexMarker));
             for (int s = 0; s < verts.count; s++) {
                 marked[s].visited = false;
                 marked[s].left    = false;
             }
             
             CollVert *previous_left   = nullptr;
             CollVert *previous_right  = nullptr;
             Edge     *previous_portal = nullptr;
             next = end_surface;
            
            
            int maxportals = 5000;
            int nportals = 0;
            
            float *portals = (float *)malloc(sizeof(float)*6*maxportals);
            portals[nportals*6+0] = end_loc->x;
            portals[nportals*6+1] = end_loc->y;
            portals[nportals*6+2] = end_loc->z;
            portals[nportals*6+3] = end_loc->x;
            portals[nportals*6+4] = end_loc->y;
            portals[nportals*6+5] = end_loc->z;
            nportals++;
            
            //std::vector<Edge *> portals = std::vector<Edge *>();
            while (visited[next].closest != -1) {
                Surface *surf1 = surface(bspTag, collision, next);
                Surface *surf2 = surface(bspTag, collision, visited[next].closest);
                
                uint32_t startEdge = surf1->edge;
                Edge *start = edge(bspTag, collision, startEdge);
                Edge *current = start;
                int edges = 0;
                do {
                    if (current->leftSurface == next) {
                        current = edge(bspTag, collision, current->forwardEdge);
                    } else {
                        current = edge(bspTag, collision, current->prevEdge);
                    }
                    edges++;
                } while (current != start);
                
                start = edge(bspTag, collision, startEdge);
                current = start;
                do {
                    if (current->leftSurface == next) {
                        if (current->rightSurface == visited[next].closest) break;
                        current = edge(bspTag, collision, current->forwardEdge);
                    } else if (current->rightSurface == next) {
                        if (current->leftSurface == visited[next].closest) break;
                        current = edge(bspTag, collision, current->prevEdge);
                    }
                } while (current != start);
                Edge *portal = current;
   
            
                // Mark the vertices
                if (!marked[current->startVert].visited &&
                    !marked[current->endVert  ].visited) {
                    if (edges % 2 == 0) {
                        marked[current->startVert ].left = false;
                        marked[current->endVert   ].left = true;
                    } else {
                        marked[current->startVert ].left = true;
                        marked[current->endVert   ].left = false;
                    }
                } else {
                    if (marked[current->startVert].visited) {
                        marked[current->endVert  ].left = !marked[current->startVert].left;
                    } else {
                        marked[current->startVert].left = !marked[current->endVert  ].left;
                    }
                }
                
                
                marked[current->startVert].visited = true;
                marked[current->endVert  ].visited = true;
                
                CollVert *v1 = vert(bspTag, collision, current->startVert);
                CollVert *v2 = vert(bspTag, collision, current->endVert);
                CollVert *leftv, *rightv;
                rightv  = marked[current->startVert].left ? v1 : v2;
                leftv  = marked[current->startVert].left ? v2 : v1;
                
                portals[nportals*6+0] = leftv->x;
                portals[nportals*6+1] = leftv->y;
                portals[nportals*6+2] = leftv->z;
                portals[nportals*6+3] = rightv->x;
                portals[nportals*6+4] = rightv->y;
                portals[nportals*6+5] = rightv->z;
                nportals++;
                
                glPushMatrix();
                glTranslatef(leftv->x, leftv->y, leftv->z);
                GLUquadric *sphere=gluNewQuadric();
                gluQuadricDrawStyle( sphere, GLU_FILL);
                gluQuadricNormals( sphere, GLU_SMOOTH);
                gluQuadricOrientation( sphere, GLU_OUTSIDE);
                gluQuadricTexture( sphere, GL_TRUE);
                gluSphere(sphere,0.2,10,10);
                gluDeleteQuadric ( sphere );
                glPopMatrix();
                
                
                glPushMatrix();
                glTranslatef(rightv->x, rightv->y, rightv->z);
                sphere=gluNewQuadric();
                gluQuadricDrawStyle( sphere, GLU_FILL);
                gluQuadricNormals( sphere, GLU_SMOOTH);
                gluQuadricOrientation( sphere, GLU_OUTSIDE);
                gluQuadricTexture( sphere, GL_TRUE);
                gluSphere(sphere,0.1,10,10);
                gluDeleteQuadric ( sphere );
                glPopMatrix();
                
                
                next = visited[next].closest;
                //portals.push_back(portal);
            }

            portals[nportals*6+0] = start_loc->x;
            portals[nportals*6+1] = start_loc->y;
            portals[nportals*6+2] = start_loc->z;
            portals[nportals*6+3] = start_loc->x;
            portals[nportals*6+4] = start_loc->y;
            portals[nportals*6+5] = start_loc->z;
            nportals++;
            
            int i;
            float z = 10.0;
            
            glColor4f(0.0, 1.0, 0.0, 1.0);
            glLineWidth(3.0);

            int maxPoints = 500;
            float *points = (float *)malloc(maxPoints * sizeof(float) * 3);
            int ptCount = stringPull(portals, nportals, points, maxPoints);
            
            glBegin(GL_LINES);
            
            if (ptCount > 0) {
                glVertex3f(points[3*0 + 0], points[3*0 + 1], points[3*0 + 2]);
            }
            for (i=1; i < ptCount-1; i++) {
                float x = points[3*i + 0];
                float y = points[3*i + 1];
                float z = points[3*i + 2] + 0.1;
                glVertex3f(x, y, z);
                glVertex3f(x, y, z);
            }
            if (ptCount > 0) {
                glVertex3f(points[3*(ptCount-1) + 0], points[3*(ptCount-1) + 1], points[3*(ptCount-1) + 2]);
            }
            glEnd();
            
            //std::cout << "Points: " << ptCount << " with Portals: " << nportals << std::endl;
            
            free(points);
            free(portals);
            
    
            
            free(visited);
            //std::cout << "Time: " << (std::clock() - start) / (double)(CLOCKS_PER_SEC / 1000) << " ms" << std::endl;
                
        }
    }

    */
    
    if (game != nullptr) {
        
        shader_options *options = new shader_options;
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnableClientState(GL_VERTEX_ARRAY);
        for (int pass = ShaderStart; pass <= ShaderEnd; pass++ )
        {
            ShaderType type = static_cast<ShaderType>(pass);
            shader *shader = render->objects->modelManager->shaders->get_shader(type);
            shader->start(options);
            
            for (int i = 0; i < kMaxObjects; i++) {
                NetworkObject *object = game->getObject(i);
                if (object->table_id != -1) {
                    glPushMatrix();
                    glTranslatef(object->position.x, object->position.y, object->position.z);
                    glRotatef((object->rotation.z * 180 / M_PI) + 180, 0.0, 0.0, 1.0); //1.5%
                    
                    HaloTagID tag = *new HaloTagID;
                    tag.table_id  = object->table_id;
                    tag.tag_index = object->tag_index;
                    HaloTagDependency dependency = *new HaloTagDependency;
                    dependency.tag_id = tag;
                    ObjectRef *reference = render->objects->create_object(render->map, dependency);
                    if (reference) {
                        reference->render(type);
                    }
                    glPopMatrix();
                }
                shader->stop();
            }
        }
        glDisableClientState(GL_VERTEX_ARRAY);
        glPopAttrib();
        glUseProgram(0);
        
        
        for (int i=0; i< kMaxBipeds; i++) {
            //Player *p = game->getPlayer(i);
            //if (p && p->alive) {
            
            
                Biped *pb = game->getBiped(i);
                if (pb && pb->alive) {
                    
                   
                    glUseProgram(0);
                    glEnable(GL_COLOR_MATERIAL);
                    if (i == 0) {
                        glColor4f(0.0, 1.0, 0.0, 1.0);
                    } else if (pb->team == 0) {
                        glColor4f(1.0, 0.0, 0.0, 1.0);
                    } else {
                        glColor4f(0.0, 0.0, 1.0, 1.0);
                    }
                    
                    glLineWidth(2.0);
                    glPushMatrix();
                    glTranslatef(pb->position->x, pb->position->y, pb->position->z + 0.75);
                    
                    GLUquadric *sphere=gluNewQuadric();
                    gluQuadricDrawStyle( sphere, GLU_FILL);
                    gluQuadricNormals( sphere, GLU_SMOOTH);
                    gluQuadricOrientation( sphere, GLU_OUTSIDE);
                    gluQuadricTexture( sphere, GL_TRUE);
                    gluSphere(sphere,0.2,10,10);
                    gluDeleteQuadric ( sphere );
                    //glPopMatrix();
                    
                    float distance = 1.0;
                    float x = distance * cos(pb->controls.look_y) * cos(pb->controls.look_x);
                    float y = distance * cos(pb->controls.look_y) * sin(pb->controls.look_x);
                    float z = distance * sin(pb->controls.look_y);
                    
                    glRotatef((pb->controls.look_x * 180 / M_PI) + 180, 0.0, 0.0, 1.0); //1.5%
                   
                    
                    
                    
                    
                    
                    
                    
                    
                    
                    // Move the player
                    float forwardSpeed      = 2.25 * timeInterval;
                    float backwardsSpeed    = 2 * timeInterval;
                    float strafeSpeed       = 2 * timeInterval;
                    
                    float div = 0.0;
                    float dx  = 0.0;
                    float dy  = 0.0;
                    
                    if (pb->controls.movement.forwards == true && pb->controls.movement.backwards == true) {
                    } else if (pb->controls.movement.forwards == true) {
                        dx += x * forwardSpeed;
                        dy += y * forwardSpeed;
                        div += 1;
                    } else if (pb->controls.movement.backwards == true) {
                        dx -= x * backwardsSpeed;
                        dy -= y * backwardsSpeed;
                        div += 1;
                    }
             
                    float tx = y;
                    float ty = -x;
                    if (pb->controls.movement.left == true && pb->controls.movement.right == true) {
                    } else if (pb->controls.movement.left == true) {
                        dx -= tx * strafeSpeed;
                        dy -= ty * strafeSpeed;
                        div += 1;
                    } else if (pb->controls.movement.right == true) {
                        dx += tx * strafeSpeed;
                        dy += ty * strafeSpeed;
                        div += 1;
                    }
                    
                    if (div > 0.9) {
                        pb->position->x += dx / div;
                        pb->position->y += dy / div;
                    }
                    
                    NetworkObject *biped_object = game->getObject(pb->object_index);
                    if (biped_object) {
                        biped_object->position.x = pb->position->x;
                        biped_object->position.y = pb->position->y;
                        biped_object->position.z = pb->position->z;
                        biped_object->rotation.z = pb->controls.look_x;
                    }
                    
                    // Fall to the bsp
                    glTranslatef(0.0, 0.0, -0.75);
                    glPopMatrix();

                    float gravity = 5.0 * timeInterval;
                    uint16_t scenarioTag = render->map->principal_tag;
                    if (scenarioTag != NULLED_TAG_ID) {
                        ProtonTag *scenarioTag = render->map->tags.at(render->map->principal_tag).get();
                        
                        float search = 0.5;
                        vector3d *position      = new vector3d(pb->position->x, pb->position->y, pb->position->z+search);
                        vector3d *position_down = new vector3d(pb->position->x, pb->position->y, pb->position->z-search);
                        vector3d *intersect = render->bsp->intersect(position_down, position, render->map, scenarioTag);
                        if (intersect != nullptr) {
                            if (pb->position->z - gravity > intersect->z) {
                                pb->position->z -= gravity;
                            } else {
                                pb->position->z = intersect->z;
                            }
                        } else {
                            pb->position->z -= gravity;
                        }
                    }
                    
                    float line = 100.0;
                    glBegin(GL_LINES);
                    glVertex3f(pb->position->x, pb->position->y, pb->position->z+0.75);
                    glVertex3f(pb->position->x+line*x, pb->position->y+line*y, pb->position->z+0.75+line*z);
                    glEnd();
                }
            //}
        }
    }
    //main->mtx.unlock();
    glColor4f(1.0, 1.0, 1.0, 1.0);
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    
    // Insert code here to initialize your application
    load_bitmaps([[NSString stringWithFormat:@"%@/Library/Application Support/HaloMD/GameData/Maps/bitmaps.map", NSHomeDirectory()] cStringUsingEncoding:NSUTF8StringEncoding]);
    NSString *bloodgulch = [NSString stringWithFormat:@"%@/Library/Application Support/HaloMD/GameData/Maps/frostbite1_1.map", NSHomeDirectory()];
    
    // Create a rendering view
    renderView = [[ERenderView alloc] initWithFrame:NSZeroRect];
    [renderView setFrame:[[_window contentView] bounds]];
    [renderView setAutoresizingMask:NSViewWidthSizable | NSViewHeightSizable];
    [renderView setData:[[NSData alloc] initWithContentsOfFile:bloodgulch]];
    [renderView setDelegate:self];
    [[_window contentView] addSubview:renderView];
    ERenderer *render = (ERenderer *)[renderView renderer];
    
    // Find the main biped
    //ERenderer *render = (ERenderer *)[renderView renderer];
    //ProtonTag *bipedTag = render->map->tags.at(render->map->principal_tag).get();
    
    //uint16_t scenarioTag = render->map->principal_tag;
    

    // Disable app nap
    if ([[NSProcessInfo processInfo] respondsToSelector:@selector(beginActivityWithOptions:reason:)]) {
        id activity;
        activity = [[NSProcessInfo processInfo] beginActivityWithOptions:0x00FFFFFF reason:@"receiving OSC messages"];
        [activity retain];
    }
    
    /*
    char *names[] = {
        "Stormcutter", "Deadly Nadder", "Gronckle", "Rumblehorn", "Hotburple", "Typhoomerang", "Thunderdrum" "Scauldron", "Skrill", "Timberjack",
        "Stormfly", "Fanghook", "Nightfury", "Meatlug", "Razorwhip", "Raincutter", "Windstriker", "Dragon1", "Dragon2", "Dragon3", "Dragon4", "Dragon5"
    };
    */
    
    char *names[] = {
        "Whitemagic", "{WAR} SMEARL", "{WAR} QUEEN", "{WAR} CHIRL", "{WAR} SHMERL", "{WAR} DERL", "GIRLSLAMER" "GIRLPLAMER", "GIRLNAMER", "GIRLFAMER",
        "GIRLKAMER", "GIRLDAMER", "GIRLZAMER", "GIRLEAMER", "GIRLQAMER", "GIRLPAMER", "Windstriker", "Dragon1", "Dragon2", "Dragon3", "Dragon4", "Dragon5"
    };

    
    char pname[255];
    sprintf(pname, "Dragon");
    
    
    //int m;
    //for (m=0; m < 1; m++) {
    int i;
    for (i=0; i < 1; i++) {
        sprintf(pname, "Wyvern %d", i);
        
        
        
        
        uint16_t scenarioTag = render->map->principal_tag;
        if (scenarioTag != NULLED_TAG_ID) {
            ProtonTag *scenarioTag = render->map->tags.at(render->map->principal_tag).get();
            ai_test *ai = new ai_test(render->bsp, render->map, scenarioTag);
            //clients[i]  = new Client(names[i], "104.236.157.223", 5000, ai);
            clients[i]  = new Client(names[i], "146.135.40.157", 2302, ai);
        }
    
        
        //clients[i]  = new Client(names[i], "198.58.124.27", 4000);
        //clients[i]  = new Client(names[i], "23.92.54.3", 2309);
        usleep(2500000);
    }
    //    sleep(1);
    //}
    
    //clients[0]  = new Client(pname, "107.191.57.51", 5000);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
}

@end
