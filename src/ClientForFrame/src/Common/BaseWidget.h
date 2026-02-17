#ifndef _BASE_WIDGET_H_
#define _BASE_WIDGET_H_

#include <QWidget>

class BaseWidget : public QWidget
{
	Q_OBJECT

public:
	BaseWidget(QWidget *parent = Q_NULLPTR);
	virtual ~BaseWidget();

protected:
	/*
	* @brief 绘制事件，继承QWidget的派生类控件不能设置qss，需要重新实现
	*/
	virtual void paintEvent(QPaintEvent *event);

	/*
	* @brief 修改控件风格并设置子控件名
	* @param parent 父控件
	* @param child 子控件
	* @param name 子控件名
	*/
	void ChangeStyle(QWidget* parent, QWidget* child, const QString& name);
};
#endif // _BASE_WIDGET_H_