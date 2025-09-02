#ifndef VIDEOPLAYERDIALOG_H
#define VIDEOPLAYERDIALOG_H

#include <QDialog>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QKeyEvent>
#include <QMouseEvent>

class VideoPlayerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VideoPlayerDialog(const QString &videoPath, QWidget *parent = nullptr);
    ~VideoPlayerDialog();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private slots:
    void play();
    void pause();
    void stop();
    void toggleFullscreen();
    void updatePosition(qint64 position);
    void setPosition(int position);
    void updateDuration(qint64 duration);
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void handleError(QMediaPlayer::Error error, const QString &errorString);

private:
    void setupUI();
    void createControls();

    QMediaPlayer *m_mediaPlayer;
    QVideoWidget *m_videoWidget;
    QPushButton *m_playButton;
    QPushButton *m_pauseButton;
    QPushButton *m_stopButton;
    QPushButton *m_fullscreenButton;
    QSlider *m_positionSlider;
    QSlider *m_volumeSlider;
    QLabel *m_positionLabel;
    QLabel *m_durationLabel;
    QString m_videoPath;
};

#endif // VIDEOPLAYERDIALOG_H
