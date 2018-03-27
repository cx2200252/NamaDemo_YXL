#include "Nama.h"
#include <funama.h>
#include <authpack.h>

#pragma comment(lib, "nama.lib")

#define _LOG_TIME_

namespace FU
{
	size_t FileSize(std::ifstream& file)
	{
		std::streampos oldPos = file.tellg();

		file.seekg(0, std::ios::beg);
		std::streampos beg = file.tellg();
		file.seekg(0, std::ios::end);
		std::streampos end = file.tellg();

		file.seekg(oldPos, std::ios::beg);

		return static_cast<size_t>(end - beg);
	}

	template<typename type> bool LoadProp(const std::string& filepath, std::vector<type>& data)
	{
		std::ifstream fin(filepath, std::ios::binary);
		if (false == fin.good())
		{
			fin.close();
			return false;
		}
		size_t size = FileSize(fin);
		if (0 == size)
		{
			fin.close();
			return false;
		}
		data.resize(size / sizeof(type));
		fin.read(reinterpret_cast<char*>(&data[0]), size);

		fin.close();
		return true;
	}

}


FU::Nama::Nama(CStr& resDir)
	:_frameID(0),
	_resDir(resDir)
{
	
}

//typedef int (*SetupFunc)(int n);

void FU::Nama::Init(std::string v3Path)
{
	std::vector<char> v3data;
	CV_Assert(true == FU::LoadProp(v3Path, v3data));
	int ret = fuSetup((float*)&v3data[0], NULL, g_auth_package, sizeof(g_auth_package));

}

void FU::Nama::InitArdataExt(std::string path)
{
	std::vector<char> data;

	CV_Assert(true == FU::LoadProp(path, data));
	fuLoadExtendedARData(&data[0], data.size());

	std::cout << "load ardata ext data." << std::endl;
}

void FU::Nama::OnCameraChange()
{
	fuOnCameraChange();
	cv::Mat tmp(100, 100, CV_8UC4, cv::Scalar::all(0));
	fuRenderItemsEx(FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int*>(tmp.data),
		FU_FORMAT_BGRA_BUFFER, reinterpret_cast<int*>(tmp.data),
		tmp.cols, tmp.rows, _frameID, nullptr, 0);
	//fuRenderItems(0, reinterpret_cast<int*>(img.data), img.cols, img.rows, _frameID, &props[0], props.size());

	std::cout << "fuOnCameraChange" << std::endl;
}

void FU::Nama::SetPropsUsed(vecS & props)
{
	_props_used = props;
}

void FU::Nama::SetMaxFace(const int max_face)
{
	fuSetMaxFaces(max_face);
	std::cout << "set max face: " << max_face << std::endl;
}

void FU::Nama::SetBGRAIn(bool is_bgra)
{
	_bgra_in = is_bgra;
}

void FU::Nama::SetBGRAOut(bool is_bgra)
{
	_bgra_out = is_bgra;
}

void FU::Nama::SetVerticalIn(bool is_vertical)
{
	_vertical_in = is_vertical;
}

void FU::Nama::SetVerticalOut(bool is_vertical)
{
	_vertical_out = is_vertical;
}

cv::Mat FU::Nama::Process(cv::Mat img)
{
	cv::Mat in = ToNamaIn(img);
	cv::Mat out(in.size(), in.type());
	//
	std::vector<int> props;
	for (auto& prop : _props_used)
	{
		if (0 == _props[prop])
		{
			int handle = CreateProp(prop);
			_props[prop] = handle;
		}
		props.push_back(_props[prop]);
	}

	if (true != props.empty())
	{
		fuRenderItemsEx2(
			_bgra_out ? FU_FORMAT_BGRA_BUFFER : FU_FORMAT_RGBA_BUFFER, 
			reinterpret_cast<int*>(out.data),
			_bgra_in ? FU_FORMAT_BGRA_BUFFER : FU_FORMAT_RGBA_BUFFER,
			reinterpret_cast<int*>(in.data),
			in.cols, in.rows, _frameID, &props[0], props.size(), NAMA_RENDER_FEATURE_FULL, 0);
		++_frameID;
	}
	else 
	{
		fuRenderItemsEx2(
			_bgra_out ? FU_FORMAT_BGRA_BUFFER : FU_FORMAT_RGBA_BUFFER,
			reinterpret_cast<int*>(out.data),
			_bgra_in ? FU_FORMAT_BGRA_BUFFER : FU_FORMAT_RGBA_BUFFER,
			reinterpret_cast<int*>(in.data),
			in.cols, in.rows, _frameID, nullptr, 0, NAMA_RENDER_FEATURE_FULL, 0);
		++_frameID;
	}
	//cv::imshow("in", in);

	return FromNamaOut(out);
}

void FU::Nama::SetPropParameter(std::string prop, std::string name, const double value)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second==0)
		return;
	fuItemSetParamd(iter->second, &name[0], value);
}

void FU::Nama::SetPropParameter(std::string prop, std::string name, std::string value)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0)
		return;
	fuItemSetParams(iter->second, &name[0], &value[0]);
}

void FU::Nama::SetPropParameter(std::string prop, std::string name, std::vector<double>& value)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0)
		return;
	fuItemSetParamdv(iter->second, &name[0], &value[0], (int)value.size());
}

double FU::Nama::GetPropParameterD(std::string prop, std::string name)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0)
		return 0.0;
	return fuItemGetParamd(iter->second, &name[0]);
}

std::string FU::Nama::GetPropParameterStr(std::string prop, std::string name)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0)
		return "";

	char buf[100];
	int cnt = fuItemGetParams(iter->second, &name[0], buf, sizeof(buf));
	if (cnt < 0)
		return "";
	if (cnt > sizeof(buf))
	{
		char* buf2 = new char[cnt];
		fuItemGetParams(iter->second, &name[0], buf2, cnt);
		auto ret = std::string(buf2);
		delete[] buf2;
		return ret;
	}
	else
		return std::string(buf);
}

bool FU::Nama::GetPropParameterDv(std::string prop, std::string name, std::vector<double>& value)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0 || value.empty())
		return false;
	return fuItemGetParamdv(iter->second, &name[0], &value[0], value.size())==value.size();
}

int FU::Nama::CreateProp(const std::string & path)
{
	std::vector<char> data;

	CV_Assert(true == FU::LoadProp(_resDir+path, data));
	std::cout << "load bundle: "<< _resDir+path << std::endl;

	int handle = fuCreateItemFromPackage(&data[0], data.size());
	return handle;
}

cv::Mat FU::Nama::ToNamaIn(cv::Mat in)
{
	cv::Mat img;
	if (false == _bgra_in)
		cv::cvtColor(in, img, CV_BGRA2RGBA);
	else
		img = in;
	if (_vertical_in)
		img = ToVertical(img);
	else
		img = ToHorizontal(img);
	return img;
}

cv::Mat FU::Nama::FromNamaOut(cv::Mat out)
{
	cv::Mat img;
	if (false == _bgra_out)
		cv::cvtColor(out, img, CV_RGBA2BGRA);
	else
		img = out;
	if (_vertical_out)
		img = ToVertical(img);
	return img;
}

cv::Mat FU::Nama::ToVertical(cv::Mat img)
{
	if (img.rows<img.cols)
	{
		cv::transpose(img, img);
		cv::flip(img, img, 0);
	}
	return img;
}

cv::Mat FU::Nama::ToHorizontal(cv::Mat img)
{
	if (img.rows>img.cols)
	{
		cv::transpose(img, img);
		cv::flip(img, img, 0);
	}
	return img;
}
