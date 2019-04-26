#pragma once
#include "ai/ai.h"
#include "utility/aubio++.h"
#include <QApplication>
#include <cstdint>

namespace ai { class GeneticAlgorithm; }
class MainWindow;
class QAudioOutput;

class Application : public QApplication {
    Q_OBJECT

public:
    using QApplication::QApplication;
    ~Application();
    void init();

    bool loadAudioFile(const QString &filename);
    bool saveFittestInstrument(const QString &filename);
    void playFittestInstrument();
    void setFmChipClock(unsigned clock);
    void setMidiPitch(unsigned key);

    void startAi();
    void setPausedAi(bool paused);
    void togglePausedAi();
    void resetAi();

    void playReferenceAudio();

    unsigned fmChipClock() const noexcept { return fmChipClock_; }
    double fmSampleRate() const noexcept { return fmChipClock_ / 144.0; }

signals:
    void midiPitchChanged(unsigned key);
    void fmChipClockChanged(unsigned clock);

private:
    void playAudio(const fvec_t &sound, double sample_rate);
    void resampleSound();
    void detectPitch();
    void onGenerationFromOtherThread(size_t generation_num, const ai::Individual &fittest);
    void onFitnessFromOtherThread(size_t generation_num, const double *fitness, size_t count);

private slots:
    void onGeneration(ulong generation_num, const ai::Individual fittest);
    void onFitness(ulong generation_num, QVector<qreal> fitness);

private:
    MainWindow *window_ = nullptr;
    unsigned fmChipClock_ = 7670454;

    fvec_u sndOriginal_;
    unsigned sampleRateOriginal_ = 44100;
    unsigned sndMidiPitch_ = 69;

    std::unique_ptr<ai::GeneticAlgorithm> ga_;
    ai::Individual currentFittest_;

    QAudioOutput *audioOut_ = nullptr;
    QByteArray audioOutData_;
};
