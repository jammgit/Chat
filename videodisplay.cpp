#include "videodisplay.h"

///////////////////////////////////////////////////////////////////////
/// MyVideo_Send_Thread
///////////////////////////////////////////////////////////////////////////
MyVideo_Send_Thread::MyVideo_Send_Thread(VideoDisplay_Send* send, QObject* parent)
    : QThread(parent),m_pSocket(nullptr),m_pServer(nullptr),m_pVideoSend(send)
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
//    if (m_pVideoSend)
//    {
//        m_pVideoSend->deleteLater();
//        m_pVideoSend = nullptr;
//    }
    this->quit();
    this->wait();
}
void MyVideo_Send_Thread::run()
{
    qDebug() << "video server start";
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
    qDebug() << "finish";
    if (m_pSocket)
    {
        m_pSocket->deleteLater();
        m_pSocket = nullptr;
    }
//    if (m_pServer)
//    {
//        m_pServer->deleteLater();
//        m_pServer = nullptr;
//    }
//    if (m_pVideoSend)
//    {
//        m_pVideoSend->deleteLater();
//        m_pVideoSend = nullptr;
//    }
}

void MyVideo_Send_Thread::slot_new_connection()
{
    qDebug() << "new connect";
    if (m_pServer->hasPendingConnections() && m_pSocket == nullptr)
    {
        m_pSocket = m_pServer->nextPendingConnection();

        if (!m_pSocket)
        {
            QMessageBox::information(nullptr, "错误", "返回连接时错误");
            exit(0);
        }
        qDebug() << "have socket";
        if (m_pVideoSend)
        {
            m_pVideoSend->Start(m_pSocket);
        }

    }
}



///////////////////////////////////////////////////////////////////////////////
/// VideoDisplay_Send
/// ///////////////////////////////////////////////////////////////////////////
VideoDisplay_Send::VideoDisplay_Send(QVideoWidget* pwin,QObject *parent)
    : QObject(parent),m_pWin(pwin),m_pSocket(nullptr)
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
        m_pCamera->deleteLater();
        m_pCamera = nullptr;
    }
}

void VideoDisplay_Send::__Init_Camera()
{
    /* 初始化摄像头 */
    m_pCamera = new QCamera;
    m_pCamera->setCaptureMode(QCamera::CaptureStillImage);

    m_pImageCapture = new QCameraImageCapture(m_pCamera);
    m_pImageCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    m_pCamera->setViewfinder(m_pWin);


    m_pTimer = new QTimer;

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(slot_capture_image()));

    connect(m_pImageCapture, SIGNAL(imageCaptured(int,QImage)),
            this, SLOT(slot_capture_image(int,QImage)));

    m_is_init_x264 = true;
}


void VideoDisplay_Send::Start(QTcpSocket* socket)
{
    m_pSocket = socket;
    if (m_pSocket)
    {
        m_pCamera->start();
        m_pTimer->start(50);
    }
}

void VideoDisplay_Send::slot_stop_timer()
{/* 结束对话，关闭视频时就调用这个函数，结束使用socket */
    m_pTimer->stop();
    m_pSocket = nullptr;
}

void VideoDisplay_Send::slot_capture_image()
{
    m_pImageCapture->capture();
}

void VideoDisplay_Send::CloseCamera()
{
    m_pCamera->stop();
    m_pTimer->stop();
}

void VideoDisplay_Send::OpenCamrea()
{
    m_pCamera->start();
    if (m_pSocket)
    {/* 如果是在正在聊天，说明socket是可用的，那么才开始计时器截图发送
      * 否则，不开始计时器
      */
        m_pTimer->start(50);
    }
}

/* 获取一张图片,并发送 */
void VideoDisplay_Send::slot_capture_image(int,QImage image)
{


    if (!m_pSocket)
    {
        m_pTimer->stop();
        return;
    }
    int w = image.width();
    int h = image.height();

    //////////////////////////////////////////////////////////////
    /// 转码成yuv
    ///////////////////////////////////////////////////////////////
    qDebug() << "yuv";
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


    if (m_is_init_x264)
    {/* 初始化,不需要重新做 */
        x264_param_default_preset(&m_param,"veryfast","zerolatency");

        m_param.i_threads = 1;
        m_param.i_width = w;
        m_param.i_height = h;
        m_param.i_fps_num = m_fps;
        m_param.i_bframe = 5;               //在两个参考帧(指每一个P帧/B帧最多参考其他帧的数量)之间B帧的数目
        m_param.i_frame_reference = 5;      //参考帧最大帧数（对端avformat_find_stream_info快了很多）
        m_param.i_fps_den = 1;              //i_fps_num/i_fps_den就是帧率
        m_param.i_keyint_max = 25;          //每过多少帧设置一个IDR帧
        m_param.b_intra_refresh = 1;
        m_param.b_annexb = 1;               // 设置开始码(4bytes)
        x264_param_apply_profile(&m_param,"main");
        m_pEncoder = x264_encoder_open(&m_param);
        x264_encoder_parameters(m_pEncoder, &m_param );
        x264_picture_alloc(&m_pic_in, X264_CSP_I420, w, h);

        //m_i_pts = 0;
        m_is_init_x264 = false;
    }
    /* 将yuv数据转交给x264接口 */
    m_pic_in.img.plane[0] = pframe_yuv->data[0];
    m_pic_in.img.plane[1] = pframe_yuv->data[1];
    m_pic_in.img.plane[2] = pframe_yuv->data[2];

    x264_nal_t *nals;
    int nnal;

    m_pic_in.i_pts = m_i_pts++;
    x264_encoder_encode(m_pEncoder, &nals, &nnal, &m_pic_in, &m_pic_out);

    x264_nal_t *nal;
    for (nal = nals; nal < nals + nnal; nal++)
    {
        if (m_pSocket)
        {
            qint64 ret = m_pSocket->write(reinterpret_cast<const char *>(nal->p_payload), nal->i_payload);

            if (ret < 0)
            {
                qDebug() << "peer close";
                m_pSocket = nullptr;
                m_is_init_x264 = true;                       // 需要重新初始化x264以构造PPS SPS
                //x264_encoder_close(m_pEncoder);            // 下次初始化,这里释放资源，等再初始化时不知道为什么异常退出，
                //x264_picture_clean(&m_pic_in);             // 暂时不了解
                //m_pTimer->stop();                            // timer cannot stop from another thread
                emit this->signal_peer_close();

                break;
            }
            else if (ret == 0)
            {
                qDebug() << "no buffer";
            }
        }
    }


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
    /* 注册库中所有可用的文件格式和解码器,执行时长：0 */
    av_register_all();

    /* 使用网络流就需要初始化，执行时长：0 */
    avformat_network_init();

    /* 申请格式上下文，并进行初始化 */
    m_pAVFormatContext = avformat_alloc_context();
    /* 分配读取视频流帧的容器 */
    m_pAVFrame = av_frame_alloc();

    /* 构造视频流地址 */
    QString url("tcp://");
    url.append(m_addr.toString());
    url.append(QString(":%1").arg(QString::number(VIDEO_SERVER_PORT)));

    /* 在avformat_open_input前执行，代表需要读到多少数据才进行分析 */
    m_pAVFormatContext->probesize = 100 * 1024;

    /* 打开视频流，读出header，由于一些数据没有带头（264的头是什么不是很清楚），所以建议调用
     * avformat_find_stream_info去读stream然后找到更多视频流信息
    */
    AVDictionary *options = NULL;

    av_dict_set(&options, "video_size", "640x480", 0);
    av_dict_set(&options, "pixel_format", "yuv420", 0);

    int result = avformat_open_input(&m_pAVFormatContext, url.toStdString().c_str(),NULL,&options);
    if (result<0){
        emit this->signal_init_video_stream_error();
        return;
    }
    av_dict_free(&options);
    /* 数据包不入缓冲区，相当于在avformat_find_stream_info接口内部读取的每一帧数据只用于分析，不显示
     * avformat_find_stream_info执行缓慢，取消使用，这里直接使用硬编码方式
    */
    m_pAVFormatContext->flags |= AVFMT_FLAG_NOBUFFER;
    /* 最长分析流信息的时间 */
    m_pAVFormatContext->max_analyze_duration = 5 * AV_TIME_BASE;

//    /* 获取视频流信息，初始化时间很长 */
//    result=avformat_find_stream_info(pAVFormatContext,NULL);
//    if (result<0){
//        qDebug()<<"获取视频流信息失败";
//        exit(0);
//    }
//    /* 获取视频流的索引 */
//    videoStreamIndex = -1;
//    for (uint i = 0; i < pAVFormatContext->nb_streams; i++) {
//        if (pAVFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
//            videoStreamIndex = i;
//            break;
//        }
//    }
    /* 目前发送端只有一个流，故视频流所以一定是0 */
    m_pAVFormatContext->streams[0]->codec->codec_id = AV_CODEC_ID_H264;
    m_pAVFormatContext->streams[0]->codec->width = 640;
    m_pAVFormatContext->streams[0]->codec->height = 480;
    m_pAVFormatContext->streams[0]->codec->ticks_per_frame = 2;             //h264设置为2
//    m_pAVFormatContext->streams[0]->codec->pix_fmt = PIX_FMT_RGB32;
//    m_pAVFormatContext->streams[0]->pts_wrap_bits = 32;
//    m_pAVFormatContext->streams[0]->time_base.den = 1000;                   //时基。通过该值可以把PTS，DTS转化为真正的时间
//    m_pAVFormatContext->streams[0]->time_base.num = 1;
//    m_pAVFormatContext->streams[0]->avg_frame_rate.den = 90;
//    m_pAVFormatContext->streams[0]->avg_frame_rate.num = 3;
//    m_pAVFormatContext->streams[0]->r_frame_rate.den = 60;
//    m_pAVFormatContext->streams[0]->r_frame_rate.num = 2;
    /* 获取视频流索引,因为一个AVFormatContext可以包含多个流（nb_streams最大值是20）
     * ,找到视频流，从而找到解码器
    */
    m_VideoStreamIdx = 0;


    //获取视频流的分辨率大小
    m_pAVCodecContext = m_pAVFormatContext->streams[m_VideoStreamIdx]->codec;
    m_VideoWidth=m_pAVCodecContext->width;
    m_VideoHeight=m_pAVCodecContext->height;

    avpicture_alloc(&m_AVPicture,PIX_FMT_RGB32,m_VideoWidth,m_VideoHeight);

    AVCodec *pAVCodec;

    /* 获取视频流解码器 */
    pAVCodec = avcodec_find_decoder(m_pAVCodecContext->codec_id);

    m_pSwsContext = sws_getContext(m_VideoWidth,m_VideoHeight,PIX_FMT_YUV420P,
                                   m_VideoWidth,m_VideoHeight,PIX_FMT_RGB32,
                                   SWS_BICUBIC,0,0,0);

    /* 打开对应解码器 */
    result = avcodec_open2(m_pAVCodecContext,pAVCodec,NULL);
    if (result<0){
        emit this->signal_init_video_stream_error();
        return;
    }
    qDebug()<<"初始化视频流成功";

}

void VideoDisplay_Recv::Play()
{
    /* 一帧一帧读取视频 */
    int frames = 0;

    while (true){
        if (av_read_frame(m_pAVFormatContext, &m_AVPacket) >= 0)
        {
            if(m_AVPacket.stream_index == m_VideoStreamIdx)
            {/* 如果这个是视频流的数据，才进行解码显示 */
                avcodec_decode_video2(m_pAVCodecContext, m_pAVFrame, &frames, &m_AVPacket);

                if (frames)
                {/* frames是返回的帧数 */
                    mutex.lock();
                    sws_scale(m_pSwsContext,
                              (const uint8_t* const *)m_pAVFrame->data,
                              m_pAVFrame->linesize,
                              0,
                              m_VideoHeight,
                              m_AVPicture.data,
                              m_AVPicture.linesize);
                    /* 发送获取一帧图像信号,QImage创建文件并写磁盘，拖慢的速度 */
                    QImage image((uchar *)m_AVPicture.data[0],
                            m_VideoWidth,
                            m_VideoHeight,
                            QImage::Format_RGB32);
                    emit this->signal_get_image(image);
                    mutex.unlock();
                }
            }
        }
        else
        {
            av_free_packet(&m_AVPacket);
            avformat_free_context(m_pAVFormatContext);
            emit this->signal_peer_close();
            break;
        }
        av_free_packet(&m_AVPacket);
    }
}

