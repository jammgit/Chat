#ifndef MSGTYPE_H
#define MSGTYPE_H

#include <QString>

typedef struct
{
    QString filepath;            //文件路径
    QString transname;           //传输时的文件名，针对传输同名文件的情况
}Source;

#define PICTURE_NAME_FILTER "Image Files(*.png *.jpg)"

#define END                 QString("END")
/* 由文本传输套接字使用 */
#define MSG_DOWNLOAD_IMAGE 'B'   // msg format -> msgtype:timestring:filename;
#define MSG_DOWNLOAD_FILE  'D'   // msg format -> msgtype:timestring:filename;
#define MSG_IMAGE_INFO     'H'   // msg format -> no(processed by 图片传输套接字)
#define MSG_FILE_INFO      'C'   // msg format -> msgtype:timestring:filename;
#define MSG_TEXT           'E'   // msg format -> msgtype:timestring:content;
#define MSG_EMOJI          'F'   // msg format -> msgtype:timestring:content;
#define MSG_SHAKE          'G'   // msg format -> msgtype:timestring:;


/* 文本框格式宏 */
#define TEXT_FRONT          QString("<p align=\"%1\" height=\"20\"><font style=\"font-family:%2\" color=\"%3\" size=\"%4\">")
#define TEXT_BACK           QString("</font></p>")
#define PIC_HTML_STRING     QString("<p align=\"%1\"><img src=\"%2\" height=\"%3\" width=\"%4\"/></p>")
#define RIGHT               "right"
#define LEFT                "left"
#define CENTER              "center"
#define TEXT_COLOR          "#505050"
#define TEXT_COLOR_2        "#000000"
#define TEXT_COLOR_3        "#FF0000"
#define NAME_COLOR          "#0099FF"
#define TIME_COLOR          "#123456"
#define FONT_SIZE           "2"
#define FONT                "微软雅黑"
#define TIME_DISPLAY_SPACE  10000


/* use by:findterminal.h */
/*  地址信息描述：多播地址为225.12.23.60，端口9999
 *              单播端口 7777
 *  A、B、C类地址都各有一个IP段专用做内网(私有地址)
 *      A类 10.0.0.0 --10.255.255.255
 *      B类 172.16.0.0--172.31.255.255
 *      C类 192.168.0.0--192.168.255.255
 */
#define MULTICAST_PORT 9999
#define MULTICAST_ADDR "225.12.23.60"
#define SINGLE_PORT 7777


/* use by:textchat.h */
/* 文本聊天的服务端端口是8888 */
#define TEXTCHAT_SERVER_PORT 8888
/* 类内部调用的控制消息，非Base64编码 */
#define ACCEPT   QString("accept").toUtf8()
#define REJECT   QString("reject").toUtf8()
#define CLOSE    QString("!").toUtf8()
/* 这是糟糕的一个设计：发送此文本表示结束聊天，所以如果用户输入
 * 此文本即发生结束聊天，问题来源：QT socket同步关闭！不过--
 * Base64编码解决了问题  */

#define PICTURE_SERVER_PORT 8887
#define FILE_SERVER_PORT    8886


#endif // MSGTYPE_H

