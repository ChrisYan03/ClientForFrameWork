#ifndef PICMATCHWIDGET_H
#define PICMATCHWIDGET_H

#include <QWidget>

class PicMatchWidget : public QWidget
{
    Q_OBJECT

public:
    PicMatchWidget(QWidget *parent = nullptr);
    ~PicMatchWidget();

    void InitUI();
    void InitPicPlayer(QWidget* playerWidget);

private:
    int m_handle;
};
#endif // PICMATCHWIDGET_H
