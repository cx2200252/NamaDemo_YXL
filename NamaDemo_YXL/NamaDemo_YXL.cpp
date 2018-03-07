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



namespace YXL
{
	namespace JSON
	{
		template<>
		struct ValueGetter<ParamItem> {
			static ParamItem Get(const rapidjson::Value & val) {
				ParamItem param;
				param.type = GetType(val);
				if (ParamItem::TYPE_NONE == param.type)
					return ParamItem();
				param.prop_idx = val["prop_idx"].GetInt();
				param.param = ValueGetter<std::string>::Get(val["param"]);
				param.show_name = ValueGetter<std::string>::Get(val["show_name"]);
				if(val.HasMember("tooltip"))
					param.tooltip = ValueGetter<std::string>::Get(val["tooltip"]);

				switch (param.type)
				{
				case ParamItem::TYPE_CHECKBOX:
					param.valB = val["val"].GetBool();
					break;
				case ParamItem::TYPE_SLIDER:
				{
					param.valI = val["val"].GetInt();
					if (false == val["range"].IsArray() || val["range"].Size()!= 2)
						return ParamItem();
					auto iter = val["range"].Begin();
					int a = iter->GetInt();
					++iter;
					int b = iter->GetInt();
					param.range = std::make_pair(a, b);
					param.scale = val["scale"].GetFloat();
				}
					break;
				case ParamItem::TYPE_COMBOBOX:
				{
					param.valStr = ValueGetter<std::string>::Get(val["val"]);
					for (auto iter = val["combo_texts"].Begin(); iter != val["combo_texts"].End(); ++iter)
						param.combo_texts.push_back(ValueGetter<std::string>::Get(*iter));
				}
					break;
				default:
					break;
				}

				return param;
			}
			static bool IsType(const rapidjson::Value & val)
			{
				return val.IsObject() && ParamItem::TYPE_NONE!= GetType(val) && val.HasMember("param") && val["param"].IsString() && val.HasMember("prop_idx") && val["prop_idx"].IsInt();
			}
			static ParamItem::TYPE GetType(const rapidjson::Value & val)
			{
				if (val.HasMember("type") && val["type"].IsString())
				{
					std::string name = ValueGetter<std::string>::Get(val["type"]);
					if ("checkbox" == name)
						return ParamItem::TYPE_CHECKBOX;
					else if ("slider" == name)
						return ParamItem::TYPE_SLIDER;
					else if ("combobox" == name)
						return ParamItem::TYPE_COMBOBOX;
				}
				return ParamItem::TYPE_NONE;
			}
		};

		template<>
		struct ValueParser<ParamItem> {
			static rapidjson::Value Parse(const ParamItem& val, rapidjson::Document& doc) {
				rapidjson::Value v(rapidjson::Type::kObjectType);
				ValueParser<std::string> str_parser;
				
				rapidjson::Value name, value;
				name = str_parser.Parse("type", doc);
				value = str_parser.Parse(ToTypeStr(val.type), doc);
				v.AddMember(name, value, doc.GetAllocator());

				name = str_parser.Parse("show_name", doc);
				value = str_parser.Parse(val.show_name, doc);
				v.AddMember(name, value, doc.GetAllocator());

				name = str_parser.Parse("tooltip", doc);
				value = str_parser.Parse(val.tooltip, doc);
				v.AddMember(name, value, doc.GetAllocator());

				name = str_parser.Parse("param", doc);
				value = str_parser.Parse(val.param, doc);
				v.AddMember(name, value, doc.GetAllocator());

				name = str_parser.Parse("prop_idx", doc);
				v.AddMember(name, val.prop_idx, doc.GetAllocator());

				name = str_parser.Parse("val", doc);
				switch (val.type)
				{
				case ParamItem::TYPE_CHECKBOX:
				{
					v.AddMember(name, val.valB, doc.GetAllocator());
				}
					break;
				case ParamItem::TYPE_SLIDER:
				{
					v.AddMember(name, val.valI, doc.GetAllocator());

					name = str_parser.Parse("scale", doc);
					v.AddMember(name, val.scale, doc.GetAllocator());

					name = str_parser.Parse("range", doc);
					value = rapidjson::Value(rapidjson::Type::kArrayType);
					value.PushBack(val.range.first, doc.GetAllocator());
					value.PushBack(val.range.second, doc.GetAllocator());
					v.AddMember(name, value, doc.GetAllocator());
				}
					break;
				case ParamItem::TYPE_COMBOBOX:
				{
					value = str_parser.Parse(val.valStr, doc);
					v.AddMember(name, value, doc.GetAllocator());

					name = str_parser.Parse("combo_texts", doc);
					value = rapidjson::Value(rapidjson::Type::kArrayType);
					for (auto& txt : val.combo_texts)
						value.PushBack(str_parser.Parse(txt, doc), doc.GetAllocator());
					v.AddMember(name, value, doc.GetAllocator());
				}
					break;
				}

				return v;
			}

			static std::string ToTypeStr(ParamItem::TYPE type)
			{
				std::string typeStr[] = {"checkbox", "slider", "combobox", "none"};
				return typeStr[type];
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
					std::vector<ParamItem> params;
					_json_ctrl->ReadValue(params, name, root["params"]);
					param_lists[name].params = params;
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

QString StdStr2QStr(std::string str)
{
	return QString::fromLocal8Bit(str.c_str());
}

std::string QStr2StdStr(QString str)
{
	return (std::string)str.toLocal8Bit();
}

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

	ui.openGLWidget->SetMainWnd(this);

	int page = g_config->Get("init_page", 0);
	ui.toolBox->setCurrentIndex(page);
	bool is_check = g_config->Get("save_with_src_img", false);
	ui.checkBox_saveWithSrcImg->setChecked(is_check);
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
		nama_out = _nama->Process(src);
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
		img0.copyTo(res.colRange(0, img0.cols));
		img1.copyTo(res.colRange(img0.cols, img0.cols+img1.cols));
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
				item.layout = new QHBoxLayout();

				item.spin_box = new QSpinBox();
				item.spin_box->setToolTip(StdStr2QStr("目标道具"));
				item.spin_box->setSingleStep(1);
				item.spin_box->setRange(0, 0);
				item.spin_box->setValue(0);
				item.spin_box->setMaximumWidth(50);
				item.layout->addWidget(item.spin_box);
				connect(item.spin_box, SIGNAL(valueChanged(int)), SLOT(SpinBoxChanged(int)));

				switch (item.type)
				{
				case ParamItem::TYPE_CHECKBOX:
				{
					item.check_box = new QCheckBox(StdStr2QStr(item.show_name));
					item.check_box->setChecked(item.valB);
					item.check_box->setObjectName(StdStr2QStr(param.first+"_"+item.param));

					connect(item.check_box, SIGNAL(clicked()), SLOT(CheckBoxClick()));
					item.layout->addWidget(item.check_box);
					item.layout->addStretch();

				}
					break;
				case ParamItem::TYPE_SLIDER:
				{
					item.label_name = new QLabel(StdStr2QStr(item.show_name));
					item.layout->addWidget(item.label_name);

					item.slider = new QSlider();
					item.slider->setRange(item.range.first, item.range.second);
					item.slider->setValue(0);
					item.slider->setOrientation(Qt::Orientation::Horizontal);
					item.slider->setMinimumHeight(15);
					item.slider->setObjectName(StdStr2QStr(param.first + "_" + item.param));
					connect(item.slider, SIGNAL(valueChanged(int)), SLOT(SliderChanged(int)));
					item.layout->addWidget(item.slider);

					item.label = new QLabel();
					item.label->setText(StdStr2QStr("0"));
					item.label->setMinimumWidth(30);
					item.label->setMaximumWidth(30);
					item.label->setAlignment(Qt::AlignRight);
					item.layout->addWidget(item.label);
				}
					break;
				case ParamItem::TYPE_COMBOBOX:
				{
					item.label_name = new QLabel(StdStr2QStr(item.show_name));
					item.layout->addWidget(item.label_name);

					item.combobox = new QComboBox();
					connect(item.combobox, SIGNAL(currentTextChanged(QString)), this, SLOT(CurrentTextChanged(QString)));
					QStringList sl;
					auto backup_str = item.valStr;
					for (auto& txt : item.combo_texts)
						sl << StdStr2QStr(txt);
					item.combobox->addItems(sl);
					item.combobox->setCurrentText(StdStr2QStr(backup_str));
					item.layout->addWidget(item.combobox);
					item.layout->addStretch();
				}
					break;
				}
				layout->addRow(StdStr2QStr(""), item.layout);

				if (item.tooltip != "")
				{
					QString txt = StdStr2QStr(item.tooltip);
					QWidget* ctrls[] = { item.check_box, item.slider, item.label, item.combobox };
					for (auto ctrl : ctrls)
						if(ctrl!=nullptr)
							ctrl->setToolTip(txt);
				}
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
	if (keyEvent->modifiers() == Qt::AltModifier)
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

	return QWidget::keyPressEvent(keyEvent);
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
	vecS dirs;
	CmFile::GetSubFolders(g_resDir, dirs);
	for (auto dir : dirs)
	{
		auto* parent_item = new QTreeWidgetItem(ui.treeWidget_props);
		parent_item->setText(0, StdStr2QStr(dir));
		//parent_item->setData(1, Qt::ItemDataRole::UserRole, QVariant(0));
		//parent_item->setText(0, _train_results[i].show_title);

		vecS names, tmp;
		for (auto postfix : g_prop_postfixs)
		{
			CmFile::GetNames(g_resDir + dir + "/"+ postfix, tmp);
			for (auto a : tmp)
				names.push_back(g_resDir + dir + "/" + a);
		}
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
			item.spin_box->setMaximum(tmp);
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

void NamaDemo_YXL::UpdateCtrlValue()
{
	auto& param = _param_lists[_cur_param_list];
	for (auto& item : param.params)
	{
		item.spin_box->setValue(item.prop_idx);
		if (item.slider)
			SetCtrlData(item.slider, item.valI);
		if (item.check_box)
			SetCtrlData(item.check_box, item.valB);
		if (item.combobox)
			SetCtrlData(item.combobox, item.valStr);
	}
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
	fuItemGetParamu8v(prop_handle, &param[0], buff, cnt);

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
		fout << "f " << tris[3 * i + order[0]] + 1 <<"/"<< tris[3 * i + order[0]] + 1 << " " << tris[3 * i + order[1]] + 1 << "/" << tris[3 * i + order[1]] + 1 << " " << tris[3 * i + order[2]] + 1 << "/" << tris[3 * i + order[2]] + 1 << std::endl;

	fout.close();
}

void NamaDemo_YXL::UpdateParamsFromProp()
{
	if (!_nama)
		return;

	for(auto& param:_param_lists)
		for (auto& item : param.second.params)
		{
			if (item.type == ParamItem::TYPE_COMBOBOX)
			{
				std::string str = "";
				if (_propsUsed.size()>item.prop_idx)
					str = _nama->GetPropParameterStr(_propsUsed[item.prop_idx], item.param);
				if ("" != str)
					item.valStr = str;
			}
			else
			{
				double val = 0.0;
				if (_propsUsed.size()>item.prop_idx)
					val = _nama->GetPropParameterD(
						_propsUsed[item.prop_idx],
						item.param);
				switch (item.type)
				{
				case ParamItem::TYPE_CHECKBOX:
					item.valB = val > 0.0;
					break;
				case ParamItem::TYPE_SLIDER:
					item.valI = val / item.scale;
					break;
				}
			}
		}
	UpdateCtrlValue();
	SaveParams();
}

void NamaDemo_YXL::CheckBoxClick()
{
	auto obj = sender();

	for(auto& param:_param_lists)
		for (auto& item : param.second.params)
		{
			if (obj == item.check_box)
			{
				item.valB = item.check_box->isChecked();

				if (_nama && item.prop_idx >= 0 && item.prop_idx < _propsUsed.size())
					_nama->SetPropParameter(_propsUsed[item.prop_idx], item.param, 
						item.valB ? 1.0 : 0.0);
				if (false == _is_param_batch_update)
					SaveParams();
				return;
			}
		}
}

void NamaDemo_YXL::SliderChanged(int val)
{
	auto obj = sender();

	for (auto& param : _param_lists)
		for (auto& item : param.second.params)
		{
			if (obj == item.slider)
			{
				item.valI = item.slider->value();
				auto tmp = item.valI*item.scale;
				item.label->setText(QString::number(tmp));
				if (_nama && item.prop_idx >= 0 && item.prop_idx < _propsUsed.size())
					_nama->SetPropParameter(_propsUsed[item.prop_idx], item.param, tmp);
				if (false == _is_param_batch_update)
					SaveParams();
				return;
			}
		}
}

void NamaDemo_YXL::SpinBoxChanged(int val)
{
	auto obj = this->sender();

	for (auto& param : _param_lists)
		for (auto& item : param.second.params)
		{
			if (obj == item.spin_box)
			{
				item.prop_idx = item.spin_box->value();
				if (false == _is_param_batch_update)
					SaveParams();
			}
		}
}

void NamaDemo_YXL::ButtonClicked()
{
	const auto obj = this->sender();

	if (obj==ui.checkBox_nama_in_bgra)
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
		_save_img_path_format = g_saveDir + time+"/%06d.png";
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
			if (obj == item.combobox)
			{
				item.valStr = QStr2StdStr(item.combobox->currentText());
				if (_nama && item.prop_idx >= 0 && item.prop_idx < _propsUsed.size())
					_nama->SetPropParameter(_propsUsed[item.prop_idx], item.param, item.valStr);
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
