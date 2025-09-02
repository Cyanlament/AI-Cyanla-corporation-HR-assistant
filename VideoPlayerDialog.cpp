#include "VideoPlayerDialog.h"
#include <QStyle>
#include <QTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QAudioOutput>
#include <QKeyEvent>
#include <QShortcut>

VideoPlayerDialog::VideoPlayerDialog(const QString &videoPath, QWidget *parent)
    : QDialog(parent), m_videoPath(videoPath)
{
    setWindowTitle("视频播放器");
    setMinimumSize(800, 600);

    setupUI();

    // 设置视频路径
    if (!m_videoPath.isEmpty() && QFileInfo(m_videoPath).exists()) {
        m_mediaPlayer->setSource(QUrl::fromLocalFile(m_videoPath));
    } else {
        QMessageBox::warning(this, "文件错误", "视频文件不存在或路径无效");
    }

    // 添加ESC键退出全屏的快捷键
    QShortcut *escShortcut = new QShortcut(QKeySequence(Qt::Key_Escape), this);
    connect(escShortcut, &QShortcut::activated, this, [this]() {
        if (m_videoWidget->isFullScreen()) {
            toggleFullscreen();
        }
    });
}

VideoPlayerDialog::~VideoPlayerDialog()
{
    m_mediaPlayer->stop();
}

void VideoPlayerDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // 创建媒体播放器
    m_mediaPlayer = new QMediaPlayer(this);
    m_mediaPlayer->setAudioOutput(new QAudioOutput(this));

    // 创建视频显示部件
    m_videoWidget = new QVideoWidget(this);
    m_mediaPlayer->setVideoOutput(m_videoWidget);
    mainLayout->addWidget(m_videoWidget, 1);

    // 创建控制部件
    createControls();

    // 连接信号槽
    connect(m_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayerDialog::updatePosition);
    connect(m_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayerDialog::updateDuration);
    connect(m_mediaPlayer, &QMediaPlayer::mediaStatusChanged, this, &VideoPlayerDialog::handleMediaStatusChanged);
    connect(m_mediaPlayer, &QMediaPlayer::errorOccurred, this, &VideoPlayerDialog::handleError);
}

void VideoPlayerDialog::createControls()
{
    QHBoxLayout *controlLayout = new QHBoxLayout;

    // 播放按钮
    m_playButton = new QPushButton("播放", this);
    m_playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_playButton, &QPushButton::clicked, this, &VideoPlayerDialog::play);

    // 暂停按钮
    m_pauseButton = new QPushButton("暂停", this);
    m_pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    connect(m_pauseButton, &QPushButton::clicked, this, &VideoPlayerDialog::pause);

    // 停止按钮
    m_stopButton = new QPushButton("停止", this);
    m_stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    connect(m_stopButton, &QPushButton::clicked, this, &VideoPlayerDialog::stop);

    // 位置滑块
    m_positionSlider = new QSlider(Qt::Horizontal, this);
    m_positionSlider->setRange(0, 0);
    connect(m_positionSlider, &QSlider::sliderMoved, this, &VideoPlayerDialog::setPosition);

    // 位置标签
    m_positionLabel = new QLabel("00:00:00", this);
    m_durationLabel = new QLabel("/00:00:00", this);

    // 音量滑块
    m_volumeSlider = new QSlider(Qt::Horizontal, this);
    m_volumeSlider->setRange(0, 100);
    m_volumeSlider->setValue(80);
    connect(m_volumeSlider, &QSlider::valueChanged, [this](int value) {
        m_mediaPlayer->audioOutput()->setVolume(value / 100.0);
    });

    // 全屏按钮
    m_fullscreenButton = new QPushButton("全屏", this);
    connect(m_fullscreenButton, &QPushButton::clicked, this, &VideoPlayerDialog::toggleFullscreen);

    // 添加到布局
    controlLayout->addWidget(m_playButton);
    controlLayout->addWidget(m_pauseButton);
    controlLayout->addWidget(m_stopButton);
    controlLayout->addWidget(m_positionLabel);
    controlLayout->addWidget(m_positionSlider);
    controlLayout->addWidget(m_durationLabel);
    controlLayout->addWidget(new QLabel("音量:", this));
    controlLayout->addWidget(m_volumeSlider);
    controlLayout->addWidget(m_fullscreenButton);

    // 添加到主布局
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(layout());
    mainLayout->addLayout(controlLayout);
}

void VideoPlayerDialog::play()
{
    m_mediaPlayer->play();
}

void VideoPlayerDialog::pause()
{
    m_mediaPlayer->pause();
}

void VideoPlayerDialog::stop()
{
    m_mediaPlayer->stop();
}

void VideoPlayerDialog::toggleFullscreen()
{
    if (m_videoWidget->isFullScreen()) {
        m_videoWidget->setFullScreen(false);
        m_fullscreenButton->setText("全屏");
        show(); // 确保对话框显示
    } else {
        m_videoWidget->setFullScreen(true);
        m_fullscreenButton->setText("退出全屏");
    }
}

void VideoPlayerDialog::updatePosition(qint64 position)
{
    m_positionSlider->setValue(position);

    QTime time(0, 0, 0);
    time = time.addMSecs(position);
    m_positionLabel->setText(time.toString("hh:mm:ss"));
}

void VideoPlayerDialog::setPosition(int position)
{
    m_mediaPlayer->setPosition(position);
}

void VideoPlayerDialog::updateDuration(qint64 duration)
{
    m_positionSlider->setRange(0, duration);

    QTime time(0, 0, 0);
    time = time.addMSecs(duration);
    m_durationLabel->setText("/" + time.toString("hh:mm:ss"));
}

void VideoPlayerDialog::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    switch (status) {
    case QMediaPlayer::LoadedMedia:
        m_playButton->setEnabled(true);
        break;
    case QMediaPlayer::EndOfMedia:
        m_mediaPlayer->setPosition(0);
        m_playButton->setEnabled(true);
        break;
    default:
        break;
    }
}

void VideoPlayerDialog::handleError(QMediaPlayer::Error error, const QString &errorString)
{
    QMessageBox::warning(this, "播放错误", "无法播放视频: " + errorString);
}

// 添加键盘事件处理
void VideoPlayerDialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape && m_videoWidget->isFullScreen()) {
        toggleFullscreen();
        event->accept();
    } else {
        QDialog::keyPressEvent(event);
    }
}

// 添加鼠标双击事件处理
void VideoPlayerDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        toggleFullscreen();
        event->accept();
    } else {
        QDialog::mouseDoubleClickEvent(event);
    }
}
