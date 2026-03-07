#include "BaseWidget.h"
#include <QStyleOption>
#include <QPainter>

BaseWidget::BaseWidget(QWidget *parent)
	: QWidget(parent)
{
}

BaseWidget::~BaseWidget()
{
}

void BaseWidget::paintEvent(QPaintEvent* event)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter painter(this);
	this->style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);
}

void BaseWidget::ChangeStyle(QWidget* parent, QWidget* child, const QString& name)
{
	if (parent && child)
	{
		parent->style()->unpolish(child);
		child->setObjectName(name);
		parent->style()->polish(child);
	}
}
