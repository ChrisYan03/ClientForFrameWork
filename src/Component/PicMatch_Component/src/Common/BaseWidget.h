#pragma once

#include <QWidget>

class BaseWidget : public QWidget
{
	Q_OBJECT

public:
	BaseWidget(QWidget *parent = Q_NULLPTR);
	virtual ~BaseWidget();

protected:
	virtual void paintEvent(QPaintEvent *event);

	void ChangeStyle(QWidget* parent, QWidget* child, const QString& name);
};
