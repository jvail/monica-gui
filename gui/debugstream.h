#ifndef DEBUGSTREAM_H
#define DEBUGSTREAM_H

#include <iostream>
#include <streambuf>
#include <string>

#include <QDebug>
#include <QTextEdit>

class DebugStream : public std::basic_streambuf<char>
{
public:
  DebugStream(std::ostream &stream, QTextEdit* text_edit, const Qt::GlobalColor &color) : m_stream(stream)
  {
    log_window = text_edit;
    m_old_buf = stream.rdbuf();
    stream.rdbuf(this);
    _color = color;
    qRegisterMetaType<QTextCharFormat>("QTextCharFormat");
    qRegisterMetaType<QTextCursor>("QTextCursor");
  }

  ~DebugStream()
  {
    m_stream.rdbuf(m_old_buf);
  }

  static void registerQDebugMessageHandler(){
    qInstallMessageHandler(myQDebugMessageHandler);
  }

  private:

  static void myQDebugMessageHandler(QtMsgType, const QMessageLogContext &, const QString &msg)
  {
    std::cout << msg.toStdString().c_str();
  }

protected:

  //This is called when a std::endl has been inserted into the stream
  virtual int_type overflow(int_type v)
  {
    if (v == '\n')
    {
      QTextCursor cursor(log_window->textCursor());
      QTextCharFormat format;
      format.setForeground(QBrush(QColor(_color)));
      cursor.movePosition(QTextCursor::End);
      cursor.insertText("");
      log_window->append("");
    }
    return v;
  }


  virtual std::streamsize xsputn(const char *p, std::streamsize n)
  {
    QTextCursor cursor(log_window->textCursor());
    QTextCharFormat format;
    format.setForeground(QBrush(QColor(_color)));
    QString str(p);

    if (str.contains("\n")) {
      QStringList strSplitted = str.split("\n");
      cursor.movePosition(QTextCursor::End);
      cursor.insertText(strSplitted.at(0), format);
      for (int i = 1; i < strSplitted.size(); i++) {
        log_window->append("");
        cursor.insertText(strSplitted.at(i), format);
      }
    }
    else {
      cursor.movePosition(QTextCursor::End);
      cursor.insertText(str, format);
    }

    cursor.movePosition(QTextCursor::End);
    return n;
  }

private:
    std::ostream &m_stream;
    std::streambuf *m_old_buf;
    QTextEdit* log_window;
    Qt::GlobalColor _color;
};


#endif // DEBUGSTREAM_H
