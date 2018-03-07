#pragma once

#define POINTER_64 __ptr64

#include <QtWidgets/QWidget>
#include "ui_NamaDemo_YXL.h"
#include "Nama.h"
#include <memory>
#include <qspinbox.h>
#include <QScrollArea>


class QProgressDialog;

struct ParamItem
{
	enum TYPE {
		TYPE_CHECKBOX,
		TYPE_SLIDER,
		TYPE_COMBOBOX,
		TYPE_NONE
	};
	TYPE type= TYPE_NONE;
	std::string show_name;
	std::string tooltip;
	std::string param;
	int prop_idx = -1;
	//TYPE_CHECKBOX
	bool valB = false;
	//TYPE_SLIDER
	int valI = 0;
	std::pair<int, int> range;
	float scale=0.0f;
	//TYPE_COMBO_BOX
	std::string valStr;
	std::vector<std::string> combo_texts;

	/*
	controls
	*/
	//for all
	QHBoxLayout* layout = nullptr;
	QSpinBox* spin_box = nullptr;
	//TYPE_CHECKBOX
	QCheckBox* check_box = nullptr;
	//TYPE_SLIDER
	QSlider* slider = nullptr;
	QLabel* label_name = nullptr;
	QLabel* label = nullptr;
	//TYPE_COMBO_BOX
	QComboBox* combobox = nullptr;
};

struct ParamList
{
	std::vector<ParamItem> params;
	QScrollArea* container=nullptr;
};

enum SOURCE_TYPE
{
	SOURCE_TYPE_CAM,
	SOURCE_TYPE_PIC,
	SOURCE_TYPE_VIDEO
};

class NamaDemo_YXL : public QWidget
{
	Q_OBJECT

public:
	NamaDemo_YXL(QWidget *parent = Q_NULLPTR);
	virtual ~NamaDemo_YXL();

	cv::Mat GetNextFrame();
	void InitNama();

	void UpdatePropsUsed();

protected:
	virtual void keyPressEvent(QKeyEvent * keyEvent);

private:
	void InitFromConfig();
	void LoadProps();
	//ignore _resDir prefix
	bool IsValidPropPath(CStr& path);
	bool AddPropUsed(CStr& path);
	void UpdateSpinBoxRange();

private:
	cv::Mat GetSourceImage();
	cv::Mat GetShowImage(cv::Mat src, cv::Mat nama_out);
	cv::Mat GetSaveImage(cv::Mat src, cv::Mat nama_out);

	void InitCtrls();

	void UpdateCtrlValue();

private:
	void UseSourcePicture(CStr& path);
	void UseSourceVideo(CStr& path);

private slots:
	void SetAllParameter();
	void UpdateParamsFromProp();
	void CheckBoxClick();
	void SliderChanged(int val);
	void SpinBoxChanged(int val);

	void ButtonClicked();
	void ValueChanged(int val);
	void PropsItemDoubleClicked(QTreeWidgetItem* item, int col);
	void UsedPropsItemDoubleClicked(QListWidgetItem* item);

	void CurrentTextChanged(QString str);

	void LoadParams();
	void SaveParams();

private:
	Ui::NamaDemo_YXLClass ui;

	void SetCtrlData(QAbstractSlider * slider, int value);
	void SetCtrlData(QRadioButton * btn, bool isCheck);
	void SetCtrlData(QCheckBox * btn, bool isCheck);
	void SetCtrlData(QComboBox * comboBox, int idx);
	void SetCtrlData(QComboBox * comboBox, CStr& txt);

private:
	std::vector<std::string> _propsUsed;

	std::string _cur_param_list;
	std::map<std::string, ParamList> _param_lists;

	bool _is_param_batch_update = false;

	//for source
private:
	SOURCE_TYPE _source_type = SOURCE_TYPE_CAM;
	std::shared_ptr<cv::VideoCapture> _cap_cam = nullptr;
	std::string _path_pic = "";
	cv::Mat _pic;
	std::string _path_video = "";
	std::shared_ptr<cv::VideoCapture> _cap_video = nullptr;

private:
	std::shared_ptr<QButtonGroup> _sourceBtnGroup=nullptr;

private:
	std::string _save_img_path_format;
	int _save_img_cur_idx = 0;

private:
	//nama
	std::shared_ptr<FU::Nama> _nama=nullptr;

	cv::VideoWriter* _writer;
};
