#include "videodisplay.h"

VideoDisplay::VideoDisplay(QObject *parent)
    : QObject(parent)
{
    this->__Init_Camera();
    m_isopen = false;
}

VideoDisplay::~VideoDisplay()
{
    delete m_pImageCapture;
    delete m_pViewfinder;
    delete m_pCamera;
}

void VideoDisplay::__Init_Camera()
{
    m_pCamera = new QCamera;
    m_pViewfinder = new QCameraViewfinder;
    m_pViewfinder->setWindowFlags(Qt::FramelessWindowHint | Qt::SubWindow);
    m_pViewfinder->hide();
    m_pImageCapture = new QCameraImageCapture(m_pCamera);

    m_pCamera->setViewfinder(m_pViewfinder);

    connect(m_pImageCapture, SIGNAL(imageCaptured(int,QImage)),
            this, SLOT(slot_capture_image(int,QImage)));
}

void VideoDisplay::OpenCamera()
{
    m_pViewfinder->show();
    m_pCamera->start();
    m_isopen = true;
}

void VideoDisplay::CloseCamera()
{
    m_pViewfinder->hide();
    m_pCamera->stop();
    m_isopen = false;
}

const QImage& VideoDisplay::CaptureIamge()
{
    m_pImageCapture->cancelCapture();
}

void VideoDisplay::slot_capture_image(int,QImage image)
{

}



