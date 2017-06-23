//
//  ai.cpp
//  Electron
//
//  Created by Samuco on 21/07/2015.
//  Copyright © 2015 Samuco. All rights reserved.
//

#include "ai.hpp"
#include <math.h>
#include <vector>
#include <queue>


void ai::debug( const char* format, ... ) {
    
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
            
            float distance = sqrtf(powf(mx - pos->x, 2) + powf(my - pos->y, 2)/* + powf(mz - pos->z, 2)*/);
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

int ai::stringPull(const float* portals, int nportals,
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

surfaceExtra *visited = NULL;
float *ai::findPath(BSP *bsp, ProtonMap *map, vector3d *start, vector3d *end) {
    
    uint16_t scenarioTag = map->principal_tag;
    if (scenarioTag == NULLED_TAG_ID) {
        return NULL;
    }
    
    ProtonTag *scenario = map->tags.at(map->principal_tag).get();
    CollisionBSP *collision = nullptr;
    ProtonTag *bspTag = nullptr;
    HaloTagReflexive bspReflex = ((HaloScenarioTag*)scenario->Data())->bsp;
    int i, m;
    for (i=0; i < bspReflex.count; i++) {
        BSP_CHUNK *chunk = (BSP_CHUNK *)(map2mem(scenario, bspReflex.address) + sizeof(BSP_CHUNK) * i); // VERIFIED
        bspTag = map->tags.at((uint16_t)(chunk->tagId)).get();
        uint32_t mesh_offset = *(uint32_t *)(bspTag->Data());
        BSP_MESH *mesh = (BSP_MESH *)map2mem(bspTag, mesh_offset);
        HaloTagReflexive collisionBSP = mesh->collBSP;
        for (m=0; m < bspReflex.count; m++) {
            collision = (CollisionBSP *)(map2mem(bspTag, collisionBSP.address) + 96 * m);
            break;
        }
        break;
    }
    if (collision == nullptr) {
        return NULL;
    }
    
    // Find the start surface
    start = new vector3d(start);
    end   = new vector3d(end);
    vector3d *startm = new vector3d(start);
    vector3d *endm   = new vector3d(end);
    
    double delta = 1.0;
    start ->z += delta;
    end   ->z += delta;
    startm->z -= delta;
    endm  ->z -= delta;
    
    intersection *start_report = new intersection;
    start_report->next = NULL;
    vector3d *start_loc = bsp->intersect_report(start, startm, map, scenario, start_report); //leaking
    delete start;
    delete startm;
    
    intersection *end_report   = new intersection;
    end_report->next = NULL;
    vector3d *end_loc   = bsp->intersect_report(end  , endm  , map, scenario, end_report);  //leaking
    delete endm;
    
    uint32_t start_surface = -1;
    if (start_loc != nullptr) {
        start_surface = surfaceId(bspTag, collision, start_report, start_loc);
    }
    uint32_t end_surface  = -1;
    if (end_loc != nullptr) {
        end_surface = surfaceId(bspTag, collision, end_report, end_loc);
    }
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
        
        
        // Render the triangles
        uint32_t next = end_surface;
        while (next != -1) {
            next = visited[next].closest;
        }
        
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
            
            next = visited[next].closest;
        }
        
        portals[nportals*6+0] = start_loc->x;
        portals[nportals*6+1] = start_loc->y;
        portals[nportals*6+2] = start_loc->z;
        portals[nportals*6+3] = start_loc->x;
        portals[nportals*6+4] = start_loc->y;
        portals[nportals*6+5] = start_loc->z;
        nportals++;
        
        int i;
        int maxPoints = 50;
        float *points = (float *)malloc(maxPoints * sizeof(float) * 3);
        
        int ptCount = this->stringPull(portals, nportals, points, maxPoints);
        if (ptCount > maxPoints) {
            return NULL;
        }
        
        float *output = (float *)malloc(maxPoints * sizeof(float) * 3);
        for (i=0; i < maxPoints; i++) {
            output[i*3 + 0] = end->x;
            output[i*3 + 1] = end->y;
            output[i*3 + 2] = end->z;
        }
        delete end;
        for (i=0; i < ptCount; i++) {
            output[3*i + 0] = points[3*(ptCount-i-1) + 0];
            output[3*i + 1] = points[3*(ptCount-i-1) + 1];
            output[3*i + 2] = points[3*(ptCount-i-1) + 2];
        }
        
        std::cout << "Points: " << ptCount << " with Portals: " << nportals << std::endl;
        
        //free(portals);
        //free(points);
        return output;
    }
    
    
    
    return NULL;
}