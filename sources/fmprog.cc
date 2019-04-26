#include "fmprog.h"
#include "mainwindow.h"
#include "instrumenteditor.h"
#include "file-formats/format_wohlstand_opn2.h"
#include "ai/algorithm.h"
#include "ai/evaluation.h"
#include "ai/ai.h"
#include "ai/qtmeta.h"
#include "utility/music.h"
#include <QMessageBox>
#include <QCommandLineParser>
#include <QMetaObject>
#include <QBuffer>
#include <QSysInfo>
#include <QAudioOutput>
#include <QDebug>
#include <cmath>

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    app.init();
    return app.exec();
}

Application::~Application()
{
    ai::GeneticAlgorithm &ga = *ga_;
    ga.stop();
}

void Application::init()
{
    setApplicationName("FMProg");

    QCommandLineParser cli;
    cli.addHelpOption();
    cli.addPositionalArgument("audio-file", tr("Reference audio file"));
    cli.process(*this);

    QStringList optargs = cli.positionalArguments();
    QString audiofile;
    if (optargs.size() > 0)
        audiofile = optargs[0];

    ai::registerQtMetaTypes();

    ai::GeneticAlgorithm *ga = new ai::GeneticAlgorithm;
    ga_.reset(ga);
    ga->set_generation_callback([this](size_t g, const ai::Individual &ind) {
                                    onGenerationFromOtherThread(g, ind);
                                });
    ga->set_fitness_callback([this](size_t g, const double *f, size_t n) {
                                    onFitnessFromOtherThread(g, f, n);
                                });

    MainWindow *window = window_ = new MainWindow;
    window->setWindowTitle(applicationDisplayName());
    window->show();

    if (!audiofile.isEmpty())
        window->loadAudioFile(audiofile);

    {
        std::unique_lock<std::mutex> lock;
        ai::GeneticData &gd = ga->lock(lock);
        ai::Evaluation &eval = *gd.eval_;
        eval.set_reference_note(sndMidiPitch_);
        eval.set_sample_rate(fmSampleRate());
        gd.population_->clear_evaluation();
    }

    emit midiPitchChanged(sndMidiPitch_);
    emit fmChipClockChanged(fmChipClock());
}

bool Application::loadAudioFile(const QString &filename)
{
    double file_sample_rate = 0;
    fvec_u file_sound = load_sound_file(filename.toLocal8Bit().data(), &file_sample_rate);

    sndOriginal_ = std::move(file_sound);
    sampleRateOriginal_ = file_sample_rate;

    resampleSound();
    detectPitch();

    return true;
}

bool Application::saveFittestInstrument(const QString &filename)
{
    FmBank::Instrument ins = currentFittest_.ins_;

    if (WohlstandOPN2().saveFileInst(filename, ins) != FfmtErrCode::ERR_OK)
        return false;

    return true;
}

void Application::playFittestInstrument()
{
    FmBank::Instrument ins = currentFittest_.ins_;
    fvec_u sound;
    double sample_rate;

    // generate
    {
        ai::GeneticAlgorithm &ga = *ga_;
        bool was_paused = ga.set_paused(true);
        std::unique_lock<std::mutex> lock;
        ai::GeneticData &gd = ga.lock(lock);
        ai::Evaluation &eval = *gd.eval_;
        sample_rate = eval.sample_rate();
        sound = ai::Evaluation::generate(ins, eval.reference().length, sample_rate, eval.reference_note());
        ga.set_paused(was_paused);
    }

    playAudio(*sound, sample_rate);
}

void Application::setFmChipClock(unsigned clock)
{
    if (fmChipClock_ == clock)
        return;

    fmChipClock_ = clock;
    resampleSound();
    emit fmChipClockChanged(clock);
}

void Application::setMidiPitch(unsigned key)
{
    if (sndMidiPitch_ == key)
        return;

    sndMidiPitch_ = key;

    {
        ai::GeneticAlgorithm &ga = *ga_;
        bool was_paused = ga.set_paused(true);
        std::unique_lock<std::mutex> lock;
        ai::GeneticData &gd = ga.lock(lock);
        gd.eval_->set_reference_note(key);
        gd.population_->clear_evaluation();
        ga.set_paused(was_paused);
    }

    emit midiPitchChanged(key);
}

void Application::startAi()
{
    ai::GeneticAlgorithm &ga = *ga_;
    ga.start();
}

void Application::setPausedAi(bool paused)
{
    ai::GeneticAlgorithm &ga = *ga_;
    ga.set_paused(paused);
}

void Application::togglePausedAi()
{
    ai::GeneticAlgorithm &ga = *ga_;
    ga.toggle_paused();
}

void Application::resetAi()
{
    ai::GeneticAlgorithm &ga = *ga_;
    ga.reinitialize();
}

void Application::playReferenceAudio()
{
    const fvec_t *snd = sndOriginal_.get();
    if (snd)
        playAudio(*snd, sampleRateOriginal_);
}

void Application::playAudio(const fvec_t &sound, double sample_rate)
{
    // stop current
    QAudioOutput *audioOut = audioOut_;
    if (audioOut) {
        audioOut->stop();
        audioOut->deleteLater();
        audioOut_ = nullptr;
    }

    // set up audio
    QAudioFormat format;
    format.setByteOrder(QAudioFormat::Endian(QSysInfo::ByteOrder));
    format.setChannelCount(2);
    format.setCodec("audio/pcm");
    format.setSampleRate(sample_rate);
    format.setSampleSize(16);
    format.setSampleType(QAudioFormat::SignedInt);

    audioOut = new QAudioOutput(format, this);
    audioOut_ = audioOut;

    QByteArray &audioOutData = audioOutData_;
    audioOutData.clear();
    audioOutData.reserve(2 * sizeof(int16_t) * sound.length);
    for (unsigned i = 0, n = sound.length; i < n; ++i) {
        long sample = std::lround(sound.data[i] * 32768.0);
        sample = std::min(sample, 32767l);
        sample = std::max(sample, -32768l);
        int16_t sample16 = (int16_t)sample;
        audioOutData.append((const char *)&sample16, sizeof(int16_t)); // left
        audioOutData.append((const char *)&sample16, sizeof(int16_t)); // right
    }

    // play
    QBuffer *buffer = new QBuffer(&audioOutData, audioOut);
    buffer->open(QBuffer::ReadOnly);
    audioOut->start(buffer);
}

void Application::resampleSound()
{
    fvec_t *src = sndOriginal_.get();

    double src_rate = sampleRateOriginal_;
    double dst_rate = fmSampleRate();

    fvec_u dst(resample_sound(src, src_rate, dst_rate));

    {
        ai::GeneticAlgorithm &ga = *ga_;
        bool was_paused = ga.set_paused(true);
        std::unique_lock<std::mutex> lock;
        ai::GeneticData &gd = ga.lock(lock);
        gd.eval_->set_reference(std::move(dst));
        gd.population_->clear_evaluation();
        ga.set_paused(was_paused);
    }
}

void Application::detectPitch()
{
    unsigned key = detect_sound_pitch(sndOriginal_.get(), sampleRateOriginal_);
    setMidiPitch(key);
}

void Application::onGenerationFromOtherThread(size_t generation_num, const ai::Individual &fittest)
{
    QMetaObject::invokeMethod(this, "onGeneration", Qt::QueuedConnection,
                              Q_ARG(ulong, generation_num), Q_ARG(ai::Individual, fittest));
}

void Application::onFitnessFromOtherThread(size_t generation_num, const double *fitness, size_t count)
{
    QVector<qreal> fitness_vector;
    fitness_vector.reserve(count);
    std::copy(fitness, fitness + count, std::back_inserter(fitness_vector));

    QMetaObject::invokeMethod(this, "onFitness", Qt::QueuedConnection,
                              Q_ARG(ulong, generation_num), Q_ARG(QVector<qreal>, std::move(fitness_vector)));
}

void Application::onGeneration(ulong generation_num, const ai::Individual fittest)
{
    window_->updateGenerationNumber(generation_num);
    currentFittest_ = fittest;
    window_->instrumentEditor()->setValuesFromInstrument(fittest.ins_);
}

void Application::onFitness(ulong generation_num, QVector<qreal> fitness)
{
    #pragma message("TODO implement fitness display")
    //qDebug() << "Fitness";
}
