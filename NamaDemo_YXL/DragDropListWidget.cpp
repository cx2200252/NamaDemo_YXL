#include "DragDropListWidget.h"
#include "NamaDemo_YXL.h"

DragDropListWidget::DragDropListWidget(QWidget *parent)
	: QListWidget(parent)
{
}

DragDropListWidget::~DragDropListWidget()
{
}

void DragDropListWidget::SetParent(NamaDemo_YXL * parent)
{
	_parent = parent;
}

void DragDropListWidget::dragEnterEvent(QDragEnterEvent * event)
{
	QListWidget::dragEnterEvent(event);
}

void DragDropListWidget::dropEvent(QDropEvent * event)
{
	QListWidget::dropEvent(event);
	_parent->UpdatePropsUsed();
}
