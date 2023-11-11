#ifndef EVENTFILTERS_H
#define EVENTFILTERS_H

#include "options.h"

#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QTextEdit>
#include <QMenuBar>
#include <QTimer>
#include <QDebug>
#include <QMainWindow>

class AOLineEditFilter : public QObject
{
    Q_OBJECT
public:
    bool preserve_selection = false;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        QLineEdit *lineEdit = qobject_cast<QLineEdit *>(obj);
       if (event->type() == QEvent::FocusOut && lineEdit != nullptr && preserve_selection) { // lost focus
            int start = lineEdit->selectionStart();
          #if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
            int len = lineEdit->selectionLength();
          #else
            int len = lineEdit->selectedText().length();
          #endif
            if (start != -1 && len != -1) {
              lineEdit->setSelection(start, len);\
              return true;
            }
        }
        return false;
    }
signals:
    void double_clicked();
};

class QMenuBarFilter : public QObject
{
    Q_OBJECT

public:
    bool collapseMenuBar = false;
    int originalMenuBarHeight = -1;
private:
    bool entered_zone = false;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        QMainWindow *mainWindow = qobject_cast<QMainWindow *>(obj);

        if (mainWindow != nullptr) {
            if (event->type() == QEvent::HoverMove) {
                if (originalMenuBarHeight == -1) {
                    originalMenuBarHeight = mainWindow->menuBar()->height();
                }
                if (Options::getInstance().menuBarLocked()) {
                    mainWindow->menuBar()->setFixedHeight(originalMenuBarHeight);
                    return false;
                }

                QPoint globalPos = QCursor::pos();
                QPoint mainWindowPos = mainWindow->mapFromGlobal(globalPos);

                int expandZoneHeight = 22;

                if (mainWindowPos.y() <= expandZoneHeight) {
                    if (!entered_zone)
                        mainWindow->menuBar()->setFixedHeight(originalMenuBarHeight);
                        entered_zone = true;
                } else {
                    if (entered_zone)
                        mainWindow->menuBar()->setFixedHeight(0);
                        entered_zone = false;
                }
            }
        }
        return false;
    }
};

class QTextEditFilter : public QObject
{
    Q_OBJECT

signals:
    void chat_return_pressed();

public:
    bool text_edit_preserve_selection = false;

protected:
    bool eventFilter(QObject *obj, QEvent *event) override {
        QTextEdit *textEdit = qobject_cast<QTextEdit *>(obj);

        if (textEdit != nullptr) {
            // Key press detection
            if (event->type() == QEvent::KeyPress) {
                QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(event);
                if (keyEvent != nullptr && keyEvent->key() == Qt::Key_Return) {
                    qDebug("Enter Key Pressed..."); // Debug it for now
                    emit chat_return_pressed();
                    return true;
                }
            }

            // Focus-out event handling
            if (event->type() == QEvent::FocusOut && text_edit_preserve_selection) {
                QTextCursor cursor = textEdit->textCursor();
                int start = cursor.selectionStart();
                int len = cursor.selectionEnd() - start;
                if (start != -1 && len != -1) {
                    cursor.setPosition(start);
                    cursor.setPosition(start + len, QTextCursor::KeepAnchor);
                    textEdit->setTextCursor(cursor);
                    return true;
                }
            }
        }
        return false;
    }
};

#endif // EVENTFILTERS_H
