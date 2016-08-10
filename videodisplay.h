#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

/*  描述：本地视频显示、以及提供传输给对端的源的接口
*/
//必须加以下内容,否则编译不能通过,为了兼容C和C99标准
#ifndef INT64_C
#define INT64_C
#define UINT64_C
#endif

//引入ffmpeg头文件
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include <libavutil/frame.h>
#include <x264.h>
#include <x264_config.h>
}

#include <QObject>
#include <QCamera>
#include <QImage>
#include <QWidget>
#include <QDate>
#include <QPoint>
#include <QMutex>
#include <QMainWindow>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QCameraViewfinderSettings>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QLabel>
#include <QVideoWidget>
#include <QMessageBox>
#include <QTimer>
#include <QThread>

#include "msginfo.h"
/*
 *  由于发送和接受两个线程是阻塞的，所以连接之后信号槽不起作用，虽自定义资源释放
 *
 *
*/
///////////////////////////////////////////////////////////////////////
/// MyVideo_Send_Thread
///////////////////////////////////////////////////////////////////////////

class VideoDisplay_Send;
class VideoDisplay_Recv;

class MyVideo_Send_Thread : public QThread
{
    Q_OBJECT
public:
    explicit MyVideo_Send_Thread(VideoDisplay_Send* send, QObject* parent=0);

    ~MyVideo_Send_Thread();
protected:
    void run();


protected slots:
    void slot_finished();

    void slot_new_connection();



private:
    QTcpSocket         * m_pSocket;
    QTcpServer         * m_pServer;

    VideoDisplay_Send  * m_pVideoSend;

};



///////////////////////////////////////////////////////////////////////////////
/// VideoDisplay_Send
/// ///////////////////////////////////////////////////////////////////////////

class VideoDisplay_Send : public QObject
{
    Q_OBJECT
public:
    explicit VideoDisplay_Send(QVideoWidget* pwin, QObject *parent = 0);
    ~VideoDisplay_Send();
    /* 传视频流(包括打开摄像头) */
    void Start(QTcpSocket* socket);
    /* 开关摄像头 */
    void OpenCamrea();
    void CloseCamera();

private:
    /* 初始化摄像头 */
    void __Init_Camera();

signals:
    void signal_peer_close();

public:


private slots:
    /* 获取一张图片 */
    void slot_capture_image(int, QImage image);

    /* 超时截图 */
    void slot_capture_image();

private:
    QVideoWidget            * m_pWin;

    QCamera                 * m_pCamera;

    QCameraImageCapture     * m_pImageCapture;

    QTcpSocket              * m_pSocket;

    QTimer                  * m_pTimer;

};

////////////////////////////////////////////////////////////////////////////
/// MyVideo_Recv_Thread
//////////////////////////////////////////////////////////////////////////////
class MyVideo_Recv_Thread : public QThread
{
    Q_OBJECT
public:
    explicit MyVideo_Recv_Thread(VideoDisplay_Recv*recv, QObject* parent=0);

    ~MyVideo_Recv_Thread();

protected:
    void run();

protected slots:
    void slot_finished();

private:

    VideoDisplay_Recv  * m_pVideoRecv;
};

///////////////////////////////////////////////////////////////////////////////
/// VideoDisplay_Recv
/// ///////////////////////////////////////////////////////////////////////////

class VideoDisplay_Recv : public QObject
{
    Q_OBJECT
public:
    VideoDisplay_Recv(const QHostAddress& addr, QObject*parent = nullptr);
    ~VideoDisplay_Recv();

signals:
    void signal_get_image(const QImage& image);

private:
    /* 初始化ffmpeg */
    void __Init();
public:

    /* 获取裸流并进行图片解析，然后将解析出来的图片通过信号发给主线程进行显示 */
    void Play();

private:


    QHostAddress          m_addr;

private:
    QMutex mutex;

    AVPicture  pAVPicture;
    AVFormatContext *pAVFormatContext;
    AVCodecContext *pAVCodecContext;
    AVFrame *pAVFrame;
    SwsContext * pSwsContext;
    AVPacket pAVPacket;

    QString url;
    int videoWidth;
    int videoHeight;
    int videoStreamIndex;

    bool isplaying;

    QImage m_image;

};

#endif // VIDEODISPLAY_H
