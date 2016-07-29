#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QObject>
#include <QTextEdit>

class MyTextEdit : public QTextEdit
{
public:
    MyTextEdit(QWidget *parent = nullptr);

};

//class TriggerTextEdit
//    : public QTextEdit
//{
//    Q_OBJECT
//public:
//    TriggerTextEdit(QWidget* parent = nullptr)
//    : QTextEdit(parent)
//    , shortcut(NONE_SHORTCUT)
//    {
//    }
//    enum ShortcutType
//    {
//        NONE_SHORTCUT = 0,
//        ENTER_SHORTCUT = 1,
//        CTRL_ENTER_SHORTCUT = 2,
//    };
//    void setShortcut(ShortcutType shortcut_)
//    {
//        shortcut = shortcut_;
//    }
//signals:
//    void triggerSignal();
//public:
//    virtual void keyPressEvent( QKeyEvent * ev )
//    {
//        if ((shortcut == ENTER_SHORTCUT && (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter))
//            || (shortcut == CTRL_ENTER_SHORTCUT && (ev->key() == Qt::Key_Return || ev->key() == Qt::Key_Enter) && ( ev->modifiers() & Qt::ControlModifier )))
//        {
//            emit triggerSignal();
//            ev->accept();
//            return;
//        }
//        QTextEdit::keyPressEvent(ev);
//    }
//private:
//    ShortcutType shortcut;
//};

#endif // MYTEXTEDIT_H
