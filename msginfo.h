#ifndef MSGTYPE_H
#define MSGTYPE_H


#define MSG_BREVIARY_IMAGE 'A'   // msg format -> msgtype:timestring:filename:content;
#define MSG_IMAGE          'B'   // msg format -> msgtype:timestring:filename:content;
#define MSG_BREVIARY_FILE  'C'   // msg format -> msgtype:timestring:filename;
#define MSG_FILE           'D'   // msg format -> msgtype:timestring:filename:content;
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

#endif // MSGTYPE_H

