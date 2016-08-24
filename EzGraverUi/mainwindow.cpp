#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QMimeData>
#include <QFileDialog>
#include <QBitmap>
#include <QIcon>
#include <QThreadPool>

#include <stdexcept>
#include <functional>

MainWindow::MainWindow(QWidget *parent)
        :  QMainWindow{parent}, _ui{new Ui::MainWindow},
          _portTimer{}, _ezGraver{}, _connected{false}, _imageLoaded{false} {
    _ui->setupUi(this);
    setAcceptDrops(true);

    connect(&_portTimer, &QTimer::timeout, this, &MainWindow::updatePorts);
    _portTimer.start(1000);

    setupBindings();
    setConnected(false);
}

MainWindow::~MainWindow() {
}

void MainWindow::setupBindings() {
    connect(_ui->burnTime, &QSlider::valueChanged, [this](const int& v) { _ui->burnTimeLabel->setText(QString::number(v)); });

    connect(this, &MainWindow::connectedChanged, _ui->ports, &QComboBox::setDisabled);
    connect(this, &MainWindow::connectedChanged, _ui->connect, &QPushButton::setDisabled);
    connect(this, &MainWindow::connectedChanged, _ui->disconnect, &QPushButton::setEnabled);

    connect(this, &MainWindow::connectedChanged, _ui->home, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->up, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->left, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->center, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->right, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->down, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->preview, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->start, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->pause, &QPushButton::setEnabled);
    connect(this, &MainWindow::connectedChanged, _ui->reset, &QPushButton::setEnabled);

    auto uploadEnabled{[this]() { _ui->upload->setEnabled(_imageLoaded && _connected); }};
    connect(this, &MainWindow::connectedChanged, uploadEnabled);
    connect(this, &MainWindow::imageLoadedChanged, uploadEnabled);
}

void MainWindow::printVerbose(QString const& verbose) {
    _ui->verbose->appendPlainText(verbose);
}

void MainWindow::updatePorts() {
    QStringList ports{EzGraver::availablePorts()};
    ports.insert(0, "");

    QString original{_ui->ports->currentText()};
    _ui->ports->clear();
    _ui->ports->addItems(ports);

    if(ports.contains(original)) {
        _ui->ports->setCurrentText(original);
    }
}

void MainWindow::loadImage(QString const& fileName) {
    printVerbose(QString("loading image: %1").arg(fileName));

    QImage image{};
    if(!image.load(fileName)) {
        printVerbose("failed to load image");
        return;
    }
    image = image.scaled(512, 512).convertToFormat(QImage::Format_Mono);

    _ui->image->setPixmap(QPixmap::fromImage(image));
    setImageLoaded(true);
}

bool MainWindow::connected() const {
    return _connected;
}

void MainWindow::setConnected(bool connected) {
    _connected = connected;
    emit connectedChanged(connected);
}

bool MainWindow::imageLoaded() const {
    return _imageLoaded;
}

void MainWindow::setImageLoaded(bool imageLoaded) {
    _imageLoaded = imageLoaded;
    emit imageLoadedChanged(imageLoaded);
}

void MainWindow::on_connect_clicked() {
    try {
        printVerbose(QString("connecting to port %1").arg(_ui->ports->currentText()));
        _ezGraver = EzGraver::create(_ui->ports->currentText());
        printVerbose("connection established successfully");
        setConnected(true);
    } catch(std::runtime_error const& e) {
        printVerbose(QString("Error: %1").arg(e.what()));
    }
}

void MainWindow::on_home_clicked() {
    printVerbose("moving to home");
    _ezGraver->home();
}

void MainWindow::on_up_clicked() {
    _ezGraver->up();
}

void MainWindow::on_left_clicked() {
    _ezGraver->left();
}

void MainWindow::on_center_clicked() {
    printVerbose("moving to center");
    _ezGraver->center();
}

void MainWindow::on_right_clicked() {
    _ezGraver->right();
}

void MainWindow::on_down_clicked() {
    _ezGraver->down();
}

void MainWindow::on_upload_clicked() {
    QImage image{_ui->image->pixmap()->toImage()};

    printVerbose("erasing EEPROM");
    _ezGraver->erase();

    QTimer::singleShot(6000, [this, image]() {
        printVerbose("uploading image to EEPROM");
        _ezGraver->uploadImage(image);
        printVerbose("upload completed");
    });
}

void MainWindow::on_preview_clicked() {
    printVerbose("drawing preview");
    _ezGraver->preview();
}

void MainWindow::on_start_clicked() {
    printVerbose(QString("starting engrave process with burn time %1").arg(_ui->burnTime->value()));
    _ezGraver->start(_ui->burnTime->value());
}

void MainWindow::on_pause_clicked() {
    printVerbose("pausing engrave process");
    _ezGraver->pause();
}

void MainWindow::on_reset_clicked() {
    printVerbose("resetting engraver");
    _ezGraver->reset();
}

void MainWindow::on_disconnect_clicked() {
    printVerbose("disconnecting");
    setConnected(false);
    _ezGraver.reset();
    printVerbose("disconnected");
}

void MainWindow::on_image_clicked() {
    auto fileName = QFileDialog::getOpenFileName(this, "Open Image", "", "Image Files (*.png *.jpeg *.jpg *.bmp)");
    loadImage(fileName);
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if(event->mimeData()->hasUrls() && event->mimeData()->urls().count() == 1) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    QString fileName{event->mimeData()->urls()[0].toLocalFile()};
    loadImage(fileName);
}

