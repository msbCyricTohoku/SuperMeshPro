#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "MeshTopology.h"
#include "MeshRenderer.h"
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QDockWidget>
#include <QVBoxLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    void on_actionUndo_triggered();

    void on_actionCatmull_Clark_triggered();
    void on_actionDoo_Sabin_triggered();
    void on_actionLoop_triggered();

    void on_actionToggle_Faces_triggered();
    void on_actionToggle_Wireframe_triggered();
    void on_actionToggle_Vertices_triggered();
    void on_actionToggle_Lighting_triggered();
    void on_actionToggle_Smooth_Shading_triggered();

    void on_actionMesh_Color_triggered();
    void on_actionBackground_Color_triggered();

    void on_actionAbout_triggered();

    void on_actionChange_Wire_Color_triggered();

private:
    Ui::MainWindow *ui;
    MeshRenderer *m_renderer;
    MeshTopology m_currentMesh;

    //the history for unsubdividion
    std::vector<MeshTopology> m_history;

    //for faces, wireframe and vertices enable/disable
    bool m_showFaces;
    bool m_showWireframe;
    bool m_showVertices;

    //enable/disable lighting
    bool m_enableLighting;
    bool m_smoothShading;

    //choice of colors to make it sexy
    QColor m_bgColor;
    QColor m_meshColor;
    QColor m_wireColor;

    void pushHistory();
    void updateRendererState();

    bool m_showHeatmap = false;
    int m_heatmapType = 0;
    QComboBox *m_heatmapSourceCombo;
    QTextEdit *m_statsDisplay; //show the text
    void updateAnalysisPanel(); //calculates the metrics and shows if heatmap is on

    //FEA analysis function
    void FEA_Analysis(double E, double nu, double t, double density, double totalForce, int axis, double visualScale, bool useGravity, bool useNonLinear);

};

#endif // MAINWINDOW_H
