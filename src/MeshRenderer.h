#ifndef MESHRENDERER_H
#define MESHRENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QWheelEvent>
#include "MeshTopology.h"
#include <Eigen/Dense>

class MeshRenderer : public QOpenGLWidget, protected QOpenGLFunctions {
    Q_OBJECT

public:
    explicit MeshRenderer(QWidget *parent = nullptr);
    void setMesh(const MeshTopology &mesh);

    //view modes here
    void setDisplayModes(bool showFaces, bool showWireframe, bool showVertices);

    //here are the shading controls
    void setShadingModes(bool enableLighting, bool smoothShading);

    void setColors(QColor bg, QColor mesh, QColor wireframe);

    void setHeatmapModes(bool showHeatmap, int heatmapType); //stress heatmap

    std::vector<double> m_curvatures;

    const MeshTopology& getMesh() const { return m_mesh; }

    enum class HeatmapSource { Curvature, Stress, Energy, Temperature };

    void setHeatmapSource(int sourceIndex);


    void setStresses(const std::vector<double>& stresses);

    void setEnergies(const std::vector<double>& energies);

    void setTemperatures(const std::vector<double>& temperatures);

    //ray tracing part
    void setRayPaths(const std::vector<std::vector<Eigen::Vector3d>>& paths);
    void addRayPath(const std::vector<Eigen::Vector3d>& path);
    void clearRays();

    void resetCameraToMesh();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    MeshTopology m_mesh;

    //for faces, wireframe and vertices enable/disable
    bool m_showFaces = true;
    bool m_showWireframe = true;
    bool m_showVertices = false;

    //enable/disable lighting
    bool m_enableLighting = true;
    bool m_smoothShading = true;

    //choice of colors to make it sexy
    QColor m_bgColor = QColor(50, 50, 50); //50, 50, 50
    QColor m_meshColor = QColor(200, 200, 200);
    QColor m_wireColor = QColor(0, 0, 0);

    // this is for camera controls, later i added proper scaling so keep this fix unless neccessary to change
    float m_xRot = 0.0f;
    float m_yRot = 0.0f;
    //float m_zTrans = -5.0f;
    float m_zTrans = -2500.0f;
    QPoint m_lastMousePos;
    float m_zoomSpeed = 0.5f;

    void calculateNormals();
    std::vector<int> getFaceVertices(int faceIndex) const;

    //heatmap vairables, defaults to jet colormap looks like standard FEA stuff
    bool m_showHeatmap = false;
    int m_heatmapType = 0; // 0=Jet, 1=Hot, 2=Cool
    //std::vector<double> m_curvatures;

    void calculateCurvature();
    QColor getColorFromMap(double val, int type);

        int pickVertex(const QPoint& mousePos);

        void drawAxisTriad();


        std::vector<double> m_stresses;
        std::vector<double> m_energies;
        std::vector<double> m_temperatures;

        HeatmapSource m_heatmapSource = HeatmapSource::Curvature; //default set to curvature heatmap

    std::vector<std::vector<Eigen::Vector3d>> m_rayPaths; //storing ray path
};

#endif // MESHRENDERER_H
