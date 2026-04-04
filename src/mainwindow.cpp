#include "mainwindow.h"
#include "SubdivisionAlgorithms.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QColorDialog>
#include <cmath>
#include <fstream>
#include <QSlider>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QIcon>
#include <QSpinBox>
#include "FEASolver.h"
#include <QPushButton>
#include <iostream>
#include "RayTracingOptic.h"
#include "HeatTransfer.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow),
    m_showFaces(true), m_showWireframe(true), m_showVertices(false),
    m_enableLighting(true), m_smoothShading(true),
    m_bgColor(50, 50, 50), m_meshColor(200, 200, 200), m_wireColor(20, 150, 100) {

    ui->setupUi(this);

    //supermeshpro icon here
    setWindowIcon(QIcon(":/new/prefix1/resources/icon.png"));

    //opengl render as central widegt
    m_renderer = new MeshRenderer(this);
    setCentralWidget(m_renderer);
    updateRendererState();

    //this is undo defautls to false unless later on
    ui->actionUndo->setEnabled(false);

    //the mesh relaxing heree
    ui->toolBar->addSeparator();

    QLabel *sliderLabel = new QLabel("  Smooth Factor: ", this);
    ui->toolBar->addWidget(sliderLabel);

    QSlider *smoothSlider = new QSlider(Qt::Horizontal, this);
    smoothSlider->setRange(1, 100);
    smoothSlider->setValue(50); //defaults to 0.5 factor

    smoothSlider->setFixedWidth(120);

    ui->toolBar->addWidget(smoothSlider);

    QAction *relaxAction = ui->toolBar->addAction("Relax Mesh");
    connect(relaxAction, &QAction::triggered, this, [this, smoothSlider]() {
        if (m_currentMesh.vertices.empty()) return;

        pushHistory(); //undo for smoothing

        //normalize factor from 0 to 1
        double factor = smoothSlider->value() / 100.0;

        //laplacian smoothing
        SubdivisionAlgorithms::laplacianSmooth(m_currentMesh, factor, 3);

        m_renderer->setMesh(m_currentMesh);
    });

    //heatmap controls
    ui->toolBar->addSeparator();

    QCheckBox *heatmapCheck = new QCheckBox(" Heatmap ", this);
    ui->toolBar->addWidget(heatmapCheck);

    //heatmap combobox dropdown menu
    m_heatmapSourceCombo = new QComboBox(this);
    m_heatmapSourceCombo->addItems({"Curvature", "Von Mises Stress", "Optical Energy", "Temperature"});
    ui->toolBar->addWidget(m_heatmapSourceCombo);

    //combobox connect
    connect(m_heatmapSourceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_renderer->setHeatmapSource(index);
    });

    QComboBox *colorMapCombo = new QComboBox(this);
    colorMapCombo->addItems({"Jet (le Classic)", "Hot (le Thermal)", "Cool (le Stress)"});
    ui->toolBar->addWidget(colorMapCombo);

    //checkbox connect
    connect(heatmapCheck, &QCheckBox::toggled, this, [this](bool checked) {
        m_showHeatmap = checked;
        m_renderer->setHeatmapModes(m_showHeatmap, m_heatmapType);
    });

    //the dropdown menu for heatmap color options using combobox -- taken some from kamakura stuff
    connect(colorMapCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        m_heatmapType = index;
        if (m_showHeatmap) {
            m_renderer->setHeatmapModes(m_showHeatmap, m_heatmapType);
        }
    });

    //dock widget -- taken some from kamakura stuff
    QDockWidget *dock = new QDockWidget("Model Analysis", this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget *dockContent = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(dockContent);

    QLabel *statsLabel = new QLabel("<b>Metrics</b>");
    layout->addWidget(statsLabel);

    QTextEdit *statsDisplay = new QTextEdit();

    statsDisplay->setReadOnly(true);

    statsDisplay->setPlaceholderText("Load a model to see analysis...");
    layout->addWidget(statsDisplay);

    layout->addStretch();
    dock->setWidget(dockContent);
    addDockWidget(Qt::RightDockWidgetArea, dock);

    m_statsDisplay = statsDisplay;


    //########################################################################
    //the sim dock
    QDockWidget *simDock = new QDockWidget("Simulation Environment", this);
    simDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QTabWidget *simTabs = new QTabWidget(simDock);


    //heat transfer tab
    QWidget *thermalTab = new QWidget();
    QVBoxLayout *thermalLayout = new QVBoxLayout(thermalTab);

    thermalLayout->addWidget(new QLabel("<b>Thermal Properties</b>"));

    QDoubleSpinBox *conductivityBox = new QDoubleSpinBox();
    conductivityBox->setRange(0.001, 10000.0); conductivityBox->setValue(45.0); // Steel is ~45
    conductivityBox->setPrefix("Conductivity (k): "); conductivityBox->setSuffix(" W/m·K");
    thermalLayout->addWidget(conductivityBox);

    QDoubleSpinBox *thermalThickBox = new QDoubleSpinBox();
    thermalThickBox->setRange(0.01, 100.0); thermalThickBox->setValue(1.0);
    thermalThickBox->setPrefix("Thickness (t): "); thermalThickBox->setSuffix(" mm");
    thermalLayout->addWidget(thermalThickBox);

    thermalLayout->addSpacing(10);

    thermalLayout->addWidget(new QLabel("<b>Thermal Boundary Conditions</b>"));
    thermalLayout->addWidget(new QLabel("<i>Ctrl+Click = Source, Shift+Click = Sink</i>"));

    QDoubleSpinBox *sourceTempBox = new QDoubleSpinBox();
    sourceTempBox->setRange(-273.15, 5000.0); sourceTempBox->setValue(100.0);
    sourceTempBox->setPrefix("Source Temp (Red): "); sourceTempBox->setSuffix(" °C");
    thermalLayout->addWidget(sourceTempBox);

    QDoubleSpinBox *sinkTempBox = new QDoubleSpinBox();
    sinkTempBox->setRange(-273.15, 5000.0); sinkTempBox->setValue(20.0);
    sinkTempBox->setPrefix("Sink Temp (Magenta): "); sinkTempBox->setSuffix(" °C");
    thermalLayout->addWidget(sinkTempBox);

    thermalLayout->addSpacing(10);

    QPushButton *btnRunThermal = new QPushButton("Run Heat Transfer", this);
    thermalLayout->addWidget(btnRunThermal);
    QPushButton *btnResetThermal = new QPushButton("Clear Thermal Data", this);
    thermalLayout->addWidget(btnResetThermal);
    thermalLayout->addStretch();


    simTabs->addTab(thermalTab, "Heat Transfer");


    //FEA simulation tab
    QWidget *feaTab = new QWidget();
    QVBoxLayout *feaLayout = new QVBoxLayout(feaTab);

    feaLayout->addWidget(new QLabel("<b>FEA Boundary Conditions</b>"));
    feaLayout->addWidget(new QLabel("<i>Ctrl+Click = Fixed, Shift+Click = Force</i>"));

    feaLayout->addWidget(new QLabel("<b>Material Properties</b>"));
    QDoubleSpinBox *youngsBox = new QDoubleSpinBox();
    youngsBox->setRange(1.0, 500000.0); youngsBox->setValue(200000.0);
    youngsBox->setPrefix("E: "); youngsBox->setSuffix(" MPa");
    feaLayout->addWidget(youngsBox);

    QDoubleSpinBox *poissonBox = new QDoubleSpinBox();
    poissonBox->setRange(0.0, 0.49); poissonBox->setValue(0.3);
    poissonBox->setPrefix("v: "); poissonBox->setSingleStep(0.01);
    feaLayout->addWidget(poissonBox);

    QDoubleSpinBox *thickBox = new QDoubleSpinBox();
    thickBox->setRange(0.01, 100.0); thickBox->setValue(1.0);
    thickBox->setPrefix("t: "); thickBox->setSuffix(" mm");
    feaLayout->addWidget(thickBox);

    QDoubleSpinBox *densityBox = new QDoubleSpinBox();
    densityBox->setDecimals(10);
    densityBox->setRange(0.0000000001, 1.0);
    densityBox->setValue(0.00000000785);
    densityBox->setPrefix("ρ: "); densityBox->setSuffix(" t/mm³");
    feaLayout->addWidget(densityBox);

    feaLayout->addSpacing(10);

    feaLayout->addWidget(new QLabel("<b>Load Setup</b>"));
    QDoubleSpinBox *forceBox = new QDoubleSpinBox();
    forceBox->setRange(-1e9, 1e9); forceBox->setValue(1000.0);
    forceBox->setPrefix("Total Force: "); forceBox->setSuffix(" N");
    feaLayout->addWidget(forceBox);

    QComboBox *axisBox = new QComboBox();
    axisBox->addItems({"Pull along X-Axis", "Pull along Y-Axis", "Pull along Z-Axis"});
    feaLayout->addWidget(axisBox);

    feaLayout->addSpacing(10);

    feaLayout->addWidget(new QLabel("<b>Visualization</b>"));
    QDoubleSpinBox *scaleBox = new QDoubleSpinBox();
    scaleBox->setRange(1.0, 10000.0); scaleBox->setValue(1.0);
    scaleBox->setPrefix("Deform Scale: "); scaleBox->setSuffix("x");
    feaLayout->addWidget(scaleBox);

    feaLayout->addSpacing(10);

    feaLayout->addWidget(new QLabel("<b>Physics Engine</b>"));
    QCheckBox *chkGravity = new QCheckBox("Enable Gravity (-Y Axis)");
    feaLayout->addWidget(chkGravity);
    QCheckBox *chkNonLinear = new QCheckBox("Non-Linear Geometry (Iterative)");
    feaLayout->addWidget(chkNonLinear);

    //QCheckBox *chkShells = new QCheckBox("6-DOF Shell Elements (Bending)");
    //feaLayout->addWidget(chkShells);

    feaLayout->addSpacing(10);

    QPushButton *btnRunFEA = new QPushButton("Run FEA Simulation", this);
    feaLayout->addWidget(btnRunFEA);
    QPushButton *btnResetFEA = new QPushButton("Reset Mesh", this);
    feaLayout->addWidget(btnResetFEA);
    feaLayout->addStretch();

    //here is the optics tab for the ray tracing
    QWidget *opticsTab = new QWidget();
    QVBoxLayout *opticsLayout = new QVBoxLayout(opticsTab);

    opticsLayout->addWidget(new QLabel("<b>Beam Origin (X, Y, Z)</b>"));
    QHBoxLayout *originLayout = new QHBoxLayout();
    QDoubleSpinBox *oxBox = new QDoubleSpinBox(); oxBox->setRange(-100, 100); oxBox->setValue(0);
    QDoubleSpinBox *oyBox = new QDoubleSpinBox(); oyBox->setRange(-100, 100); oyBox->setValue(5.0);
    QDoubleSpinBox *ozBox = new QDoubleSpinBox(); ozBox->setRange(-100, 100); ozBox->setValue(0);
    originLayout->addWidget(oxBox); originLayout->addWidget(oyBox); originLayout->addWidget(ozBox);
    opticsLayout->addLayout(originLayout);

    //opticsLayout->addWidget(new QLabel("<b>Laser Direction (X, Y, Z)</b>"));
    //QHBoxLayout *dirLayout = new QHBoxLayout();
    //QDoubleSpinBox *dxBox = new QDoubleSpinBox(); dxBox->setRange(-1, 1); dxBox->setSingleStep(0.1); dxBox->setValue(0);
    //QDoubleSpinBox *dyBox = new QDoubleSpinBox(); dyBox->setRange(-1, 1); dyBox->setSingleStep(0.1); dyBox->setValue(-1.0);
    //QDoubleSpinBox *dzBox = new QDoubleSpinBox(); dzBox->setRange(-1, 1); dzBox->setSingleStep(0.1); dzBox->setValue(0.5);
    //dirLayout->addWidget(dxBox); dirLayout->addWidget(dyBox); dirLayout->addWidget(dzBox);
    //opticsLayout->addLayout(dirLayout);


    opticsLayout->addWidget(new QLabel("<b>Beam Direction (Angles)</b>"));
    QHBoxLayout *dirLayout = new QHBoxLayout();

    QDoubleSpinBox *yawBox = new QDoubleSpinBox();
    yawBox->setRange(-180.0, 180.0); yawBox->setValue(0.0);
    yawBox->setPrefix("Yaw: "); yawBox->setSuffix(" °");

    QDoubleSpinBox *pitchBox = new QDoubleSpinBox();
    pitchBox->setRange(-90.0, 90.0); pitchBox->setValue(-45.0); //point downward -45
    pitchBox->setPrefix("Pitch: "); pitchBox->setSuffix(" °");

    dirLayout->addWidget(yawBox); dirLayout->addWidget(pitchBox);
    opticsLayout->addLayout(dirLayout);



    opticsLayout->addSpacing(10);

    opticsLayout->addWidget(new QLabel("<b>Simulation Settings</b>"));
    QSpinBox *bouncesBox = new QSpinBox();
    bouncesBox->setRange(0, 100); bouncesBox->setValue(5);
    bouncesBox->setPrefix("Max Bounce Cutoff: ");
    opticsLayout->addWidget(bouncesBox);

    QSpinBox *raysBox = new QSpinBox();
    raysBox->setRange(1, 100000); raysBox->setValue(100);
    raysBox->setPrefix("Ray Count: ");
    opticsLayout->addWidget(raysBox);

    QDoubleSpinBox *spreadBox = new QDoubleSpinBox();
    spreadBox->setRange(0.0, 90.0); spreadBox->setValue(2.0);
    spreadBox->setPrefix("Beam Spread: "); spreadBox->setSuffix(" °");
    opticsLayout->addWidget(spreadBox);


    QDoubleSpinBox *energyBeam = new QDoubleSpinBox();
    energyBeam->setRange(0.0, 1000.0);
    energyBeam->setValue(1.0);
    energyBeam->setPrefix("Beam Energy: ");
    //spreadBox->setSuffix(" °");
    opticsLayout->addWidget(energyBeam);

    QDoubleSpinBox *reflectiveBox = new QDoubleSpinBox();
    reflectiveBox->setRange(0.0, 1.0);
    reflectiveBox->setValue(0.7);
    reflectiveBox->setSingleStep(0.1);   //set increment to 0.1
    reflectiveBox->setPrefix("Reflectivity: ");
    //spreadBox->setSuffix(" °");
    opticsLayout->addWidget(reflectiveBox);


    opticsLayout->addSpacing(10);

    QPushButton *btnShootLaser = new QPushButton("Run Particle Transport", this);
    opticsLayout->addWidget(btnShootLaser);
    QPushButton *btnClearLasers = new QPushButton("Clear Viewport", this);
    opticsLayout->addWidget(btnClearLasers);
    opticsLayout->addStretch();

    opticsLayout->addSpacing(10);

    //the tabs to the left dock
    simTabs->addTab(feaTab, "FEA");
    simTabs->addTab(opticsTab, "Optics");
    simDock->setWidget(simTabs);
    addDockWidget(Qt::LeftDockWidgetArea, simDock);

    //for FEA run button connect ******************** double check
    connect(btnRunFEA, &QPushButton::clicked, this, [this, youngsBox, poissonBox, thickBox, densityBox, forceBox, axisBox, scaleBox, chkGravity, chkNonLinear]() {
        // Clear lasers before running FEA
        m_renderer->clearRays();
        FEA_Analysis(youngsBox->value(), poissonBox->value(), thickBox->value(), densityBox->value(),
                     forceBox->value(), axisBox->currentIndex(), scaleBox->value(),
                     chkGravity->isChecked(), chkNonLinear->isChecked());
    });
    connect(btnResetFEA, &QPushButton::clicked, this, &MainWindow::on_actionUndo_triggered);

    //ray tracing optic shoot button
    connect(btnShootLaser, &QPushButton::clicked, this, [this, oxBox, oyBox, ozBox, yawBox, pitchBox, bouncesBox, raysBox, spreadBox, energyBeam, reflectiveBox]() {
        if (m_currentMesh.vertices.empty()) return;

        Eigen::Vector3d origin(oxBox->value(), oyBox->value(), ozBox->value());

        //pitch and yaw control for the beam that will be shot
        double yawRad = yawBox->value() * (M_PI / 180.0);
        double pitchRad = pitchBox->value() * (M_PI / 180.0);

        double dx = std::cos(pitchRad) * std::sin(yawRad);
        double dy = std::sin(pitchRad);
        double dz = std::cos(pitchRad) * std::cos(yawRad);
        double energy = energyBeam->value();
        double reflectivity = reflectiveBox->value();

        Eigen::Vector3d direction(dx, dy, dz);

        Ray centerLaser(origin, direction);
        RayTracerOptic tracer(m_currentMesh);

        std::vector<double> energies(m_currentMesh.vertices.size(), 0.0);

        std::vector<std::vector<Eigen::Vector3d>> allPaths = tracer.simulateBeamSwarm(
            centerLaser, raysBox->value(), spreadBox->value(), bouncesBox->value(), energies, energy, reflectivity
            );

        double maxEnergy = 1e-9;
        for (double e : energies) if (e > maxEnergy) maxEnergy = e;
        for (double& e : energies) e /= maxEnergy;

        m_renderer->setEnergies(energies);
        m_renderer->setRayPaths(allPaths);

        m_heatmapSourceCombo->setCurrentIndex(2);
        m_showHeatmap = true;
        m_heatmapType = 1;
        m_renderer->setHeatmapModes(true, m_heatmapType);
        updateRendererState();
    });

    connect(btnClearLasers, &QPushButton::clicked, this, [this]() {
        m_renderer->clearRays();
    });
    //########################################################################

    //thermal heat transfer button connection
    connect(btnRunThermal, &QPushButton::clicked, this, [this, conductivityBox, thermalThickBox, sourceTempBox, sinkTempBox]() {
        if (m_currentMesh.vertices.empty()) return;

        ThermalProps props = { conductivityBox->value(), thermalThickBox->value() };
        HeatTransferSolver solver(m_currentMesh, props);

        bool hasSource = false;
        bool hasSink = false;

        //applying boundary condition based on selected vertices
        const std::vector<Vertex>& renderVerts = m_renderer->getMesh().vertices;
        for (size_t i = 0; i < m_currentMesh.vertices.size(); ++i) {
            if (renderVerts[i].isAnchored) { //Ctrl+Click (Red)
                solver.addFixedTemperature(i, sourceTempBox->value());
                hasSource = true;
            }
            if (renderVerts[i].isSelected) { //Shift+Click (Magenta)
                solver.addFixedTemperature(i, sinkTempBox->value());
                hasSink = true;
            }
        }

        if (!hasSource && !hasSink) {
            QMessageBox::warning(this, "Missing Boundaries", "Please select at least one Heat Source (Ctrl+Click) or Heat Sink (Shift+Click).");
            return;
        }

        if (solver.solve()) {
            std::vector<double> temps = solver.getTemperatures();

            //normalize color map
            double minTemp = 99999.0;
            double maxTemp = -99999.0;
            for (double t : temps) {
                if (t < minTemp) minTemp = t;
                if (t > maxTemp) maxTemp = t;
            }

            std::vector<double> normalizedTemps(temps.size(), 0.0);
            double range = maxTemp - minTemp;
            if (range > 1e-6) {
                for (size_t i = 0; i < temps.size(); ++i) {
                    normalizedTemps[i] = (temps[i] - minTemp) / range;
                }
            }

            //m_renderer->setEnergies(normalizedTemps);

            m_renderer->setTemperatures(
                normalizedTemps);

            m_heatmapSourceCombo->setCurrentIndex(3);
            m_showHeatmap = true;
            m_heatmapType = 1;
            m_renderer->setHeatmapModes(true, m_heatmapType);
            updateRendererState();

            std::cout << "Thermal analysis complete. Max Temp: " << maxTemp << " C\n";
        } else {
            QMessageBox::warning(this, "Thermal Failed", "Matrix factorization failed.");
        }
    });

    //*************************************************************************


    //physics
    //thermalLayout->addWidget(new QLabel("<b>Heat Transfer Model</b>"));
    feaLayout->addWidget(new QLabel("<b>FEA Model</b>"));
    opticsLayout->addWidget(new QLabel("<b>Ray Tracing Model</b>"));

    thermalLayout->addWidget(new QLabel("<b>Heat Transfer Model</b>"));

 //   QCheckBox *chkGravity = new QCheckBox("Enable Gravity (-Y Axis)");
  //  chkGravity->setChecked(false);
  //  feaLayout->addWidget(chkGravity);

  //  QCheckBox *chkNonLinear = new QCheckBox("Non-Linear Geometry (Iterative)");
   // chkNonLinear->setChecked(false);
   // feaLayout->addWidget(chkNonLinear);

   // QCheckBox *chkShells = new QCheckBox("6-DOF Shell Elements (Bending)");
   // chkShells->setChecked(false);
   // feaLayout->addWidget(chkShells);

    feaLayout->addSpacing(10);
    opticsLayout->addSpacing(10);
    thermalLayout->addSpacing(10);


   // connect(btnRunFEA, &QPushButton::clicked, this, [this, youngsBox, poissonBox, thickBox, densityBox, forceBox, axisBox, scaleBox, chkGravity, chkNonLinear, chkShells]() {
    //    FEA_Analysis(youngsBox->value(), poissonBox->value(), thickBox->value(), densityBox->value(),
    //                            forceBox->value(), axisBox->currentIndex(), scaleBox->value(),
    //                            chkGravity->isChecked(), chkNonLinear->isChecked(), chkShells->isChecked());
   // });
   // connect(btnResetFEA, &QPushButton::clicked, this, &MainWindow::on_actionUndo_triggered);


}

MainWindow::~MainWindow() {
    delete ui;
}



void MainWindow::updateRendererState() {
    m_renderer->setDisplayModes(m_showFaces, m_showWireframe, m_showVertices);
    m_renderer->setShadingModes(m_enableLighting, m_smoothShading);
    m_renderer->setColors(m_bgColor, m_meshColor, m_wireColor);
}

void MainWindow::pushHistory() {
    m_history.push_back(m_currentMesh);
    ui->actionUndo->setEnabled(true);
}

//open diag for file opening, for txt and obj -- add dat later to this

void MainWindow::on_actionOpen_triggered() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Open Mesh File", "", "Mesh Files (*.txt *.obj);;All Files (*)");

    if (!fileName.isEmpty()) {
        if (m_currentMesh.loadFromFile(fileName.toStdString())) {

            //center camera and scale
            //m_currentMesh.normalize();

            m_history.clear(); //reset history upong new file upload, note it is important otherwise it creates a mess
            ui->actionUndo->setEnabled(false);

            m_renderer->setMesh(m_currentMesh);


            m_renderer->resetCameraToMesh();

            updateAnalysisPanel();
            setWindowTitle("SuperMeshPro - " + fileName);
        } else {
            QMessageBox::warning(this, "Error", "Failed to load the mesh file. Please check the format.");
        }
    }
}

//this saves subdivided cases in obj, aka the standard for blender import which is really useful
void MainWindow::on_actionSaveAs_triggered() {
    if (m_currentMesh.vertices.empty()) return;

    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Mesh", "", "OBJ Files (*.obj);;All Files (*)");

    if (!fileName.isEmpty()) {
        //here i append obj automatically to the user given name
        if (!fileName.endsWith(".obj", Qt::CaseInsensitive)) {
            fileName += ".obj";
        }

        std::ofstream out(fileName.toStdString());
        if (!out.is_open()) {
            QMessageBox::warning(this, "Error", "Could not write to file.");
            return;
        }

        out << "# Exported from SuperMeshPro\n";

        // Write vertices
        for (const auto& v : m_currentMesh.vertices) {
            out << "v " << v.x << " " << v.y << " " << v.z << "\n";
        }

        // Write faces
        for (const auto& f : m_currentMesh.faces) {
            out << "f ";
            //obj files use 1-based indexing
            for (const auto& ref : f.edgeRefs) {
                int vIdx = (ref.orientation == 0) ? m_currentMesh.edges[ref.edgeIndex].v1
                                                  : m_currentMesh.edges[ref.edgeIndex].v2;
                out << (vIdx + 1) << " ";
            }
            out << "\n";
        }

        out.close();
        QMessageBox::information(this, "Success", "Mesh successfully saved as OBJ.");
    }
}

void MainWindow::on_actionExit_triggered() {
    close();
}

//below is for undo based upon the history
void MainWindow::on_actionUndo_triggered() {
    if (!m_history.empty()) {
        m_currentMesh = m_history.back();
        m_history.pop_back();

        m_renderer->setMesh(m_currentMesh);

        if (m_history.empty()) {
            ui->actionUndo->setEnabled(false);
        }
    }
}

//catmull clark algo
void MainWindow::on_actionCatmull_Clark_triggered() {
    if (m_currentMesh.vertices.empty()) return;

    pushHistory();
    MeshTopology newMesh;
    MeshTopology limitMesh;

    SubdivisionAlgorithms::catmullClark(m_currentMesh, newMesh, limitMesh);

    m_currentMesh = newMesh;
    m_renderer->setMesh(m_currentMesh);
    updateAnalysisPanel();
}

//Doo-sabin algo
void MainWindow::on_actionDoo_Sabin_triggered() {
    if (m_currentMesh.vertices.empty()) return;

    pushHistory();
    MeshTopology newMesh;

    SubdivisionAlgorithms::dooSabin(m_currentMesh, newMesh);

    m_currentMesh = newMesh;
    m_renderer->setMesh(m_currentMesh);
    updateAnalysisPanel();
}

//loop algo
void MainWindow::on_actionLoop_triggered() {
    if (m_currentMesh.vertices.empty()) return;

    pushHistory();
    MeshTopology newMesh;
    MeshTopology limitMesh;

    SubdivisionAlgorithms::loop(m_currentMesh, newMesh, limitMesh);

    m_currentMesh = newMesh;
    m_renderer->setMesh(m_currentMesh);
    updateAnalysisPanel();
}

//viewing features
void MainWindow::on_actionToggle_Faces_triggered() {
    m_showFaces = !m_showFaces;
    updateRendererState();
}

void MainWindow::on_actionToggle_Wireframe_triggered() {
    m_showWireframe = !m_showWireframe;
    updateRendererState();
}

void MainWindow::on_actionToggle_Vertices_triggered() {
    m_showVertices = !m_showVertices;
    updateRendererState();
}

void MainWindow::on_actionToggle_Lighting_triggered() {
    m_enableLighting = !m_enableLighting;
    updateRendererState();
}

void MainWindow::on_actionToggle_Smooth_Shading_triggered() {
    m_smoothShading = !m_smoothShading;
    updateRendererState();
}

//colors
void MainWindow::on_actionMesh_Color_triggered() {
    QColor color = QColorDialog::getColor(m_meshColor, this, "Select Mesh Color");
    if (color.isValid()) {
        m_meshColor = color;
        updateRendererState();
    }
}

void MainWindow::on_actionBackground_Color_triggered() {
    QColor color = QColorDialog::getColor(m_bgColor, this, "Select Background Color");
    if (color.isValid()) {
        m_bgColor = color;
        updateRendererState();
    }
}

//about
void MainWindow::on_actionAbout_triggered() {
    QMessageBox::about(this, "About SuperMeshPro",
                       "<b>SuperMeshPro</b><br><br>"
                       "A mesh processing and numerical simulation tool.<br>"
                       "Built with Qt C++ and OpenGL.<br><br>"
                       "Developed by msb.");
}


//uisng polyhedron volume formula, much more accurate compared ot the rough estimation before
void MainWindow::updateAnalysisPanel() {
    if (m_currentMesh.vertices.empty()) return;

    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9, minZ = 1e9, maxZ = -1e9;
    double totalVolume = 0.0;
    double totalSurfaceArea = 0.0;

    //bounding box limit calculation
    for (const auto &v : m_currentMesh.vertices) {
        minX = std::min(minX, v.x); maxX = std::max(maxX, v.x);
        minY = std::min(minY, v.y); maxY = std::max(maxY, v.y);
        minZ = std::min(minZ, v.z); maxZ = std::max(maxZ, v.z);
    }

    //calc. exact vol and surf area
    for (size_t fIdx = 0; fIdx < m_currentMesh.faces.size(); ++fIdx) {

    //vertices for specific face
        std::vector<int> vList = SubdivisionAlgorithms::getFaceVertices(m_currentMesh, fIdx);

    if (vList.size() < 3)
            continue;

        const Vertex &v0 = m_currentMesh.vertices[vList[0]];

        //triangulate
        for (size_t i = 1; i + 1 < vList.size(); ++i) {
          const Vertex &v1 = m_currentMesh.vertices[vList[i]];

        const Vertex &v2 = m_currentMesh.vertices[vList[i + 1]];

            //surf area calc.
            //edge vecttor
            double d1x = v1.x - v0.x, d1y = v1.y - v0.y, d1z = v1.z - v0.z;
            double d2x = v2.x - v0.x, d2y = v2.y - v0.y, d2z = v2.z - v0.z;

            //cross product here for the matrix
            double cpx = d1y * d2z - d1z * d2y;
            double cpy = d1z * d2x - d1x * d2z;
            double cpz = d1x * d2y - d1y * d2x;

            //area of triangle multiply by 0.5
            totalSurfaceArea += 0.5 * std::sqrt(cpx*cpx + cpy*cpy + cpz*cpz);


            //exact vol calc. here
            //dot(v0, v1 x v2) / 6.0 scalar product
            double crossX = v1.y * v2.z - v1.z * v2.y;
            double crossY = v1.z * v2.x - v1.x * v2.z;
            double crossZ = v1.x * v2.y - v1.y * v2.x;

            totalVolume += (v0.x * crossX + v0.y * crossY + v0.z * crossZ) / 6.0;
        }
    }

    //inward facing normals that makes the volume negative
    totalVolume = std::abs(totalVolume);

    //topology metrics
    int V = m_currentMesh.vertices.size();
    int E = m_currentMesh.edges.size();
    int F = m_currentMesh.faces.size();

    //euler characteristic --- topological invariant
    int euler = V - E + F;  //check this *******************************

    //calculate genus, number of holes like a donut
    //assume the mesh is a single closed surface
    double genus = 1.0 - (euler / 2.0);  //check this *******************************

    //stats
    QString stats = QString(
                        "<b>Geometry</b><br>"
                        "Vertices: %1<br>"
                        "Edges: %2<br>"
                        "Faces: %3<br><br>"

                        "<b>Topology</b><br>"
                        "Euler Characteristic (χ): %4<br>"
                        "Estimated Genus: %5<br><br>"

                        "<b>Physical Dimensions (mm)</b><br>"
                        "W: %6 | H: %7 | D: %8<br><br>"

                        "<b>Properties</b><br>"
                        "Surface Area: %9 mm²<br>"
                        "Exact Volume: %10 mm³<br><br>"

                        "<b>Analysis</b><br>"
                        "Status: %11"
                        )
                        .arg(V)
                        .arg(E)
                        .arg(F)
                        .arg(euler)
                        .arg(genus >= 0 ? QString::number(genus) : "Non-manifold / Open Boundary")
                        .arg(maxX - minX, 0, 'f', 2)
                        .arg(maxY - minY, 0, 'f', 2)
                        .arg(maxZ - minZ, 0, 'f', 2)
                        .arg(totalSurfaceArea, 0, 'f', 2)
                        .arg(totalVolume, 0, 'f', 2)
                        .arg(m_showHeatmap ? "<font color='orange'>Heatmap Active</font>" : "Standard Shading");

    m_statsDisplay->setHtml(stats);
}


void MainWindow::FEA_Analysis(double E, double nu, double t, double density, double totalForce, int axis, double visualScale, bool useGravity, bool useNonLinear) {
    if (m_currentMesh.vertices.empty()) return;

    std::vector<int> pullNodes;
    std::vector<int> anchorNodes;

    const std::vector<Vertex>& renderVerts = m_renderer->getMesh().vertices;
    for (size_t i = 0; i < m_currentMesh.vertices.size(); ++i) {
        m_currentMesh.vertices[i].isSelected = renderVerts[i].isSelected;
        m_currentMesh.vertices[i].isAnchored = renderVerts[i].isAnchored;

        if (m_currentMesh.vertices[i].isSelected) pullNodes.push_back(i);
        if (m_currentMesh.vertices[i].isAnchored) anchorNodes.push_back(i);
    }

    if (anchorNodes.empty()) {
        QMessageBox::warning(this, "No BCs", "Please hold Ctrl + Left Click to set fixed boundary conditions (Red).");
        return;
    }
    if (pullNodes.empty() && !useGravity) {
        QMessageBox::warning(this, "No Loads", "Please hold Shift + Left Click to select vertices to pull (Magenta), or enable Gravity.");
        return;
    }

    pushHistory();

    MaterialProps mat = { E, nu, t, density };
    FEASolver solver(m_currentMesh, mat);

    double forcePerNode = pullNodes.empty() ? 0.0 : totalForce / pullNodes.size();

    //here force and boundary applied to vertices
    for (size_t i = 0; i < m_currentMesh.vertices.size(); ++i) {
        if (m_currentMesh.vertices[i].isAnchored) {
            //apply all 6 constraints (translations + rotations)
            solver.addConstraint(i, true, true, true, true, true, true);
        }

        if (m_currentMesh.vertices[i].isSelected) {
            double fx = (axis == 0) ? forcePerNode : 0.0;
            double fy = (axis == 1) ? forcePerNode : 0.0;
            double fz = (axis == 2) ? forcePerNode : 0.0;
            solver.addForce(i, fx, fy, fz, 0.0, 0.0, 0.0);
        }
    }


    if (solver.solve(useGravity, useNonLinear)) {

        solver.exportValidationDataCSV("supermeshpro_validation_data.csv");
        std::cout << "Validation data successfully saved.\n";

        //physical mesh deformation scaled by the UI visualScale slider/box
        Eigen::VectorXd disp = solver.getDisplacements();
        int dpn = 6; //6-DOF Shell

        for (size_t i = 0; i < m_currentMesh.vertices.size(); ++i) {
            m_currentMesh.vertices[i].x += disp(i * dpn) * visualScale;
            m_currentMesh.vertices[i].y += disp(i * dpn + 1) * visualScale;
            m_currentMesh.vertices[i].z += disp(i * dpn + 2) * visualScale;
        }

        m_renderer->setMesh(m_currentMesh);

        //stresses specifically scaled for the heatmap
        std::vector<double> stresses = solver.calculateVonMisesStresses();
        m_renderer->setStresses(stresses);

        m_heatmapSourceCombo->setCurrentIndex(1); //set combo box to stress

        m_showHeatmap = true;
        m_heatmapType = 0; //jet colormap
        updateRendererState();
        updateAnalysisPanel();

    } else {
        QMessageBox::warning(this, "FEA Failed", "The matrix solver failed. Check constraints.");
        on_actionUndo_triggered();
    }
}



void MainWindow::on_actionChange_Wire_Color_triggered()
{
    QColor color = QColorDialog::getColor(m_wireColor, this, "Select Wire Color");
    if (color.isValid()) {
        m_wireColor = color;
        updateRendererState();
    }
}

