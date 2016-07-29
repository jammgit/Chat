#ifndef VIDEODISPLAY_H
#define VIDEODISPLAY_H

/*  描述：本地视频显示、以及提供传输给对端的源的接口
*/

#include <QObject>
#include <QCamera>
#include <QImage>
#include <QWidget>
#include <QPoint>
#include <QCameraImageCaptureControl>
#include <QCameraImageCapture>
#include <QCameraViewfinder>
#include <QLabel>

class VideoDisplay : public QObject
{
    Q_OBJECT
public:
    explicit VideoDisplay(QObject *parent = 0);
    ~VideoDisplay();
    /* 打开摄像头 */
    void OpenCamera();
    /* 关闭摄像头 */
    void CloseCamera();

    bool CameraIsOpen()
    {
        return m_isopen;
    }

    /* 获取一张图片 */
    const QImage& CaptureIamge();

    /* 设置初始显示位置,移动窗口 */
    void SetInitPosition(const QPoint& mainwindow_position)
    {
         (m_pViewfinder==nullptr)?0:(m_pViewfinder->setGeometry
                 (mainwindow_position.x() + 20,
                  mainwindow_position.y() + 450,
                  151,131),0);
    }
    QCameraViewfinder* GetViewfinder()
    {
        return m_pViewfinder;
    }
    void SetPosition(const QPoint& point)
    {
        m_position = point;
    }
    void MoveWindow(const QPoint& global)
    {
        m_pViewfinder->move(global - m_position);
    }

private:
    /* 初始化摄像头 */
    void __Init_Camera();

signals:
    void signal_capture_image(const QImage& image);

private slots:
    /* 获取一张图片 */
    void slot_capture_image(int,QImage);

private:
    bool m_isopen;
    QPoint m_position;
    QCamera *m_pCamera;
    QCameraViewfinder *m_pViewfinder;
    QCameraImageCapture *m_pImageCapture;

};

#endif // VIDEODISPLAY_H
