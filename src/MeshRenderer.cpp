#include "MeshRenderer.h"
#include <cmath>
#include <algorithm>
#include <QMatrix4x4>
#include <QVector3D>

MeshRenderer::MeshRenderer(QWidget *parent) : QOpenGLWidget(parent) {}

void MeshRenderer::setMesh(const MeshTopology &mesh) {
    m_mesh = mesh;
    calculateNormals(); //trigger calculateCurvature()
    update();
}

void MeshRenderer::setDisplayModes(bool showFaces, bool showWireframe, bool showVertices) {
    m_showFaces = showFaces;
    m_showWireframe = showWireframe;
    m_showVertices = showVertices;
    update();
}

void MeshRenderer::setShadingModes(bool enableLighting, bool smoothShading) {
    m_enableLighting = enableLighting;
    m_smoothShading = smoothShading;
    update();
}

void MeshRenderer::setColors(QColor bg, QColor mesh, QColor wireframe) {
    m_bgColor = bg;
    m_meshColor = mesh;
    m_wireColor = wireframe;
    update();
}

void MeshRenderer::setHeatmapModes(bool showHeatmap, int heatmapType) {
    m_showHeatmap = showHeatmap;
    m_heatmapType = heatmapType;
    update();
}

void MeshRenderer::initializeGL() {
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    //here we setup directional light
    GLfloat lightPos[] = { 1.0f, 1.0f, 1.0f, 0.0f };
    GLfloat lightAmbient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat lightDiffuse[]  = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat lightSpecular[] = { 0.5f, 0.5f, 0.5f, 1.0f };

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    glEnable(GL_LIGHT0);
}

/*
void MeshRenderer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(w) / (h ? h : 1);
    float fovY = 30.0f;
    float zNear = 1.0f, zFar = 50.0f;
    float fH = tan(fovY / 360.0f * 3.14159f) * zNear;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
}
*/


void MeshRenderer::resizeGL(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = static_cast<float>(w) / (h ? h : 1);
    float fovY = 30.0f;

    //extending the far clipping plane to 10000
    float zNear = 1.0f, zFar = 10000.0f;

    float fH = tan(fovY / 360.0f * 3.14159f) * zNear;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, zNear, zFar);
    glMatrixMode(GL_MODELVIEW);
}

void MeshRenderer::paintGL() {
    glClearColor(m_bgColor.redF(), m_bgColor.greenF(), m_bgColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    glTranslatef(0.0f, 0.0f, m_zTrans);
    glRotatef(m_xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(m_yRot, 0.0f, 1.0f, 0.0f);

    if (m_mesh.vertices.empty()) return;

    //shading modes
    if (m_enableLighting) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);

    if (m_smoothShading) glShadeModel(GL_SMOOTH);
    else glShadeModel(GL_FLAT);

    //draw Faces
    if (m_showFaces) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        //if heatmap is ON, tell OpenGL to use our vertex colors instead of the flat mat
        if (m_showHeatmap) {
            glEnable(GL_COLOR_MATERIAL);
            glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        } else {
            glDisable(GL_COLOR_MATERIAL);
            GLfloat matDiffuse[] = { static_cast<GLfloat>(m_meshColor.redF()),
                                    static_cast<GLfloat>(m_meshColor.greenF()),
                                    static_cast<GLfloat>(m_meshColor.blueF()), 1.0f };
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, matDiffuse);
        }

        GLfloat matSpecular[] = { 0.2f, 0.2f, 0.2f, 1.0f };
        glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpecular);
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 32.0f);

        for (size_t i = 0; i < m_mesh.faces.size(); ++i) {
            std::vector<int> vList = getFaceVertices(i);
            glBegin(GL_POLYGON);
            for (int vIdx : vList) {
                const Vertex &v = m_mesh.vertices[vIdx];

                //heatmap color per vertex
                if (m_showHeatmap) {
                    double val = 0.0;
                    if (m_heatmapSource == HeatmapSource::Curvature && vIdx < (int)m_curvatures.size()) {
                        val = m_curvatures[vIdx];
                    }
                    else if (m_heatmapSource == HeatmapSource::Stress && vIdx < (int)m_stresses.size()) {
                        val = m_stresses[vIdx];
                    }
                    else if (m_heatmapSource == HeatmapSource::Energy && vIdx < (int)m_energies.size()) {
                        val = m_energies[vIdx];
                    }
                    else if (m_heatmapSource == HeatmapSource::Temperature && vIdx < (int)m_temperatures.size()) {
                        val = m_temperatures[vIdx];
                    }

                    QColor c = getColorFromMap(val, m_heatmapType);
                    glColor3f(c.redF(), c.greenF(), c.blueF());
                }

                glNormal3f(v.nx, v.ny, v.nz);
                glVertex3f(v.x, v.y, v.z);
            }
            glEnd();
        }
        if (m_showHeatmap) glDisable(GL_COLOR_MATERIAL); //clean up state
    }

    //here draw wireframe
    if (m_showWireframe) {
        glDisable(GL_LIGHTING); //shade lights disabled for wireframes
        for (const Edge &e : m_mesh.edges) {
            if (e.sharpness >= 1) {
                glColor3f(1.0f, 0.0f, 0.0f);
                glLineWidth(3.0f);
            } else {
                glColor3f(m_wireColor.redF(), m_wireColor.greenF(), m_wireColor.blueF());
                glLineWidth(1.0f);
            }
            glBegin(GL_LINES);
            glVertex3f(m_mesh.vertices[e.v1].x, m_mesh.vertices[e.v1].y, m_mesh.vertices[e.v1].z);
            glVertex3f(m_mesh.vertices[e.v2].x, m_mesh.vertices[e.v2].y, m_mesh.vertices[e.v2].z);
            glEnd();
        }
        if (m_enableLighting) glEnable(GL_LIGHTING); //restore
    }

    //draw vertices
    /*
    if (m_showVertices) {
        glDisable(GL_LIGHTING);
        glPointSize(6.0f);
        glBegin(GL_POINTS);
        for (const Vertex &v : m_mesh.vertices) {
            if (v.sharpness >= 3) glColor3f(0.0f, 0.0f, 1.0f);
            else if (v.sharpness == 2) glColor3f(0.0f, 1.0f, 0.0f);
            else glColor3f(1.0f, 1.0f, 1.0f);
            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
        if (m_enableLighting) glEnable(GL_LIGHTING); //restore
    }
*/

    if (m_showVertices) {
        glDisable(GL_LIGHTING);
        glPointSize(10.0f);
        glBegin(GL_POINTS);
        for (const Vertex &v : m_mesh.vertices) {
            if (v.isAnchored) {
                glColor3f(1.0f, 0.0f, 0.0f); //RED color for fixed anchors
            }
            else if (v.isSelected) {
                glColor3f(1.0f, 0.0f, 1.0f); //MAGENTA for pulling
            }
            else if (v.sharpness >= 3) glColor3f(0.0f, 0.0f, 1.0f);
            else if (v.sharpness == 2) glColor3f(0.0f, 1.0f, 0.0f);
            else glColor3f(1.0f, 1.0f, 1.0f);

            glVertex3f(v.x, v.y, v.z);
        }
        glEnd();
        if (m_enableLighting) glEnable(GL_LIGHTING); // Restore
    }

    //ray tracing-------------------
    if (!m_rayPaths.empty()) {
        glDisable(GL_LIGHTING); //lasers should glow brightly, no shading
        glLineWidth(3.0f);      //make the beam thick

        for (const auto& path : m_rayPaths) {
            glBegin(GL_LINE_STRIP);
            glColor3f(0.0f, 1.0f, 0.0f); //Neon green for rays
            for (const auto& pt : path) {
                glVertex3f(pt.x(), pt.y(), pt.z());
            }
            glEnd();

            //draw a small red dot at the impact points
            glPointSize(8.0f);
            glBegin(GL_POINTS);
            glColor3f(1.0f, 0.0f, 0.0f);
            for (const auto& pt : path) {
                glVertex3f(pt.x(), pt.y(), pt.z());
            }
            glEnd();
        }

        if (m_enableLighting) glEnable(GL_LIGHTING); //restore lighting
    }

        //ray tracing-------------------

    drawAxisTriad();
}

//mouse interaction for rotation and zooming -- need to add movements with right click or something
//void MeshRenderer::mousePressEvent(QMouseEvent *event) { m_lastMousePos = event->pos(); }
void MeshRenderer::mouseMoveEvent(QMouseEvent *event) {
    int dx = event->x() - m_lastMousePos.x();
    int dy = event->y() - m_lastMousePos.y();
    if (event->buttons() & Qt::LeftButton) { m_xRot += dy * 0.5f; m_yRot += dx * 0.5f; update(); }
    else if (event->buttons() & Qt::RightButton) { m_zTrans += dy * 0.05f; update(); }
    m_lastMousePos = event->pos();
}

void MeshRenderer::wheelEvent(QWheelEvent *event) {
    //m_zTrans += (event->angleDelta().y() / 120.0f) * 50.0f;
    m_zTrans += (event->angleDelta().y() / 120.0f) * m_zoomSpeed;
    update();
}

std::vector<int> MeshRenderer::getFaceVertices(int faceIndex) const {
    std::vector<int> vList;
    const Face &f = m_mesh.faces[faceIndex];
    for (const EdgeRef &ref : f.edgeRefs) {
        vList.push_back(ref.orientation == 0 ? m_mesh.edges[ref.edgeIndex].v1 : m_mesh.edges[ref.edgeIndex].v2);
    }
    return vList;
}

//calculates normal vecs for curvature calculation
void MeshRenderer::calculateNormals() {
    for (Vertex &v : m_mesh.vertices) { v.nx = v.ny = v.nz = 0.0; }

    for (size_t i = 0; i < m_mesh.faces.size(); ++i) {
        std::vector<int> vList = getFaceVertices(i);
        if (vList.size() < 3) continue;
        const Vertex &v1 = m_mesh.vertices[vList[0]];
        const Vertex &v2 = m_mesh.vertices[vList[1]];
        const Vertex &v3 = m_mesh.vertices[vList[2]];

        double d1x = v1.x - v2.x, d1y = v1.y - v2.y, d1z = v1.z - v2.z;
        double d2x = v2.x - v3.x, d2y = v2.y - v3.y, d2z = v2.z - v3.z;
        double nx = d1y * d2z - d1z * d2y, ny = d1z * d2x - d1x * d2z, nz = d1x * d2y - d1y * d2x;

        for (int vIdx : vList) {
            m_mesh.vertices[vIdx].nx += nx;
            m_mesh.vertices[vIdx].ny += ny;
            m_mesh.vertices[vIdx].nz += nz;
        }
    }

    //normalize vectors to length 1, important for later dot product which yeild cos(theta) then we can ignore division by absolute vals
    for (Vertex &v : m_mesh.vertices) {
        double len = std::sqrt(v.nx * v.nx + v.ny * v.ny + v.nz * v.nz);
        if (len > 0) {
            v.nx /= len; v.ny /= len; v.nz /= len;
        }
    }

    //update curvature calcs.
    calculateCurvature();
}

//changing to Gaussian curvature with formula Kv = (2pi - Sigma(theta_i))/A
void MeshRenderer::calculateCurvature() {
    m_curvatures.assign(m_mesh.vertices.size(), 0.0);
    if (m_mesh.vertices.empty()) return;

    double minC = 999999.0, maxC = -999999.0;

    for (size_t i = 0; i < m_mesh.vertices.size(); ++i) {
        const Vertex &v = m_mesh.vertices[i];

        //sum of interior angles around the vertex
        double angleSum = 0.0;

        for (int fIdx : v.faceIndices) {
            std::vector<int> faceVerts = getFaceVertices(fIdx);

            // where vertex_i is in this face
            auto it = std::find(faceVerts.begin(), faceVerts.end(), i);
            if (it == faceVerts.end()) continue;

            int vIndex = std::distance(faceVerts.begin(), it);
            int prevIdx = faceVerts[(vIndex - 1 + faceVerts.size()) % faceVerts.size()];
            int nextIdx = faceVerts[(vIndex + 1) % faceVerts.size()];

            const Vertex &vPrev = m_mesh.vertices[prevIdx];
            const Vertex &vNext = m_mesh.vertices[nextIdx];

            //vectors from current vertex to previous and next
            double vec1x = vPrev.x - v.x, vec1y = vPrev.y - v.y, vec1z = vPrev.z - v.z;
            double vec2x = vNext.x - v.x, vec2y = vNext.y - v.y, vec2z = vNext.z - v.z;

            //here we normalize the vectirs
            double len1 = std::sqrt(vec1x*vec1x + vec1y*vec1y + vec1z*vec1z);
            double len2 = std::sqrt(vec2x*vec2x + vec2y*vec2y + vec2z*vec2z);

            if (len1 > 0 && len2 > 0) {
                double dot = (vec1x*vec2x + vec1y*vec2y + vec1z*vec2z) / (len1 * len2);

            //ensure no float point error
              dot = std::max(-1.0, std::min(1.0, dot));

            angleSum += std::acos(dot);
            }
        }

        //angle defect: 2 * PI - sum of interior angles
        //NOTE:::--->> for boundary vertices, this should be PI - angleSum,
        //however 2pi should work well
        double K = (2.0 * M_PI) - angleSum;

        m_curvatures[i] = K;
        if (K < minC) minC = K;
        if (K > maxC) maxC = K;
    }

    //normalize curvatures to 0 and 1 for the heatmap
    double range = maxC - minC;
    if (range < 1e-6) range = 1.0;
    for (size_t i = 0; i < m_curvatures.size(); ++i) {
        m_curvatures[i] = (m_curvatures[i] - minC) / range;
    }
}

QColor MeshRenderer::getColorFromMap(double val, int type) {
    val = std::max(0.0, std::min(1.0, val));
    double r = 0, g = 0, b = 0;

    if (type == 0) { // le jet: blue -> cyan -> green -> yellow -> red
        if (val < 0.25) { r = 0; g = 4 * val; b = 1; }
        else if (val < 0.5) { r = 0; g = 1; b = 1 - 4 * (val - 0.25); }
        else if (val < 0.75) { r = 4 * (val - 0.5); g = 1; b = 0; }
        else { r = 1; g = 1 - 4 * (val - 0.75); b = 0; }
    } else if (type == 1) { // le hot: black -> red -> yellow -> white
        if (val < 0.33) { r = 3 * val; g = 0; b = 0; }
        else if (val < 0.66) { r = 1; g = 3 * (val - 0.33); b = 0; }
        else { r = 1; g = 1; b = 3 * (val - 0.66); }
    } else if (type == 2) { // le cool: cyan -> magenta
        r = val; g = 1.0 - val; b = 1.0;
    }

    return QColor::fromRgbF(r, g, b);
}


//vertex picking for force
int MeshRenderer::pickVertex(const QPoint& mousePos) {
    if (m_mesh.vertices.empty()) return -1;

    QMatrix4x4 proj;

    float aspect = static_cast<float>(width()) / (height() ? height() : 1);
    //proj.perspective(30.0f, aspect, 1.0f, 50.0f);
    proj.perspective(30.0f, aspect, 1.0f, 10000.0f);

    QMatrix4x4 mv;
    mv.translate(0.0f, 0.0f, m_zTrans);
    mv.rotate(m_xRot, 1.0f, 0.0f, 0.0f);
    mv.rotate(m_yRot, 0.0f, 1.0f, 0.0f);

    QRect rect(0, 0, width(), height());

    double mouseX = mousePos.x();

   double mouseY = height() - mousePos.y();

    int closestVertex = -1;
    double minDistance = 15.0; //selection rad may need to increase for easier selection or mass selection*********************

    for (size_t i = 0; i < m_mesh.vertices.size(); ++i) {
        QVector3D worldPos(m_mesh.vertices[i].x, m_mesh.vertices[i].y, m_mesh.vertices[i].z);
        QVector3D screenPos = worldPos.project(mv, proj, rect);

        double dx = screenPos.x() - mouseX;
        double dy = screenPos.y() - mouseY;
        double dist = std::sqrt(dx*dx + dy*dy);

        if (dist < minDistance && screenPos.z() >= 0.0 && screenPos.z() <= 1.0) {
            minDistance = dist;
            closestVertex = i;
        }
    }

    return closestVertex;
}

//mouse event shift click to choose vertex for force and ctrl click for fixed boundary condition
void MeshRenderer::mousePressEvent(QMouseEvent *event) {
    m_lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        int vIdx = pickVertex(event->pos());
        if (vIdx != -1) {
            if (event->modifiers() & Qt::ControlModifier) {
                //ctrl + click = fix anchor boundary (red)
                m_mesh.vertices[vIdx].isAnchored = !m_mesh.vertices[vIdx].isAnchored;
                m_mesh.vertices[vIdx].isSelected = false; //avoid two selection
                update();
            }
            else if (event->modifiers() & Qt::ShiftModifier) {
                //shift + click = pull (magenta)
                m_mesh.vertices[vIdx].isSelected = !m_mesh.vertices[vIdx].isSelected;
                m_mesh.vertices[vIdx].isAnchored = false; //avoid two selection
                update();
            }
        }
    }
}


//here we draw a nice axis at the left bottom, in futrue x and y and z must be labelled, i do not know how to do it now**************************
void MeshRenderer::drawAxisTriad() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, width(), 0, height(), -100, 100);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glTranslatef(40.0f, 40.0f, 0.0f);
    glRotatef(m_xRot, 1.0f, 0.0f, 0.0f);
    glRotatef(m_yRot, 0.0f, 1.0f, 0.0f);
    glLineWidth(3.0f);
    glBegin(GL_LINES);

    //x axis -- red color
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(30.0f, 0.0f, 0.0f);
    //y axis -- green color
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 30.0f, 0.0f);
    //z axis -- blue color
    glColor3f(0.2f, 0.6f, 1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f); glVertex3f(0.0f, 0.0f, 30.0f);

    glEnd();

    if (m_enableLighting) glEnable(GL_LIGHTING);

    glEnable(GL_DEPTH_TEST);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void MeshRenderer::setHeatmapSource(int sourceIndex) {
    m_heatmapSource = static_cast<HeatmapSource>(sourceIndex);
    update();
}

void MeshRenderer::setStresses(const std::vector<double>& stresses) {
    m_stresses = stresses;
    update();
}


//the ray tracing part
void MeshRenderer::setRayPaths(const std::vector<std::vector<Eigen::Vector3d>>& paths) {
    m_rayPaths = paths;
    update();
}

void MeshRenderer::addRayPath(const std::vector<Eigen::Vector3d>& path) {
    m_rayPaths.push_back(path);
    update();
}

void MeshRenderer::clearRays() {
    m_rayPaths.clear();
    update();
}

void MeshRenderer::resetCameraToMesh()
{
    if (m_mesh.vertices.empty()) return;

    //calculate the physical bounding box
    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    for (const auto& v : m_mesh.vertices) {
        if (v.x < minX) minX = v.x;
        if (v.x > maxX) maxX = v.x;
        if (v.y < minY) minY = v.y;
        if (v.y > maxY) maxY = v.y;
        if (v.z < minZ) minZ = v.z;
        if (v.z > maxZ) maxZ = v.z;
    }

    double spanX = maxX - minX;
    double spanY = maxY - minY;
    double spanZ = maxZ - minZ;

    double maxSpan = spanX;
    if (spanY > maxSpan) maxSpan = spanY;
    if (spanZ > maxSpan) maxSpan = spanZ;
    if (maxSpan == 0.0) maxSpan = 1.0; //avoid division by zero

    //pull the camera back by roughly 1.5x to 2.0x the largest dimension
    m_zTrans = -(maxSpan * 1.5f);
    m_xRot = 0.0f;
    m_yRot = 0.0f;

    //dynamically set the mouse wheel zoom speed so it feels natural
    m_zoomSpeed = maxSpan / 20.0f;
    if (m_zoomSpeed < 0.1f) m_zoomSpeed = 0.1f; //minimum zoom speed limit

    update();
}


void MeshRenderer::setEnergies(const std::vector<double>& energies) {
    m_energies = energies;
    update();
}

void MeshRenderer::setTemperatures(const std::vector<double> &temperatures)
{
    m_temperatures = temperatures;
    update();
}
