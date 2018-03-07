#include "NamaOpenGLWidget.h"
#include "NamaDemo_YXL.h"
#include <qtimer.h>
#include <gl/GLU.h>


NamaOpenGLWidget::NamaOpenGLWidget(QWidget *parent)
	: QOpenGLWidget(parent),
	_timer(NULL),
	_mainWnd(NULL),
	_texID(0),
	_isStop(false)
{
}

NamaOpenGLWidget::~NamaOpenGLWidget()
{
}

void NamaOpenGLWidget::AutoRepaint(bool isOn, float interval)
{
	if (NULL == _timer)
	{
		return;
	}
	if (true == isOn)
	{
		_timer->start(interval);
	}
	else 
	{
		_timer->stop();
	}
}

void NamaOpenGLWidget::SetMainWnd(NamaDemo_YXL * mainWnd)
{
	_mainWnd = mainWnd;
}

void NamaOpenGLWidget::Stop(bool isStop)
{
	_isStop = isStop;
}

bool NamaOpenGLWidget::IsStop()
{
	return _isStop;
}

void NamaOpenGLWidget::initializeGL()
{
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width(), 0, height(), 0, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//texture
	glGenTextures(1, &_texID);
	glBindTexture(GL_TEXTURE_2D, _texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	_timer = std::tr1::shared_ptr<QTimer>(new QTimer(this));
	connect(_timer.get(), SIGNAL(timeout()), SLOT(update()));
	_timer->start(1000.0/30);

	_mainWnd->InitNama();
}

void NamaOpenGLWidget::resizeGL(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, 0, 1000);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void NamaOpenGLWidget::paintGL()
{
	if (NULL == _mainWnd)
	{
		return;
	}

	cv::Mat frame = _mainWnd->GetNextFrame();
	if (true == frame.empty())
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, _texID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame.cols, frame.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, frame.data);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ShowFrame();

	static int count(0);
	static int64 timePre = cv::getTickCount();

	++count;
	if (count > 1000)
	{
		count = 0;
		int64 tmp = cv::getTickCount();
		float fps = cv::getTickFrequency() * 1000.0 / (tmp - timePre);
		printf("FPS: %6.3f\n", fps);
		timePre = tmp;
	}

	//qDebug("paintGL: %d", idx++);
}

void NamaOpenGLWidget::ShowFrame()
{
	if (_isStop)
	{
		return;
	}

	glBindTexture(GL_TEXTURE_2D, _texID);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(0.0f, height(), 0.0f);
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(width(), height(), 0);
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(width(), 0.0f, 0.0f);
	glEnd();
}
