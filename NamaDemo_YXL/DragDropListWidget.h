#pragma once

#include <QListWidget>

class NamaDemo_YXL;
class DragDropListWidget : public QListWidget
{
	Q_OBJECT

public:
	DragDropListWidget(QWidget *parent);
	~DragDropListWidget();

	void SetParent(NamaDemo_YXL* parent);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

private:
	NamaDemo_YXL* _parent=nullptr;
};
