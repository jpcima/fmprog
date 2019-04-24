#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "fmprog.h"
#include "utility/music.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);

    setupPitchValues();
    setupClockValues();

    Application &app = static_cast<Application &>(*qApp);
    connect(&app, &Application::midiPitchChanged, this, &MainWindow::updateMidiPitch);
    connect(&app, &Application::fmChipClockChanged, this, &MainWindow::updateFmChipClock);
}

MainWindow::~MainWindow()
{
}

InstrumentEditor *MainWindow::instrumentEditor() const
{
    return ui_->instrumentEditor;
}

void MainWindow::updateGenerationNumber(size_t gen_num)
{
    ui_->genNumLabel->setText(QString::number(gen_num));
}

void MainWindow::updateMidiPitch(unsigned key)
{
    ui_->pitchComboBox->setCurrentIndex(ui_->pitchComboBox->findData(key));
}

void MainWindow::updateFmChipClock(unsigned clock)
{
    ui_->fmClockComboBox->setCurrentIndex(ui_->fmClockComboBox->findData(clock));
}

void MainWindow::loadAudioFile(const QString &filename)
{
    Application &app = static_cast<Application &>(*qApp);
    if (app.loadAudioFile(filename)) {
        ui_->fileNameEdit->setText(QFileInfo(filename).fileName());
        ui_->fileNameEdit->setCursorPosition(0);
    }
}

bool MainWindow::saveInstrumentFile()
{
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Save instrument"), QString(), tr("Instruments (*.opni)"));
    if (filename.isEmpty())
        return false;

    Application &app = static_cast<Application &>(*qApp);
    if (!app.saveFittestInstrument(filename)) {
        QMessageBox::warning(this, tr("Error"), tr("Could not save the instrument."));
        return false;
    }

    return true;
}

void MainWindow::setupPitchValues()
{
    for (unsigned note = 0; note < 128; ++note)
        ui_->pitchComboBox->addItem(QString::fromStdString(midi_note_to_string(note)), note);
}

void MainWindow::setupClockValues()
{
    unsigned clockOPN2 = 7670454;
    unsigned clockOPNA = 7987200;
    ui_->fmClockComboBox->addItem(QString("OPN2: %1 MHz").arg(clockOPN2 * 1e-6), clockOPN2);
    ui_->fmClockComboBox->addItem(QString("OPNA: %1 MHz").arg(clockOPNA * 1e-6), clockOPNA);
}

void MainWindow::on_loadButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Load audio"), QString(), tr("Audio files (*)"));
    if (filename.isEmpty())
        return;

    loadAudioFile(filename);
}

void MainWindow::on_saveButton_clicked()
{
    saveInstrumentFile();
}

void MainWindow::on_playButton_clicked()
{
    Application &app = static_cast<Application &>(*qApp);
    app.playFittestInstrument();
}

void MainWindow::on_startButton_clicked()
{
    Application &app = static_cast<Application &>(*qApp);
    app.startAi();
}

void MainWindow::on_pauseButton_clicked(bool checked)
{
    Application &app = static_cast<Application &>(*qApp);
    app.setPausedAi(checked);
}

void MainWindow::on_pitchComboBox_currentIndexChanged(int index)
{
    Application &app = static_cast<Application &>(*qApp);
    app.setMidiPitch(ui_->pitchComboBox->itemData(index).toUInt());
}

void MainWindow::on_fmClockComboBox_currentIndexChanged(int index)
{
    Application &app = static_cast<Application &>(*qApp);
    app.setFmChipClock(ui_->fmClockComboBox->itemData(index).toUInt());
}
