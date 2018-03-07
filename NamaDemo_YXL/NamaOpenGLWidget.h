#pragma once
#define POINTER_64 __ptr64

#include <QOpenGLWidget>
#include <QTime>
#include <memory>

class NamaDemo_YXL;

class NamaOpenGLWidget : public QOpenGLWidget
{
	Q_OBJECT

public:
	NamaOpenGLWidget(QWidget *parent);
	~NamaOpenGLWidget();

	void AutoRepaint(bool isOn, float interval=1000.0/30);

	void SetMainWnd(NamaDemo_YXL* mainWnd);

	void Stop(bool isStop);
	bool IsStop();

protected:
	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	void ShowFrame();

protected:
	std::tr1::shared_ptr<QTimer> _timer;
	NamaDemo_YXL* _mainWnd;

	GLuint _texID;
	bool _isStop;
};
