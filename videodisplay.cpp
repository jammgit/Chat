#include "videodisplay.h"

///////////////////////////////////////////////////////////////////////
/// MyVideo_Send_Thread
///////////////////////////////////////////////////////////////////////////
MyVideo_Send_Thread::MyVideo_Send_Thread(QVideoWidget*pwin, QObject* parent)
    : QThread(parent),m_pSocket(nullptr),m_pServer(nullptr),m_pWin(pwin),
      m_pVideoSend(nullptr)
{

}
MyVideo_Send_Thread::~MyVideo_Send_Thread()
{
    if (m_pSocket)
    {
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
    if (m_pServer)
    {
        m_pServer->deleteLater();
        m_pServer = nullptr;
    }
    if (m_pVideoSend)
    {
        m_pVideoSend->deleteLater();
        m_pVideoSend = nullptr;
    }
    this->quit();
    this->wait();
}
void MyVideo_Send_Thread::run()
{
    if (!m_pServer)
    {
        m_pServer = new QTcpServer();
        if (!m_pServer)
        {
            QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
            exit(0);
        }

        if(!m_pServer->listen(QHostAddress::AnyIPv4, VIDEO_SERVER_PORT))
        {
            QMessageBox::information(nullptr, "错误", "初始化网络出现错误");
            exit(0);
        }

    }
    connect(m_pServer, SIGNAL(newConnection()), this, SLOT(slot_new_connection()));

    connect(this, SIGNAL(finished()), this, SLOT(slot_finished()));

    this->exec();
}

void MyVideo_Send_Thread::slot_finished()
{
//    if (m_pServer)
//    {
//        m_pServer->close();
//        delete m_pServer;
//        m_pServer = nullptr;
//    }
//    if (m_pSocket)
//    {
//        m_pSocket->close();
//        delete m_pSocket;
//        m_pSocket = nullptr;
//    }

//    if (m_pVideoSend)
//    {
//        delete m_pVideoSend;
//        m_pVideoSend = nullptr;
//    }
}

void MyVideo_Send_Thread::slot_new_connection()
{
    if (m_pServer->hasPendingConnections())
    {
        m_pSocket = m_pServer->nextPendingConnection();

        if (!m_pSocket)
        {
            QMessageBox::information(nullptr, "错误", "返回连接时错误");
            exit(0);
        }

        m_pVideoSend = new VideoDisplay_Send(m_pWin, m_pSocket);
    }
}



///////////////////////////////////////////////////////////////////////////////
/// VideoDisplay_Send
/// ///////////////////////////////////////////////////////////////////////////
VideoDisplay_Send::VideoDisplay_Send(QVideoWidget* pwin,QTcpSocket* socket, QObject *parent)
    : QObject(parent),m_pWin(pwin),m_pSocket(socket)
{
    this->__Init_Camera();
}

VideoDisplay_Send::~VideoDisplay_Send()
{
    if (m_pTimer)
    {
        m_pTimer->deleteLater();
        m_pTimer = nullptr;
    }
    if (m_pImageCapture)
    {
        m_pImageCapture->deleteLater();
        m_pImageCapture = nullptr;
    }
    if (m_pCamera)
    {
        m_pCamera->stop();
        m_pCamera->setViewfinder((QVideoWidget*)nullptr);
        m_pCamera->deleteLater();
        m_pCamera = nullptr;
    }
}

void VideoDisplay_Send::__Init_Camera()
{
    m_pCamera = new QCamera;

    m_pImageCapture = new QCameraImageCapture(m_pCamera);

    m_pCamera->setViewfinder(m_pWin);


    m_pCamera->start();

    m_pTimer = new QTimer;

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_capture_image()));

    m_pTimer->start(50);

    connect(m_pImageCapture, SIGNAL(imageCaptured(int,QImage)),
            this, SLOT(slot_capture_image(int,QImage)));
}

void VideoDisplay_Send::slot_capture_image()
{
    m_pImageCapture->capture();
}

/* 获取一张图片 */
void VideoDisplay_Send::slot_capture_image(int,QImage image)
{
    if (!m_pSocket)
        return;
    int w = image.width();
    int h = image.height();

    //////////////////////////////////////////////////////////////
    /// 转码成yuv
    ///////////////////////////////////////////////////////////////
    static AVFrame* pframe = av_frame_alloc();

    static int numbytes = avpicture_get_size(PIX_FMT_RGB32, w, h);


    static uint8_t *buffer = (uint8_t*)av_malloc(numbytes*sizeof(uint8_t));

    int ret = avpicture_fill((AVPicture*)pframe, buffer, PIX_FMT_RGB32, w, h);

    if (ret < 0)
    {
        qDebug() << "error1";
    }
    pframe->data[0] = image.bits(); //读出image的数据

    static AVFrame *pframe_yuv = av_frame_alloc();
    static int  numbytes_yuv = avpicture_get_size(PIX_FMT_YUV420P, w, h);;
    static uint8_t *buffer_yuv = (uint8_t*)av_malloc(numbytes_yuv*sizeof(uint8_t));

    ret = avpicture_fill((AVPicture*)pframe_yuv, buffer_yuv, PIX_FMT_YUV420P, w, h);
    if (ret < 0)
    {
        qDebug() << "error2";
    }


    static SwsContext *rgb_to_yuv_ctx = sws_getContext(w, h, PIX_FMT_RGB32,
                                                w, h, PIX_FMT_YUV420P,
                                                SWS_BICUBIC, NULL,NULL,NULL);  //上下文

    if (!rgb_to_yuv_ctx)
    {
        qDebug() << "error3";
    }

    ret = sws_scale(rgb_to_yuv_ctx, pframe->data, pframe->linesize,
              0, h, pframe_yuv->data, pframe_yuv->linesize);

    if (ret < 0)
    {
        qDebug() << "error4";
    }

    /////////////////////////////////////////////////////////////////////////////
    /// 编码成h264
    ///////////////////////////////////////////////////////////////////////////////

    static int width = w;
    static int height = h;
    static int fps = 25;
    static x264_picture_t pic_in,pic_out;
    static x264_t *encoder;
    static x264_param_t m_param;
    static bool b = true;

    if (b)
    {/* 初始化,不需要重新做 */
        x264_param_default_preset(&m_param,"veryfast","zerolatency");
        m_param.i_threads = 1;
        m_param.i_width = width;
        m_param.i_height = height;
        m_param.i_fps_num = fps;
        m_param.i_bframe = 10;
        m_param.i_fps_den = 1;
        m_param.i_keyint_max = 25;
        m_param.b_intra_refresh = 1;
        m_param.b_annexb = 1;
        x264_param_apply_profile(&m_param,"main");
        encoder = x264_encoder_open(&m_param);
        x264_encoder_parameters( encoder, &m_param );
        x264_picture_alloc(&pic_in, X264_CSP_I420, width, height);

        b = !b;
    }
    /* 将yuv数据转交给x264接口 */
    pic_in.img.plane[0] = pframe_yuv->data[0];
    pic_in.img.plane[1] = pframe_yuv->data[1];
    pic_in.img.plane[2] = pframe_yuv->data[2];

    static int64_t i_pts = 0;
    x264_nal_t *nals;
    int nnal;

    pic_in.i_pts = i_pts++;
    x264_encoder_encode(encoder, &nals, &nnal, &pic_in, &pic_out);
    x264_nal_t *nal;
    FILE *file = fopen("me.h264", "ab+");
    for (nal = nals; nal < nals + nnal; nal++)
    {
      if (m_pSocket)
      {
          qint64 ret = m_pSocket->write(reinterpret_cast<const char *>(nal->p_payload), nal->i_payload);

          if (ret < 0)
              qDebug() << ret;
      }
      fwrite(reinterpret_cast<const char *>(nal->p_payload),1,nal->i_payload,file);
    }
    fclose(file);
}

void VideoDisplay_Send::slot_close_camera()
{

}

void VideoDisplay_Send::slot_open_camera()
{

}

////////////////////////////////////////////////////////////////////////////
/// MyVideo_Recv_Thread
//////////////////////////////////////////////////////////////////////////////

MyVideo_Recv_Thread::MyVideo_Recv_Thread(VideoDisplay_Recv*recv, QObject* parent)
    :QThread(parent),m_pVideoRecv(recv)
{

}

MyVideo_Recv_Thread::~MyVideo_Recv_Thread()
{
    this->quit();
    this->wait();
}

void MyVideo_Recv_Thread::run()
{
    m_pVideoRecv->Play();
//    this->exec();
}

void MyVideo_Recv_Thread::slot_finished()
{

}

///////////////////////////////////////////////////////////////////////////////
/// VideoDisplay_Recv
/// ///////////////////////////////////////////////////////////////////////////

VideoDisplay_Recv::VideoDisplay_Recv(const QHostAddress& addr, QObject*parent)
    : QObject(parent),m_addr(addr)
{
    this->__Init();
}

VideoDisplay_Recv::~VideoDisplay_Recv()
{

}

void VideoDisplay_Recv::__Init()
{
    videoStreamIndex=-1;
    av_register_all();                          //注册库中所有可用的文件格式和解码器
    avformat_network_init();                    //初始化网络流格式,使用网络流时必须先执行
    pAVFormatContext = avformat_alloc_context();//申请一个AVFormatContext结构的内存,并进行简单初始化
    pAVFrame=av_frame_alloc();

    QString url("tcp://");
    url.append(m_addr.toString());
    url.append(QString(":%1").arg(QString::number(VIDEO_SERVER_PORT)));
    qDebug() << url;
    //打开视频流
    int result=avformat_open_input(&pAVFormatContext, url.toStdString().c_str(),NULL,NULL);
    if (result<0){
        qDebug()<<"打开视频流失败";
        exit(0);
    }
    qDebug() << "打开视频流成功";

    //获取视频流信息
    result=avformat_find_stream_info(pAVFormatContext,NULL);
    if (result<0){
        qDebug()<<"获取视频流信息失败";
        exit(0);
    }

    //获取视频流索引
    videoStreamIndex = -1;
    for (uint i = 0; i < pAVFormatContext->nb_streams; i++) {
        if (pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex==-1){
        qDebug()<<"获取视频流索引失败";
        exit(0);
    }

    //获取视频流的分辨率大小
    pAVCodecContext = pAVFormatContext->streams[videoStreamIndex]->codec;
    videoWidth=pAVCodecContext->width;
    videoHeight=pAVCodecContext->height;

    avpicture_alloc(&pAVPicture,PIX_FMT_RGB32,videoWidth,videoHeight);

    AVCodec *pAVCodec;

    //获取视频流解码器
    pAVCodec = avcodec_find_decoder(pAVCodecContext->codec_id);
    pSwsContext = sws_getContext(videoWidth,videoHeight,AV_PIX_FMT_YUV420P,videoWidth,videoHeight,PIX_FMT_RGB32,SWS_BICUBIC,0,0,0);

    //打开对应解码器
    result=avcodec_open2(pAVCodecContext,pAVCodec,NULL);
    if (result<0){
        qDebug()<<"打开解码器失败";
        exit(0);
    }

    qDebug()<<"初始化视频流成功";

}

void VideoDisplay_Recv::Play()
{
    //一帧一帧读取视频
    int frameFinished=0;
    while (true){
        if (av_read_frame(pAVFormatContext, &pAVPacket) >= 0){
            if(pAVPacket.stream_index==videoStreamIndex){
                qDebug()<<"开始解码"<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                avcodec_decode_video2(pAVCodecContext, pAVFrame, &frameFinished, &pAVPacket);

                if (frameFinished){
                    mutex.lock();
                    sws_scale(pSwsContext,(const uint8_t* const *)pAVFrame->data,pAVFrame->linesize,0,videoHeight,pAVPicture.data,pAVPicture.linesize);
                    //发送获取一帧图像信号
                    QImage image(pAVPicture.data[0],videoWidth,videoHeight,QImage::Format_RGB32);
                    emit this->signal_get_image(image);
                    mutex.unlock();
                }
            }
        }
        else
        {
            qDebug() << "play over";
            av_free_packet(&pAVPacket);//释放资源,否则内存会一直上升
            break;
        }
        av_free_packet(&pAVPacket);//释放资源,否则内存会一直上升
    }
}

