#pragma once

#define POINTER_64 __ptr64

#include <QtWidgets/QWidget>
#include "ui_NamaDemo_YXL.h"
#include "Nama.h"
#include <memory>
#include <qspinbox.h>
#include <QScrollArea>
#include <YXLJsonReader.h>



class QProgressDialog;
class NamaDemo_YXL;

class ParamItemBase
{
public:
	enum TYPE {
		TYPE_CHECKBOX,
		TYPE_SLIDER,
		TYPE_COMBOBOX,
		TYPE_SLIDER_LIST,
		TYPE_NONE
	};

	ParamItemBase(TYPE type) :_type(type) {}
	virtual ~ParamItemBase() {};

	static bool IsType(const rapidjson::Value & val)
	{
		return val.IsObject() && val.HasMember("type") && val["type"].IsString() && val.HasMember("param") && val["param"].IsString() && val.HasMember("prop_idx") && val["prop_idx"].IsInt();
	}
	virtual void LoadFromJson(const rapidjson::Value & val)
	{
		_show_name = JsonGetStr(val["show_name"]);
		if (val.HasMember("tooltip") && val["tooltip"].IsString())
			_tooltip = JsonGetStr(val["tooltip"]);
		_param = JsonGetStr(val["param"]);
		_prop_idx = JsonGetInt(val["prop_idx"]);
	}
	virtual void SaveToJson(rapidjson::Value & val, rapidjson::Document& doc)
	{
		auto& alloc = doc.GetAllocator();
		val.AddMember(JsonParseStr("show_name", doc), JsonParseStr(_show_name, doc), alloc);
		if(_tooltip!="")
			val.AddMember(JsonParseStr("tooltip", doc), JsonParseStr(_tooltip, doc), alloc);
		val.AddMember(JsonParseStr("param", doc), JsonParseStr(_param, doc), alloc);
		val.AddMember(JsonParseStr("prop_idx", doc), _prop_idx, alloc);
	}

	virtual void InitCtrl(NamaDemo_YXL* wnd);
	QLayout* GetLayout() const
	{
		return _layout;
	}

	void SetSpinBoxRange(int min, int max)
	{
		_spin_box->setRange(min, max);
	}
	virtual void UpdateCtrlValue()
	{
		_spin_box->setValue(_prop_idx);
	}
	virtual void SetCtrlValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed) = 0;
	virtual bool SetPropValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed, QObject* sender) = 0;
	virtual QObject* GetCtrl() = 0;
	virtual bool SpinBoxChanged(QSpinBox* spinbox)
	{
		if (spinbox != _spin_box)
			return false;
		_prop_idx = _spin_box->value();
		return true;
	}

protected:
	void SetCtrlData(QAbstractSlider * slider, int value)
	{
		if (slider->value() != value)
			slider->setValue(value);
		else
			slider->valueChanged(value);
	}
	void SetCtrlData(QRadioButton * btn, bool isCheck)
	{
		if (isCheck == btn->isChecked())
			btn->click();
		btn->click();
	}
	void SetCtrlData(QCheckBox * btn, bool isCheck)
	{
		//为了响应得调用click函数，值不变则调用两次
		if (isCheck == btn->isChecked())
			btn->click();
		btn->click();
	}
	void SetCtrlData(QComboBox * comboBox, int idx)
	{
		if (comboBox->currentIndex() != idx)
			comboBox->setCurrentIndex(idx);
		else
			comboBox->currentIndexChanged(idx);
	}
	void SetCtrlData(QComboBox * comboBox, CStr & txt)
	{
		comboBox->setCurrentText(QString::fromLocal8Bit(txt.c_str()));
	}

protected:
	TYPE _type = TYPE_NONE;
	std::string _show_name="";
	std::string _tooltip="";
	std::string _param="";
	int _prop_idx = -1;

protected:
	QHBoxLayout* _layout = nullptr;
	QSpinBox* _spin_box = nullptr;
};

struct ParamList
{
	std::vector<std::shared_ptr<ParamItemBase> > params;
	QScrollArea* container=nullptr;
};

enum SOURCE_TYPE
{
	SOURCE_TYPE_CAM,
	SOURCE_TYPE_PIC,
	SOURCE_TYPE_VIDEO
};

class NamaDemo_YXL;
typedef void(NamaDemo_YXL::*ParamUpdateEveryFrameFunc)();

class NamaDemo_YXL : public QWidget
{
	Q_OBJECT

private:

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

	void ButtonClicked(QObject* sender);

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
	ParamUpdateEveryFrameFunc _param_update_every_frame_func = nullptr;

private:
	std::shared_ptr<QButtonGroup> _sourceBtnGroup = nullptr;
	std::shared_ptr<QButtonGroup> _paramUpdateBtnGroup=nullptr;

private:
	std::string _save_img_path_format;
	int _save_img_cur_idx = 0;

private:
	//nama
	std::shared_ptr<FU::Nama> _nama=nullptr;

	cv::VideoWriter* _writer;
};
