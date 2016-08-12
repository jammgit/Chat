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

const int width = 640;
const int height = 480;

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

    void close_socket()
    {
        m_mutex.lock();
        if (m_pSocket)
        {
            m_pSocket->deleteLater();
            m_pSocket = nullptr;
        }
        m_mutex.unlock();
    }
protected:
    void run();


protected slots:
    void slot_finished();

    void slot_new_connection();



private:
    QTcpSocket         * m_pSocket;
    QTcpServer         * m_pServer;
    QMutex               m_mutex;
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

public slots:
    void slot_stop_timer();

private slots:
    /* 获取一张图片 */
    void slot_capture_image(int, QImage image);

    /* 超时截图 */
    void slot_capture_image();
private:
private:
    /* 显示控件，及传输接口 */
    QVideoWidget            * m_pWin;

    QCamera                 * m_pCamera;

    QCameraImageCapture     * m_pImageCapture;

    QTcpSocket              * m_pSocket;

    QTimer                  * m_pTimer;
private:
    /* FFmpeg图片格式转换变量 */
//    AVFrame                 * m_rgb_pframe;
//    int                       m_rgb_bytes;
//    uint8_t                 * m_rgb_buffer;

//    AVFrame                 * m_yuv_pframe;
//    int                       m_yuv_bytes;
//    uint8_t                 * m_yuv_buffer;

//    SwsContext              * m_rgb2yuv_ctx;
private:
    /* X264实现变量,没次断开连接时就释放，再初始化 */
    bool                      m_is_init_x264;

    int                       m_fps;
    int                       m_i_pts;
    x264_picture_t            m_pic_in;
    x264_picture_t            m_pic_out;
    x264_t                  * m_pEncoder;
    x264_param_t              m_param;

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
    /* 通知主窗口更新 */
    void signal_get_image(const QImage& image);
    /* 通知主线程初始化流失败 */
    void signal_init_video_stream_error();

    void signal_peer_close();

private:
    /* 初始化ffmpeg */
    void __Init();
public:

    /* 获取裸流并进行图片解析，然后将解析出来的图片通过信号发给主线程进行显示 */
    void Play();

private:
    QMutex                 mutex;

    QHostAddress           m_addr;

private:
    /* FFmpeg处理接口 */
    AVPicture              m_AVPicture;
    AVFormatContext      * m_pAVFormatContext;
    AVCodecContext       * m_pAVCodecContext;
    AVFrame              * m_pAVFrame;
    SwsContext           * m_pSwsContext;
    AVPacket               m_AVPacket;
    int                    m_VideoStreamIdx;
    int                    m_VideoWidth;
    int                    m_VideoHeight;

};

#endif // VIDEODISPLAY_H
