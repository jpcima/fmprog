#pragma once
#include <QMainWindow>
#include <memory>

namespace Ui { class MainWindow; }
class InstrumentEditor;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    InstrumentEditor *instrumentEditor() const;

    void updateGenerationNumber(size_t gen_num);

public slots:
    void updateMidiPitch(unsigned key);
    void updateFmChipClock(unsigned clock);
    void loadAudioFile(const QString &filename);
    bool saveInstrumentFile();

private:
    void setupPitchValues();
    void setupClockValues();

private slots:
    void on_loadButton_clicked();
    void on_saveButton_clicked();
    void on_playButton_clicked();
    void on_playReferenceButton_clicked();
    void on_startButton_clicked();
    void on_pauseButton_clicked(bool checked);
    void on_pitchComboBox_currentIndexChanged(int index);
    void on_fmClockComboBox_currentIndexChanged(int index);

private:
    std::unique_ptr<Ui::MainWindow> ui_;
};
