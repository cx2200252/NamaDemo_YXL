#include "CmFile.h"
#include "NamaDemo_YXL.h"
#include <QButtonGroup>
#include <qmessagebox.h>
#include <qprogressdialog.h>
#include <qinputdialog.h>
#include <qtimer.h>
#include <qevent.h>
#include <YXLHelper.h>
#include <YXLJsonReader.h>
#include <qmenu.h>
#include <qspinbox.h>

#pragma comment( lib, CV_LIB("videoio"))

const std::string g_prop_postfixs[] = {"*.bundle", "*.mp3"};

//#define _HAIR_PARAM_
#define _HAIR_BUST_PARAM_
//#define _PORTRAIT_LIGHTING_PARAM_

#ifdef _WIN64
std::string g_resDir = "../../resources/";
#else
std::string g_resDir = "../resources/";
#endif

CStr g_saveDir = "./saved/";
const cv::Size g_video_size(1280, 720);

QString StdStr2QStr(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}

std::string QStr2StdStr(QString str)
{
	return (std::string)str.toLocal8Bit();
}

void ParamItemBase::InitCtrl(NamaDemo_YXL * wnd)
{
	_layout = new QHBoxLayout();
	_v_layout = new QVBoxLayout();

	_spin_box = new QSpinBox();
	_spin_box->setToolTip(QString::fromLocal8Bit("目标道具"));
	_spin_box->setSingleStep(1);
	_spin_box->setRange(0, 0);
	_spin_box->setValue(0);
	_spin_box->setMaximumWidth(50);
	_v_layout->addWidget(_spin_box);
	_v_layout->addStretch();
	_layout->addLayout(_v_layout);
	QObject::connect(_spin_box, SIGNAL(valueChanged(int)), wnd, SLOT(SpinBoxChanged(int)));
}

struct ParamItemCheckbox :public ParamItemBase
{
public:
	ParamItemCheckbox() :ParamItemBase(TYPE_CHECKBOX) {}

	static bool IsType(const rapidjson::Value & val)
	{
		return ParamItemBase::IsType(val) && "checkbox" == JsonGetStr(val["type"]) && JsonValHasMemberAndIsBool(val, "val");
	}
	virtual void LoadFromJson(const rapidjson::Value & val)
	{
		ParamItemBase::LoadFromJson(val);
		_val = JsonGetBool(val["val"]);
	}
	virtual void SaveToJson(rapidjson::Value & val, rapidjson::Document& doc)
	{
		auto& alloc = doc.GetAllocator();
		val.AddMember(JsonParseStr("type", doc), JsonParseStr("checkbox", doc), alloc);
		ParamItemBase::SaveToJson(val, doc);
		val.AddMember(JsonParseStr("val", doc), _val, alloc);
	}
	virtual void InitCtrl(NamaDemo_YXL* wnd)
	{
		ParamItemBase::InitCtrl(wnd);

		_check_box = new QCheckBox(StdStr2QStr(_show_name));
		_check_box->setChecked(_val);

		QObject::connect(_check_box, SIGNAL(clicked()), wnd, SLOT(CheckBoxClick()));
		_layout->addWidget(_check_box);
		_layout->addStretch();

		if (_tooltip != "")
			_check_box->setToolTip(StdStr2QStr(_tooltip));
	}

	virtual void UpdateCtrlValue()
	{
		ParamItemBase::UpdateCtrlValue();
		SetCtrlData(_check_box, _val);
	}
	virtual void SetCtrlValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed)
	{
		double val = 0.0;
		if (propsUsed.size()>_prop_idx)
			val = nama->GetPropParameterD(propsUsed[_prop_idx], _param);
		_val = val >0.0;
	}
	virtual bool SetPropValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed, QObject* sender)
	{
		if (sender != _check_box)
			return false;
		_val = _check_box->isChecked();
		if (nama && _prop_idx >= 0 && _prop_idx < propsUsed.size())
			nama->SetPropParameter(propsUsed[_prop_idx], _param, _val ? 1.0 : 0.0);
		return true;
	}

protected:
	bool _val = false;

protected:
	QCheckBox* _check_box = nullptr;
};

struct ParamItemSlider :public ParamItemBase
{
public:
	ParamItemSlider() :ParamItemBase(TYPE_SLIDER) {}

	static bool IsType(const rapidjson::Value & val)
	{
		return ParamItemBase::IsType(val) && "slider" == JsonGetStr(val["type"]) 
			&& JsonValHasMemberAndIsInt(val, "val") 
			&& JsonValHasMemberAndIsFloat(val, "scale") 
			&& JsonValHasMemberAndIsIntVec(val, "range") && val["range"].Size()==2;
	}
	virtual void LoadFromJson(const rapidjson::Value & val)
	{
		ParamItemBase::LoadFromJson(val);
		_val = JsonGetInt(val["val"]);
		_scale = JsonGetFloat(val["scale"]);
		auto tmp = JsonGetIntVec(val["range"]);
		_range = std::make_pair(tmp[0], tmp[1]);
	}
	virtual void SaveToJson(rapidjson::Value & val, rapidjson::Document& doc)
	{
		auto& alloc = doc.GetAllocator();
		val.AddMember(JsonParseStr("type", doc), JsonParseStr("slider", doc), alloc);
		ParamItemBase::SaveToJson(val, doc);
		val.AddMember(JsonParseStr("val", doc), _val, alloc);
		val.AddMember(JsonParseStr("scale", doc), _scale, alloc);
		std::vector<int> tmp({ _range.first, _range.second });
		val.AddMember(JsonParseStr("range", doc), JsonParseIntVec(tmp, doc), alloc);
	}

	virtual void InitCtrl(NamaDemo_YXL* wnd)
	{
		ParamItemBase::InitCtrl(wnd);

		_label_name = new QLabel(StdStr2QStr(_show_name));
		_layout->addWidget(_label_name);

		_slider = new QSlider();
		_slider->setRange(_range.first, _range.second);
		_slider->setValue(0);
		_slider->setOrientation(Qt::Orientation::Horizontal);
		_slider->setMinimumHeight(15);
		QObject::connect(_slider, SIGNAL(valueChanged(int)), wnd, SLOT(SliderChanged(int)));
		_layout->addWidget(_slider);

		_label = new QLabel();
		_label->setText(StdStr2QStr("0"));
		_label->setMinimumWidth(30);
		_label->setMaximumWidth(30);
		_label->setAlignment(Qt::AlignRight);
		_layout->addWidget(_label);

		if (_tooltip != "")
		{
			_slider->setToolTip(StdStr2QStr(_tooltip));
			_label->setToolTip(StdStr2QStr(_tooltip));
		}
	}
	virtual void UpdateCtrlValue()
	{
		ParamItemBase::UpdateCtrlValue();
		SetCtrlData(_slider, _val);
	}
	virtual void SetCtrlValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed)
	{
		double val = 0.0;
		if (propsUsed.size()>_prop_idx)
			val = nama->GetPropParameterD(propsUsed[_prop_idx], _param);
		_val = val / _scale;
	}
	virtual bool SetPropValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed, QObject* sender)
	{
		if (sender != _slider)
			return false;
		_val = _slider->value();
		auto tmp = _val*_scale;
		_label->setText(QString::number(tmp));
		if (nama && _prop_idx >= 0 && _prop_idx < propsUsed.size())
			nama->SetPropParameter(propsUsed[_prop_idx], _param, tmp);
		return true;
	}

protected:
	int _val = 0;
	std::pair<int, int> _range;
	float _scale = 0.0f;


protected:
	QSlider* _slider = nullptr;
	QLabel* _label_name = nullptr;
	QLabel* _label = nullptr;
};

struct ParamItemComboBox :public ParamItemBase
{
public:
	ParamItemComboBox() :ParamItemBase(TYPE_COMBOBOX) {}

	static bool IsType(const rapidjson::Value & val)
	{
		return ParamItemBase::IsType(val) && "combobox" == JsonGetStr(val["type"]) 
			&& JsonValHasMemberAndIsStr(val, "val") 
			&& JsonValHasMemberAndIsStrVec(val, "combo_texts");
	}
	virtual void LoadFromJson(const rapidjson::Value & val)
	{
		ParamItemBase::LoadFromJson(val);
		_val = JsonGetStr(val["val"]);
		_combo_texts = JsonGetStrVec(val["combo_texts"]);
	}
	virtual void SaveToJson(rapidjson::Value & val, rapidjson::Document& doc)
	{
		auto& alloc = doc.GetAllocator();
		val.AddMember(JsonParseStr("type", doc), JsonParseStr("combobox", doc), alloc);
		ParamItemBase::SaveToJson(val, doc);
		val.AddMember(JsonParseStr("val", doc), JsonParseStr(_val, doc), alloc);
		val.AddMember(JsonParseStr("combo_texts", doc), JsonParseVecStr(_combo_texts, doc), alloc);
	}

	virtual void InitCtrl(NamaDemo_YXL* wnd)
	{
		ParamItemBase::InitCtrl(wnd);

		_label_name = new QLabel(StdStr2QStr(_show_name));
		_layout->addWidget(_label_name);

		_combobox = new QComboBox();
		QObject::connect(_combobox, SIGNAL(currentTextChanged(QString)), wnd, SLOT(CurrentTextChanged(QString)));
		QStringList sl;
		auto backup_str = _val;
		for (auto& txt : _combo_texts)
			sl << StdStr2QStr(txt);
		_combobox->addItems(sl);
		_combobox->setCurrentText(StdStr2QStr(backup_str));
		_layout->addWidget(_combobox);
		_layout->addStretch();

		if (_tooltip != "")
			_combobox->setToolTip(StdStr2QStr(_tooltip));
	}

	virtual void UpdateCtrlValue()
	{
		ParamItemBase::UpdateCtrlValue();
		SetCtrlData(_combobox, _val);
	}

	virtual void SetCtrlValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed)
	{
		std::string str = "";
		if (propsUsed.size()>_prop_idx)
			str = nama->GetPropParameterStr(propsUsed[_prop_idx], _param);
		if ("" != str)
			_val = str;
	}
	virtual bool SetPropValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed, QObject* sender)
	{
		if (sender != _combobox)
			return false;
		_val = QStr2StdStr(_combobox->currentText());
		if (nama && _prop_idx >= 0 && _prop_idx < propsUsed.size())
			nama->SetPropParameter(propsUsed[_prop_idx], _param, _val);
		return true;
	}
protected:
	std::string _val;
	std::vector<std::string> _combo_texts;

protected:
	QComboBox* _combobox = nullptr;
	QLabel* _label_name = nullptr;
};

struct ParamItemSliderList :public ParamItemBase
{
public:
	ParamItemSliderList() :ParamItemBase(TYPE_SLIDER_LIST) {}

	static bool IsType(const rapidjson::Value & val)
	{
		return ParamItemBase::IsType(val) && "slider_list" == JsonGetStr(val["type"]) 
			&& JsonValHasMemberAndIsIntVec(val, "vals")
			&& JsonValHasMemberAndIsIntVec(val, "min")
			&& JsonValHasMemberAndIsIntVec(val, "max")
			&& JsonValHasMemberAndIsFloatVec(val, "scales")
			&& JsonValHasMemberAndIsStrVec(val, "names");
	}
	virtual void LoadFromJson(const rapidjson::Value & val)
	{
		ParamItemBase::LoadFromJson(val);
		_vals = JsonGetIntVec(val["vals"]);
		_min = JsonGetIntVec(val["min"]);
		_max = JsonGetIntVec(val["max"]);
		_scales = JsonGetFloatVec(val["scales"]);
		_names = JsonGetStrVec(val["names"]);
	}
	virtual void SaveToJson(rapidjson::Value & val, rapidjson::Document& doc)
	{
		auto& alloc = doc.GetAllocator();
		val.AddMember(JsonParseStr("type", doc), JsonParseStr("slider_list", doc), alloc);
		ParamItemBase::SaveToJson(val, doc);
		val.AddMember(JsonParseStr("vals", doc), JsonParseIntVec(_vals, doc), alloc);
		val.AddMember(JsonParseStr("min", doc), JsonParseIntVec(_min, doc), alloc);
		val.AddMember(JsonParseStr("max", doc), JsonParseIntVec(_max, doc), alloc);
		val.AddMember(JsonParseStr("scales", doc), JsonParseFloatVec(_scales, doc), alloc);
		val.AddMember(JsonParseStr("names", doc), JsonParseVecStr(_names, doc), alloc);
	}

	virtual void InitCtrl(NamaDemo_YXL* wnd)
	{
		CV_Assert(_vals.size() == _min.size() && _vals.size() == _max.size() && _vals.size() == _scales.size() && _vals.size() == _names.size());
		ParamItemBase::InitCtrl(wnd);

		_label_name = new QLabel(StdStr2QStr(_show_name));
		_vert_layout_left = new QVBoxLayout();
		_vert_layout_left->addWidget(_label_name);
		_vert_layout_left->addStretch();
		_vert_layout_right = new QVBoxLayout();
		for (int i(0); i != _vals.size(); ++i)
		{
			auto layout = new QHBoxLayout();
			_hori_layouts.push_back(layout);
			auto slider = new QSlider();
			_sliders.push_back(slider);
			auto label = new QLabel();
			_labels.push_back(label);
			auto label_name = new QLabel(StdStr2QStr(_names[i]));
			_label_names.push_back(label_name);

			layout->addWidget(label_name);

			slider->setRange(_min[i], _max[i]);
			slider->setValue(0);
			slider->setOrientation(Qt::Orientation::Horizontal);
			slider->setMinimumHeight(15);
			QObject::connect(slider, SIGNAL(valueChanged(int)), wnd, SLOT(SliderChanged(int)));
			layout->addWidget(slider);

			label->setText(StdStr2QStr("0"));
			label->setMinimumWidth(30);
			label->setMaximumWidth(30);
			label->setAlignment(Qt::AlignRight);
			layout->addWidget(label);

			_vert_layout_right->addLayout(layout);
		}
		_layout->addLayout(_vert_layout_left);
		_layout->addLayout(_vert_layout_right);

	}

	virtual void UpdateCtrlValue()
	{
		_is_updating = true;
		ParamItemBase::UpdateCtrlValue();
		for (int i(0); i != _vals.size(); ++i)
		{
			if (i + 1 == _vals.size())
				_is_updating = false;
			SetCtrlData(_sliders[i], _vals[i]);
		}
	}

	virtual void SetCtrlValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed)
	{
		if (_is_updating)
			return;
		std::vector<double> tmp(_vals.size());
		bool ret=false;
		if (propsUsed.size() > _prop_idx)
			ret=nama->GetPropParameterDv(propsUsed[_prop_idx], _param, tmp);
		if (ret)
			for (int i(0); i != _vals.size(); ++i)
				_vals[i] = static_cast<float>(tmp[i]) / _scales[i];
	}
	virtual bool SetPropValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed, QObject* sender)
	{
		bool is_find(false);
		for(auto& slider:_sliders)
			if (sender == slider)
			{
				is_find = true;
				break;
			}
		if (is_find == false)
			return false;
		if (_is_updating)
			return true;

		std::vector<double> nama_vals;
		for (int i(0); i != _vals.size(); ++i)
		{
			_vals[i] = _sliders[i]->value();
			auto tmp = _vals[i]*_scales[i];
			_labels[i]->setText(QString::number(tmp));
			nama_vals.push_back(tmp);
		}
		if (nama && _prop_idx >= 0 && _prop_idx < propsUsed.size())
			nama->SetPropParameter(propsUsed[_prop_idx], _param, nama_vals);
		return true;
	}

protected:
	std::vector<int> _vals, _min, _max;
	std::vector<float> _scales;
	std::vector<std::string> _names;

protected:
	QLabel* _label_name = nullptr;
	QVBoxLayout* _vert_layout_left = nullptr;
	QVBoxLayout* _vert_layout_right = nullptr;

	std::vector<QHBoxLayout*> _hori_layouts;
	std::vector<QSlider*> _sliders;
	std::vector<QLabel*> _labels;
	std::vector<QLabel*> _label_names;
	
	bool _is_updating = false;
};

struct ParamItemHLine :public ParamItemBase
{
public:
	ParamItemHLine() :ParamItemBase(TYPE_HORIZONAL_LINE) {}

	static bool IsType(const rapidjson::Value & val)
	{
		return val.IsObject()&&JsonValHasMemberAndIsStr(val, "type")&& JsonGetStr(val["type"])=="h_line";
	}
	virtual void LoadFromJson(const rapidjson::Value & val)
	{
		
	}
	virtual void SaveToJson(rapidjson::Value & val, rapidjson::Document& doc)
	{
		auto& alloc = doc.GetAllocator();
		val.AddMember(JsonParseStr("type", doc), JsonParseStr("h_line", doc), alloc);
	}
	virtual void InitCtrl(NamaDemo_YXL* wnd)
	{
		_layout = new QHBoxLayout();

		auto line = new QFrame();
		line->setFrameShape(QFrame::HLine);
		line->setFrameShadow(QFrame::Sunken);
		_layout->addWidget(line);
	}

	virtual void UpdateCtrlValue()
	{
	}
	virtual void SetCtrlValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed)
	{
	}
	virtual bool SetPropValue(std::shared_ptr<FU::Nama> nama, std::vector<std::string>& propsUsed, QObject* sender)
	{
		return false;
	}

protected:

protected:
};


namespace YXL
{
	namespace JSON
	{
		template<>
		struct ValueGetter<std::shared_ptr<ParamItemBase> > {
			static std::shared_ptr<ParamItemBase> Get(const rapidjson::Value & val) {
				std::shared_ptr<ParamItemBase> ret = nullptr;
				auto type = GetType(val);
				switch (type)
				{
				case ParamItemBase::TYPE_CHECKBOX:
					ret = std::shared_ptr<ParamItemBase>(new ParamItemCheckbox);
					break;
				case ParamItemBase::TYPE_SLIDER:
					ret = std::shared_ptr<ParamItemBase>(new ParamItemSlider);
					break;
				case ParamItemBase::TYPE_COMBOBOX:
					ret = std::shared_ptr<ParamItemBase>(new ParamItemComboBox);
					break;
				case ParamItemBase::TYPE_SLIDER_LIST:
					ret = std::shared_ptr<ParamItemBase>(new ParamItemSliderList);
					break;
				case ParamItemBase::TYPE_HORIZONAL_LINE:
					ret = std::shared_ptr<ParamItemBase>(new ParamItemHLine);
					break;
				default:
					break;
				}
				if (ret)
					ret->LoadFromJson(val);
				return ret;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				auto type = GetType(val);
				switch (type)
				{
				case ParamItemBase::TYPE_CHECKBOX:
					return ParamItemCheckbox::IsType(val);
				case ParamItemBase::TYPE_SLIDER:
					return ParamItemSlider::IsType(val);
				case ParamItemBase::TYPE_COMBOBOX:
					return ParamItemComboBox::IsType(val);
				case ParamItemBase::TYPE_SLIDER_LIST:
					return ParamItemSliderList::IsType(val);
				case ParamItemBase::TYPE_HORIZONAL_LINE:
					return ParamItemHLine::IsType(val);
				default:
					break;
				}
				return false;
			}
			static ParamItemBase::TYPE GetType(const rapidjson::Value & val)
			{
				if (val.HasMember("type") && val["type"].IsString())
				{
					std::string name = ValueGetter<std::string>::Get(val["type"]);
					if ("checkbox" == name)
						return ParamItemBase::TYPE_CHECKBOX;
					else if ("slider" == name)
						return ParamItemBase::TYPE_SLIDER;
					else if ("combobox" == name)
						return ParamItemBase::TYPE_COMBOBOX;
					else if ("slider_list" == name)
						return ParamItemBase::TYPE_SLIDER_LIST;
					else if ("h_line" == name)
						return ParamItemBase::TYPE_HORIZONAL_LINE;
				}
				return ParamItemBase::TYPE_NONE;
			}
		};

		template<>
		struct ValueParser<std::shared_ptr<ParamItemBase> > {
			static rapidjson::Value Parse(const std::shared_ptr<ParamItemBase>& val, rapidjson::Document& doc) {
				rapidjson::Value v(rapidjson::Type::kObjectType);
				CV_Assert(val != nullptr);
				val->SaveToJson(v, doc);
				return v;
			}
		};
	}
}

class Config
{
public:
	Config()
	{
		_json = std::shared_ptr<YXL::JSON::Json>(new YXL::JSON::Json);
		_json->Load(_config_path);

		_json_ctrl = std::shared_ptr<YXL::JSON::Json>(new YXL::JSON::Json);
		_json_ctrl->Load(_config_ctrl_path);
	}
	template<class type> void Set(CStr& name, type val, bool is_save = false)
	{
		_json->SetMember<type>(name, val);
		if (is_save)
			Save();
	}
	template<class type> void Set(CStr& name, std::vector<type> vals, bool is_save = false)
	{
		_json->SetMember<type>(name, vals);
		if (is_save)
			Save();
	}
	template<class type> type Get(CStr& name, type def_val)
	{
		return _json->ReadValue(name, def_val);
	}
	template<class type> void Get(CStr& name, std::vector<type>& ret, type def_val)
	{
		_json->ReadValue(ret, name);
	}
	void LoadParamList(std::map<std::string, ParamList >& param_lists)
	{
		auto& root = _json_ctrl->GetRoot();
		if (root.HasMember("params") && root["params"].IsObject())
			for (auto iter = root["params"].MemberBegin(); iter != root["params"].MemberEnd(); ++iter)
			{
				std::string name = iter->name.GetString();
				if (iter->value.IsArray())
				{
					std::vector<std::shared_ptr<ParamItemBase> > params;
					_json_ctrl->ReadValue(params, name, root["params"]);
					for (auto& item : params)
						if (item)
							param_lists[name].params.push_back(item);
				}
		
			}
	}
	void SaveParamList(std::map<std::string, ParamList >& param_lists, bool is_save = false)
	{
		auto& doc = _json_ctrl->GetDoc();
		rapidjson::Value v(rapidjson::Type::kObjectType);
		for (auto& pair : param_lists)
		{
			_json_ctrl->AddMember(pair.first, pair.second.params, v);
		}
		_json_ctrl->SetMember(std::string("params"), v);
		if (is_save)
			Save();
	}

	void Save()
	{
		_json->Save(_config_path);
		_json_ctrl->Save(_config_ctrl_path);
	}

private:
	std::string _config_path = "config.json";
	std::string _config_ctrl_path = g_resDir+"ctrl_config.json";
	std::shared_ptr<YXL::JSON::Json> _json;
	std::shared_ptr<YXL::JSON::Json> _json_ctrl;
};
std::shared_ptr<Config> g_config(new Config);

NamaDemo_YXL::NamaDemo_YXL(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	CmFile::MkDir(g_saveDir);

	LoadParams();

	ui.listWidget_props_used->SetParent(this);

	//group up radio buttons
	{
		_sourceBtnGroup = std::shared_ptr<QButtonGroup>(new QButtonGroup(this));
		_sourceBtnGroup->addButton(ui.radioButton_sourceCam);
		_sourceBtnGroup->addButton(ui.radioButton_sourcePic);
		_sourceBtnGroup->addButton(ui.radioButton_sourceVideo);
	}
	{
		_paramUpdateBtnGroup = std::shared_ptr<QButtonGroup>(new QButtonGroup(this));
		_paramUpdateBtnGroup->addButton(ui.radioButton_update_every_frame_none);
		_paramUpdateBtnGroup->addButton(ui.radioButton_update_every_frame_read);
		_paramUpdateBtnGroup->addButton(ui.radioButton_update_every_frame_write);
	}

	ui.openGLWidget->SetMainWnd(this);

	int page = g_config->Get("init_page", 0);
	ui.toolBox->setCurrentIndex(page);
	bool is_check = g_config->Get("save_with_src_img", false);
	ui.checkBox_saveWithSrcImg->setChecked(is_check);

	{
		/*QRadioButton* btns[] = { ui.radioButton_update_every_frame_none, ui.radioButton_update_every_frame_write, ui.radioButton_update_every_frame_read};
		int update_every_frame_type = g_config->Get("update_every_frame_type", 0);
		btns[update_every_frame_type]->setChecked(true);
		ButtonClicked(btns[update_every_frame_type]);*/

		ui.radioButton_update_every_frame_none->setChecked(true);
		ButtonClicked(ui.radioButton_update_every_frame_none);
	}

	//SetParamRelatedButtonState();

	InitCtrls();
	LoadProps();
}

NamaDemo_YXL::~NamaDemo_YXL()
{
	g_config->Set("init_page", (int)ui.toolBox->currentIndex());
	g_config->Set("save_with_src_img", (bool)ui.checkBox_saveWithSrcImg->isChecked());
	g_config->Save();
}

cv::Mat NamaDemo_YXL::GetNextFrame()
{
	cv::Mat src = GetSourceImage();
	if (src.empty())
		return cv::Mat();

	cv::Mat nama_out;
	if (_nama)
	{
		if (_param_update_every_frame_func)
			(this->*_param_update_every_frame_func)();
		nama_out = _nama->Process(src);
	}
	else
		return src;

	cv::Mat src2 = _nama->ToNamaIn(src);

	cv::Mat show_img = GetShowImage(src2, nama_out);

	if ("" != _save_img_path_format)
	{
		int cnt_to_save = ui.spinBox_savePic_cnt->value();
		if (cnt_to_save > 0)
		{
			cv::Mat save_img = GetSaveImage(src2, nama_out);
			if (save_img.empty() == false)
			{
				cv::imwrite(cv::format(_save_img_path_format.c_str(), _save_img_cur_idx), save_img);
				++_save_img_cur_idx;
				--cnt_to_save;
				ui.spinBox_savePic_cnt->setValue(cnt_to_save);
			}
		}
		else
		{
			ui.pushButton_savePic_stop->click();
		}
	}

	//saving post-process

	return show_img;
}

cv::Mat NamaDemo_YXL::GetSourceImage()
{
	cv::Mat img;
	switch (_source_type)
	{
	case SOURCE_TYPE_PIC:
		if (false == _pic.empty())
			img = _pic.clone();
		break;
	case SOURCE_TYPE_VIDEO:
		if (_cap_video)
		{
			*_cap_video >> img;
			if (img.empty() && CmFile::FileExist(_path_video))
			{
				_cap_video->open(_path_video);
				*_cap_video >> img;
				
			}
		}
		break;
	}

	if (img.empty())
	{
		if (nullptr == _cap_cam)
		{
			_cap_cam = std::shared_ptr<cv::VideoCapture>(new cv::VideoCapture(0));
			if (false == _cap_cam->isOpened())
			{
				_cap_cam = nullptr;
				return img;
			}
			else
			{
				_cap_cam->set(CV_CAP_PROP_FRAME_WIDTH, 1280);
				_cap_cam->set(CV_CAP_PROP_FRAME_HEIGHT, 720);
			}
		}
		if (_cap_cam)
		{
			*_cap_cam >> img;
			if (false == img.empty())
				cv::flip(img, img, 1);
		}
	}
	if (img.empty()==false&& img.channels() == 3)
		cv::cvtColor(img, img, CV_BGR2BGRA);

	return img;
}

cv::Mat NamaDemo_YXL::GetShowImage(cv::Mat src, cv::Mat nama_out)
{
	const cv::Size target_size(1280, 720);
	const int w0 = nama_out.cols*target_size.height / nama_out.rows;
	//if same aspect, only show nama result
	if (w0 == target_size.width)
		return nama_out;

	const int w1 = src.cols*target_size.height / src.rows;

	float scale = 1.0f;
	if (w0 + w1 > target_size.width)
		scale = (float)target_size.width / (w0 + w1);
	
	cv::Size s0(w0*scale, target_size.height*scale);
	cv::Size s1(w1*scale, target_size.height*scale);
	cv::Mat img0, img1;
	cv::resize(nama_out, img0, s0);
	cv::resize(src, img1, s1);

	cv::Mat res(target_size, nama_out.type(), cv::Scalar::all(127));

	int off0 = (res.rows - img0.rows) / 2;
	int off1 = (res.rows - img1.rows) / 2;
	int off = (res.cols - img0.cols - img1.cols) / 4;

	img0.copyTo(res.colRange(off, off+img0.cols).rowRange(off0, off0 + img0.rows));
	img1.copyTo(res.colRange(res.cols-img1.cols- off, res.cols-off).rowRange(off1, off1 + img1.rows));

	return res;
}

cv::Mat NamaDemo_YXL::GetSaveImage(cv::Mat src, cv::Mat nama_out)
{
	bool need_src = ui.checkBox_saveWithSrcImg->isChecked();
	if (need_src)
	{
		const int h = std::min(src.rows, nama_out.rows);
		cv::Size s0(nama_out.cols*h / nama_out.rows, h);
		cv::Size s1(src.cols*h / src.rows, h);

		cv::Mat res(h, s0.width + s1.width, CV_8UC4);

		cv::Mat img0, img1;
		cv::resize(nama_out, img0, s0);
		cv::resize(src, img1, s1);
		img1.copyTo(res.colRange(0, img0.cols));
		img0.copyTo(res.colRange(img0.cols, img0.cols+img1.cols));
		return res;
	}
	else
		return nama_out;
}

void NamaDemo_YXL::InitCtrls()
{
	//params
	//push_btn
	{
		QStringList strs;
		for (auto& param:_param_lists)
		{
			strs << StdStr2QStr(param.first);
		}
		ui.comboBox_param_list->addItems(strs);
		ui.comboBox_param_list->setCurrentText(StdStr2QStr(_cur_param_list));
		connect(ui.comboBox_param_list, SIGNAL(currentTextChanged(QString)), this, SLOT(CurrentTextChanged(QString)));
	}
	//ctrl
	{
		for (auto& param : _param_lists)
		{
			QFormLayout* layout = new QFormLayout();
			bool is_show = (param.first == _cur_param_list);
			for (auto& item : param.second.params)
			{
				item->InitCtrl(this);
				layout->addRow(StdStr2QStr(""), item->GetLayout());
			}

			param.second.container = new QScrollArea();

			QWidget* qw = new QWidget(param.second.container);
			qw->setLayout(layout);
			param.second.container->setWidget(qw);

			qw->setMinimumWidth(ui.toolBox->width() - 50);

			ui.verticalLayout_2->addWidget(param.second.container);
			param.second.container->setVisible(is_show);
		}

		
	}
}

void NamaDemo_YXL::InitNama()
{
	_nama = std::shared_ptr<FU::Nama>(new FU::Nama(g_resDir));
	_nama->Init(g_resDir + "v3.bundle");
	_nama->InitArdataExt(g_resDir + "ardata_ex.bundle");

	InitFromConfig();

	int val = g_config->Get("max_face", 1);
	ui.spinBox_max_face->setValue(val);

	int is_nama_in_bgra = g_config->Get("is_nama_in_bgra", false);
	SetCtrlData(ui.checkBox_nama_in_bgra, is_nama_in_bgra);
	int is_nama_out_bgra = g_config->Get("is_nama_out_bgra", false);
	SetCtrlData(ui.checkBox_nama_out_bgra, is_nama_out_bgra);

	int is_nama_in_vertical = g_config->Get("is_nama_in_vertical", false);
	SetCtrlData(ui.checkBox_nama_in_vertical, is_nama_in_vertical);
	int is_nama_out_vertical = g_config->Get("is_nama_out_vertical", false);
	SetCtrlData(ui.checkBox_nama_out_vertical, is_nama_out_vertical);

	auto source = g_config->Get<std::string>("source", "camera");
	if ("camera" == source)
		SetCtrlData(ui.radioButton_sourceCam, true);
	else if ("picture" == source)
		SetCtrlData(ui.radioButton_sourcePic, true);
	else if ("video" == source)
		SetCtrlData(ui.radioButton_sourceVideo, true);

	auto path = g_config->Get<std::string>("path_pic", "");
	UseSourcePicture(path);
	path = g_config->Get<std::string>("path_video", "");
	UseSourceVideo(path);
}

void NamaDemo_YXL::keyPressEvent(QKeyEvent * keyEvent)
{
	if (keyEvent->modifiers() == Qt::ControlModifier)
		_is_ctrl_down=true;
	if (keyEvent->modifiers() == Qt::AltModifier)
		_is_alt_down = true;
	if (keyEvent->modifiers() == Qt::ShiftModifier)
		_is_shift_down = true;

	if (_is_alt_down)
	{
		std::string msg="";
		if (keyEvent->key() == Qt::Key::Key_S)
		{
			ui.openGLWidget->Stop(!ui.openGLWidget->IsStop());
		}
		if ("" != msg)
		{
			QMessageBox::information(this, StdStr2QStr("NamaDemo_YXL"), StdStr2QStr(msg), QMessageBox::Ok);
		}
	}
	if (_is_ctrl_down)
	{
		if (keyEvent->key() == Qt::Key::Key_S)
		{
			ui.pushButton_savePic_stop->click();
			if (ui.pushButton_savePic_start->isEnabled())
			{
				ui.spinBox_savePic_cnt->setValue(1);
				ButtonClicked(ui.pushButton_savePic_start);
			}
		}
	}

	return QWidget::keyPressEvent(keyEvent);
}

void NamaDemo_YXL::keyReleaseEvent(QKeyEvent * keyEvent)
{
	if (keyEvent->modifiers() == Qt::ControlModifier || keyEvent->key() == Qt::Key::Key_Control)
		_is_ctrl_down = false;
	if (keyEvent->modifiers() == Qt::AltModifier || keyEvent->key() == Qt::Key::Key_Alt)
		_is_alt_down = false;
	if (keyEvent->modifiers() == Qt::ShiftModifier || keyEvent->key() == Qt::Key::Key_Shift)
		_is_shift_down = false;
	return QWidget::keyReleaseEvent(keyEvent);
}

//nama relative, must call in InitNama()
void NamaDemo_YXL::InitFromConfig()
{
	_is_param_batch_update = true;
	std::vector<std::string> tmp(_propsUsed.begin(), _propsUsed.end());
	_propsUsed.clear();
	for (auto& prop : tmp)
		AddPropUsed(prop);

	_nama->SetPropsUsed(_propsUsed);
	UpdateSpinBoxRange();

	UpdateCtrlValue();
	_is_param_batch_update = false;
}

void NamaDemo_YXL::LoadProps()
{
	CStr propDir = g_resDir;
	vecS dirs;
	CmFile::GetSubFolders(propDir, dirs);
	for (auto dir : dirs)
	{
		vecS names, tmp;
		for (auto postfix : g_prop_postfixs)
		{
			CmFile::GetNames(propDir + dir + "/" + postfix, tmp);
			for (auto a : tmp)
				names.push_back(propDir + dir + "/" + a);
		}
		if (names.empty())
			continue;

		auto* parent_item = new QTreeWidgetItem(ui.treeWidget_props);
		parent_item->setText(0, StdStr2QStr(dir));
		//parent_item->setData(1, Qt::ItemDataRole::UserRole, QVariant(0));
		//parent_item->setText(0, _train_results[i].show_title);
		
		std::sort(names.begin(), names.end(), [](const std::string& a, const std::string& b) {
			auto aa = YXL::GetFileInfo(a, YXL::FileInfo_LastWriteTime);
			auto bb = YXL::GetFileInfo(b, YXL::FileInfo_LastWriteTime);
			if (aa.first != bb.first)
				return aa.first > bb.first;
			else
				return aa.second > bb.second;
		});

		for (auto name : names)
		{
			auto* item = new QTreeWidgetItem(parent_item);
			item->setText(0, StdStr2QStr(CmFile::GetName(name)));
		}
	}
}

//ignore _resDir prefix
bool NamaDemo_YXL::IsValidPropPath(CStr & path)
{
	bool is_postfix_ok(false);
	for (auto postfix : g_prop_postfixs)
	{
		if (path.length()>=postfix.length() && path.substr(path.length() - postfix.length()+1, postfix.length()-1) == postfix.substr(1, postfix.length()-1))
		{
			is_postfix_ok = true;
			break;
		}
	}
	return is_postfix_ok && CmFile::FileExist(g_resDir +path);
}

void NamaDemo_YXL::ClearPropUsed()
{
	std::vector<QListWidgetItem*> items;
	for (int i(0); i != ui.listWidget_props_used->count(); ++i)
		items.push_back(ui.listWidget_props_used->item(i));

	for (auto&item : items)
		UsedPropsItemDoubleClicked(item);
}

void NamaDemo_YXL::PopPropUsed()
{
	const int cnt = ui.listWidget_props_used->count();
	if (cnt == 0)
		return;
	auto item = ui.listWidget_props_used->item(cnt - 1);
	UsedPropsItemDoubleClicked(item);
}

bool NamaDemo_YXL::AddPropUsed(CStr& path)
{
	if (false == IsValidPropPath(path))
		return false;
	int cnt = ui.listWidget_props_used->count();
	for (int i(0); i != cnt; ++i)
		if (path == ui.listWidget_props_used->item(i)->text().toStdString())
			return false;
	
	ui.listWidget_props_used->addItem(StdStr2QStr(path));

	auto iter=std::find(_propsUsed.begin(), _propsUsed.end(), path);
	if(iter==_propsUsed.end())
		_propsUsed.push_back(path);

	_nama->SetPropsUsed(_propsUsed);
	UpdateSpinBoxRange();
	g_config->Set<std::string>("props_used", _propsUsed, true);

	return true;
}

void NamaDemo_YXL::UpdateSpinBoxRange()
{
	int tmp = _propsUsed.empty() ? 0 : _propsUsed.size() - 1;
	for (auto& params : _param_lists)
		for (auto& item : params.second.params)
			item->SetSpinBoxRange(0, tmp);
}

void NamaDemo_YXL::UpdatePropsUsed()
{
	_is_param_batch_update = true;
	auto cnt = ui.listWidget_props_used->count();
	_propsUsed.clear();
	for (int i(0); i != cnt; ++i)
		_propsUsed.push_back(ui.listWidget_props_used->item(i)->text().toStdString());
	_nama->SetPropsUsed(_propsUsed);
	UpdateSpinBoxRange();
	_is_param_batch_update = false;
	SaveParams();
}

std::vector<char> buff;

void NamaDemo_YXL::UpdateCtrlValue()
{
	if (_nama && _is_shift_down && false== _propsUsed.empty())
	{
		std::string path = CmFile::BrowseFile("Bin (*.bin)\0*.bin\0All (*.*)\0*.*\0\0");
		if ("" != path)
		{
			YXL::LoadFileContentBinary(path, buff);
			//auto a = cv::getTickCount();
			//_nama->SetPropParameter(_propsUsed[0], "pr_data", &buff[0], buff.size());

			long long val = (long long)&buff[0];
			_nama->SetPropParameter(_propsUsed[0], "pr_data_ptr", val);
			_nama->SetPropParameter(_propsUsed[0], "pr_data_size", (long long)buff.size());

			//auto b = cv::getTickCount();
			//std::cout <<"set time: "<< (b - a) / cv::getTickFrequency() << std::endl;
		}
		_is_shift_down = false;
	}

	auto& param = _param_lists[_cur_param_list];
	for (auto& item : param.params)
		item->UpdateCtrlValue();
}

void NamaDemo_YXL::UseSourcePicture(CStr & path)
{
	if ("" != path)
	{
		auto img = cv::imread(path, -1);
		if (false == img.empty())
		{
			_path_pic = path;
			_pic = img;
			g_config->Set("path_pic", path);
			if (3 == _pic.channels())
				cv::cvtColor(_pic, _pic, CV_BGR2BGRA);
			ui.lineEdit_pathPic->setText(StdStr2QStr(_path_pic));
		}
	}
}

void NamaDemo_YXL::UseSourceVideo(CStr & path)
{
	if ("" != path)
	{
		auto cap = std::shared_ptr<cv::VideoCapture>(new cv::VideoCapture(path));
		if (cap->isOpened())
		{
			_cap_video = cap;
			_path_video = path;
			g_config->Set("path_video", _path_video);
			ui.lineEdit_pathVideo->setText(StdStr2QStr(_path_video));
		}
	}
}

//page 3
#include <funama.h>
void NamaDemo_YXL::SetAllParameter()
{
	_is_param_batch_update = true;
	UpdateCtrlValue();
	_is_param_batch_update = false;
	SaveParams();
}

void SaveObj(int prop_handle, bool is_ext, int face_id, const std::string& save_path)
{
	std::string param = (is_ext ? "ar_mesh_ext " : "ar_mesh ")+std::to_string(face_id);
	auto cnt = fuItemGetParamu8v(prop_handle, &param[0], nullptr, 0);
	if (cnt < 8)
		return;
	unsigned char* buff = new unsigned char[cnt];
	fuItemGetParamu8v(prop_handle, &param[0], (char*)buff, cnt);

	int n_vertex, n_triangle;
	{
		int* ptr = reinterpret_cast<int*>(buff);
		n_vertex = ptr[0];
		n_triangle = ptr[1];
	}
	if (n_vertex <= 0 || n_triangle <= 0)
		return;
	float* vertices = reinterpret_cast<float*>(buff + sizeof(int) * 2);
	unsigned short* uv = reinterpret_cast<unsigned short*>(buff + sizeof(int) * 2 + n_vertex * 3 * sizeof(float));
	unsigned short* tris = reinterpret_cast<unsigned short*>(buff + sizeof(int) * 2 + n_vertex * 4 * sizeof(float));
	cv::Mat a;
	a.at<cv::Vec4d>(1, 0);
	std::ofstream fout(save_path);
	for (int i(0); i != n_vertex; ++i)
		fout << "v " << vertices[3 * i] << " " << vertices[3 * i + 1] << " " << vertices[3 * i + 2] << std::endl;
	for (int i(0); i != n_vertex; ++i)
		fout << "vt " << float(uv[2 * i]) / 65535.f << " " << float(uv[2 * i + 1]) / 65535.f << std::endl;
	int order[] = { 2, 1, 0 };
	for (int i(0); i != n_triangle; ++i)
		fout << "f " << tris[3 * i + order[0]] + 1 <<"/"<< tris[3 * i + order[0]] + 1 << " " 
		<< tris[3 * i + order[1]] + 1 << "/" << tris[3 * i + order[1]] + 1 << " " 
		<< tris[3 * i + order[2]] + 1 << "/" << tris[3 * i + order[2]] + 1 << std::endl;

	fout.close();
}

void NamaDemo_YXL::UpdateParamsFromProp()
{
	if (!_nama)
		return;

	auto& param = _param_lists[_cur_param_list];
	for (auto& item : param.params)
		item->SetCtrlValue(_nama, _propsUsed);
	UpdateCtrlValue();
	SaveParams();
}

void NamaDemo_YXL::CheckBoxClick()
{
	auto obj = sender();
	for(auto& param:_param_lists)
		for (auto& item : param.second.params)
			if (item->SetPropValue(_nama, _propsUsed, obj))
			{
				if (false == _is_param_batch_update)
					SaveParams();
				return;
			}
}

void NamaDemo_YXL::SliderChanged(int val)
{
	auto obj = sender();

	for (auto& param : _param_lists)
		for (auto& item : param.second.params)
			if (item->SetPropValue(_nama, _propsUsed, obj))
			{
				if (false == _is_param_batch_update)
					SaveParams();
				return;
			}
}

void NamaDemo_YXL::SpinBoxChanged(int val)
{
	auto obj = this->sender();
	for (auto& param : _param_lists)
		for (auto& item : param.second.params)
			if (item->SpinBoxChanged(reinterpret_cast<QSpinBox*>(obj)))
			{
				if (false == _is_param_batch_update)
					SaveParams();
				return;
			}
}

void NamaDemo_YXL::ButtonClicked()
{
	const auto obj = this->sender();
	ButtonClicked(obj);
}

void NamaDemo_YXL::ValueChanged(int val)
{
	auto obj = this->sender();
	if (obj == ui.spinBox_max_face)
	{
		int val = ui.spinBox_max_face->value();
		g_config->Set("max_face", val, true);
		_nama->SetMaxFace(val);
	}
}

void NamaDemo_YXL::PropsItemDoubleClicked(QTreeWidgetItem * item, int col)
{
	std::string path;
	QTreeWidgetItem* cur = item;
	while (cur)
	{
		path = cur->text(0).toStdString() + (path!=""?"/":"")+ path;
		cur = cur->parent();
	}

	if (false == IsValidPropPath(path))
		return;

	if (_is_ctrl_down)
		ClearPropUsed();
	else if (_is_alt_down)
		PopPropUsed();

	AddPropUsed(path);


}

void NamaDemo_YXL::UsedPropsItemDoubleClicked(QListWidgetItem * item)
{
	ui.listWidget_props_used->removeItemWidget(item);
	delete item;
	UpdatePropsUsed();
}

void NamaDemo_YXL::CurrentTextChanged(QString str)
{
	auto obj = this->sender();
	auto txt = QStr2StdStr(str);
	if (obj == ui.comboBox_param_list)
	{
		auto iter = _param_lists.find(txt);
		if (iter != _param_lists.end())
		{
			_param_lists[_cur_param_list].container->setVisible(false);
			_cur_param_list = txt;
			_param_lists[_cur_param_list].container->setVisible(true);
			if (false==_is_param_batch_update)
				SaveParams();
			return;
		}
	}

	for (auto& param : _param_lists)
		for (auto& item : param.second.params)
			if (item->SetPropValue(_nama, _propsUsed, obj))
			{
				if (false == _is_param_batch_update)
					SaveParams();
				return;
			}
}

void NamaDemo_YXL::LoadParams()
{
	g_config->Get<std::string>("props_used", _propsUsed, "-1");

	g_config->LoadParamList(_param_lists);
	auto tmp= g_config->Get<std::string>("cur_param_list", "-1");
	_cur_param_list = _param_lists.find(tmp) != _param_lists.end() ? tmp : (_param_lists.empty()?"-1": _param_lists.begin()->first);
}

void NamaDemo_YXL::SaveParams()
{
	if (_propsUsed.empty())
	{
		std::string tmp[] = {"-1"};
		g_config->Set<std::string>("props_used", std::vector<std::string>(tmp, tmp+1));
	}
	else
		g_config->Set<std::string>("props_used", _propsUsed);

	g_config->SaveParamList(_param_lists);

	g_config->Set<std::string>("cur_param_list", _cur_param_list);

	g_config->Save();
}

void NamaDemo_YXL::ButtonClicked(QObject * obj)
{
	if (obj == ui.checkBox_nama_in_bgra)
	{
		auto tmp = ui.checkBox_nama_in_bgra->isChecked();
		if (_nama)
			_nama->SetBGRAIn(tmp);
		g_config->Set("is_nama_in_bgra", tmp, true);
	}
	else if (obj == ui.checkBox_nama_out_bgra)
	{
		auto tmp = ui.checkBox_nama_out_bgra->isChecked();
		if (_nama)
			_nama->SetBGRAOut(tmp);
		g_config->Set("is_nama_out_bgra", tmp, true);
	}
	else if (obj == ui.checkBox_nama_in_vertical)
	{
		auto tmp = ui.checkBox_nama_in_vertical->isChecked();
		if (_nama)
			_nama->SetVerticalIn(tmp);
		g_config->Set("is_nama_in_vertical", tmp, true);
	}
	else if (obj == ui.checkBox_nama_out_vertical)
	{
		auto tmp = ui.checkBox_nama_out_vertical->isChecked();
		if (_nama)
			_nama->SetVerticalOut(tmp);
		g_config->Set("is_nama_out_vertical", tmp, true);
	}
	else if (obj == ui.radioButton_sourceCam)
	{
		_source_type = SOURCE_TYPE_CAM;
		g_config->Set("source", "camera", true);
	}
	else if (obj == ui.radioButton_sourcePic)
	{
		_source_type = SOURCE_TYPE_PIC;
		g_config->Set("source", "picture", true);
		_cap_cam = nullptr;
	}
	else if (obj == ui.radioButton_sourceVideo)
	{
		_source_type = SOURCE_TYPE_VIDEO;
		g_config->Set("source", "video", true);
		_cap_cam = nullptr;
	}
	else if (obj == ui.pushButton_selPic)
	{
		auto str = CmFile::BrowseFile();
		UseSourcePicture(str);
	}
	else if (obj == ui.pushButton_selVideo)
	{
		auto str = CmFile::BrowseFile("Video (*.avi;*.mp4;*MOV)\0*.avi;*.mp4;*MOV\0All (*.*)\0*.*\0\0");
		UseSourceVideo(str);
	}
	else if (obj == ui.pushButton_saveDir)
	{
		CmFile::RunProgram("explorer", YXL::ToWindowsPath(g_saveDir), false, false);
	}
	else if (obj == ui.pushButton_savePic_start)
	{
		_save_img_cur_idx = 0;
		std::string time = YXL::GetCurTime();
		_save_img_path_format = g_saveDir + time + "_%06d.png";
		CmFile::MkDir(CmFile::GetFolder(_save_img_path_format));
		ui.pushButton_savePic_start->setEnabled(false);
		ui.pushButton_savePic_stop->setEnabled(true);
	}
	else if (obj == ui.pushButton_savePic_stop)
	{
		_save_img_path_format = "";
		ui.pushButton_savePic_start->setEnabled(true);
		ui.pushButton_savePic_stop->setEnabled(false);
	}
	else if (obj == ui.radioButton_update_every_frame_none)
	{
		//g_config->Set("update_every_frame_type", 0, true);
		_param_update_every_frame_func = nullptr;
	}
	else if (obj == ui.radioButton_update_every_frame_write)
	{
		//g_config->Set("update_every_frame_type", 1, true);
		_param_update_every_frame_func = &NamaDemo_YXL::SetAllParameter;
	}

	else if (obj == ui.radioButton_update_every_frame_read)
	{
		//g_config->Set("update_every_frame_type", 2, true);
		_param_update_every_frame_func = &NamaDemo_YXL::UpdateParamsFromProp;
	}
}

void NamaDemo_YXL::SetCtrlData(QAbstractSlider * slider, int value)
{
	if (slider->value() != value)
		slider->setValue(value);
	else
		slider->valueChanged(value);
}

void NamaDemo_YXL::SetCtrlData(QRadioButton * btn, bool isCheck)
{
	if (isCheck == btn->isChecked())
		btn->click();
	btn->click();
}

void NamaDemo_YXL::SetCtrlData(QCheckBox * btn, bool isCheck)
{
	//为了响应得调用click函数，值不变则调用两次
	if (isCheck == btn->isChecked())
		btn->click();
	btn->click();
}

void NamaDemo_YXL::SetCtrlData(QComboBox * comboBox, int idx)
{
	if (comboBox->currentIndex() != idx)
		comboBox->setCurrentIndex(idx);
	else
		comboBox->currentIndexChanged(idx);
}

void NamaDemo_YXL::SetCtrlData(QComboBox * comboBox, CStr & txt)
{
	comboBox->setCurrentText(StdStr2QStr(txt));
}


