#include "Nama.h"
#include <funama.h>
#include <authpack.h>

#pragma comment(lib, "nama.lib")

#define _LOG_TIME_

//namespace FU
//{
//	size_t FileSize(std::ifstream& file)
//	{
//		std::streampos oldPos = file.tellg();
//
//		file.seekg(0, std::ios::beg);
//		std::streampos beg = file.tellg();
//		file.seekg(0, std::ios::end);
//		std::streampos end = file.tellg();
//
//		file.seekg(oldPos, std::ios::beg);
//
//		return static_cast<size_t>(end - beg);
//	}
//
//	template<typename type> bool LoadProp(const std::string& filepath, std::vector<type>& data)
//	{
//		std::ifstream fin(filepath, std::ios::binary);
//		if (false == fin.good())
//		{
//			fin.close();
//			return false;
//		}
//		size_t size = FileSize(fin);
//		if (0 == size)
//		{
//			fin.close();
//			return false;
//		}
//		data.resize(size / sizeof(type));
//		fin.read(reinterpret_cast<char*>(&data[0]), size);
//
//		fin.close();
//		return true;
//	}
//
//}


FU::Nama::Nama(CStr& resDir)
	:_frameID(0),
	_resDir(resDir)
{
	
}

//typedef int (*SetupFunc)(int n);

void FU::Nama::Init(std::string v3Path)
{
	std::string v3data;
	CV_Assert(true == YXL::File::LoadFileContentBinary(v3Path, v3data));
	int ret = fuSetup((float*)&v3data[0], NULL, g_auth_package, sizeof(g_auth_package));
}

void FU::Nama::InitArdataExt(std::string path)
{
	std::string data;

	CV_Assert(true == YXL::File::LoadFileContentBinary(path, data));
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

//void SaveObj2(bool is_ext, int face_id, const std::string& save_path)
//{
//	unsigned char* buff = fuGetARMesh(face_id);
//	if (!buff)
//		return;
//
//	int n_vertex, n_triangle;
//	{
//		int* ptr = reinterpret_cast<int*>(buff);
//		n_vertex = ptr[0];
//		n_triangle = ptr[1];
//	}
//	if (n_vertex <= 0 || n_triangle <= 0)
//		return;
//	float* vertices = reinterpret_cast<float*>(buff + sizeof(int) * 2);
//	unsigned short* uv = reinterpret_cast<unsigned short*>(buff + sizeof(int) * 2 + n_vertex * 3 * sizeof(float));
//	unsigned short* tris = reinterpret_cast<unsigned short*>(buff + sizeof(int) * 2 + n_vertex * 4 * sizeof(float));
//	
//	std::ofstream fout(save_path);
//	for (int i(0); i != n_vertex; ++i)
//		fout << "v " << vertices[3 * i] << " " << vertices[3 * i + 1] << " " << vertices[3 * i + 2] << std::endl;
//	for (int i(0); i != n_vertex; ++i)
//		fout << "vt " << float(uv[2 * i]) / 65535.f << " " << float(uv[2 * i + 1]) / 65535.f << std::endl;
//	int order[] = { 2, 1, 0 };
//	for (int i(0); i != n_triangle; ++i)
//		fout << "f " << tris[3 * i + order[0]] + 1 << "/" << tris[3 * i + order[0]] + 1 << " "
//		<< tris[3 * i + order[1]] + 1 << "/" << tris[3 * i + order[1]] + 1 << " "
//		<< tris[3 * i + order[2]] + 1 << "/" << tris[3 * i + order[2]] + 1 << std::endl;
//
//	fout.close();
//}


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
			in.cols, in.rows, _frameID, &props[0], props.size(), NAMA_RENDER_FEATURE_FULL | NAMA_RENDER_OPTION_FLIP_X, 0);
		++_frameID;
	}
	else 
	{
		fuRenderItemsEx2(
			_bgra_out ? FU_FORMAT_BGRA_BUFFER : FU_FORMAT_RGBA_BUFFER,
			reinterpret_cast<int*>(out.data),
			_bgra_in ? FU_FORMAT_BGRA_BUFFER : FU_FORMAT_RGBA_BUFFER,
			reinterpret_cast<int*>(in.data),
			in.cols, in.rows, _frameID, nullptr, 0, NAMA_RENDER_FEATURE_FULL | NAMA_RENDER_OPTION_FLIP_X, 0);
		++_frameID;
	}
	//cv::imshow("in", in);

	/*{
		float exp[46] = { 0.0f };
		fuGetFaceInfo(0, "expression", exp, 46);
		std::cout << "[";
		for (int i(0); i != 46; ++i)
		{
			if (i)
				std::cout << ",";
			std::cout << exp[i];
		}
		std::cout << "]" << std::endl;
	}*/

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

void FU::Nama::SetPropParameter(std::string prop, std::string name, char * value, int size)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0)
		return;
	fuItemSetParamu8v(iter->second, &name[0], value, size);
}

void FU::Nama::SetPropParameter(std::string prop, std::string name, long long value)
{
	auto iter = _props.find(prop);
	if (iter == _props.end() || iter->second == 0)
		return;
	fuItemSetParami64(iter->second, &name[0], value);
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
	std::string data;

	CV_Assert(true == YXL::File::LoadFileContentBinary(_resDir+path, data));
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
