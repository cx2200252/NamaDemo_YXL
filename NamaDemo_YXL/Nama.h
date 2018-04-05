#pragma once
#include "CmFile.h"

class CCameraDS;

namespace FU
{
	class Nama
	{
	public:
		Nama(CStr& resDir);
		void Init(std::string v3Path);

		void InitArdataExt(std::string path);

		void OnCameraChange();

		void SetPropsUsed(vecS& props);

		void SetMaxFace(const int max_face);
		void SetBGRAIn(bool is_bgra);
		void SetBGRAOut(bool is_bgra);
		void SetVerticalIn(bool is_vertical);
		void SetVerticalOut(bool is_vertical);

		//return processed image
		cv::Mat Process(cv::Mat img);
		cv::Mat ToNamaIn(cv::Mat in);

		void SetPropParameter(std::string prop, std::string name, const double value);
		void SetPropParameter(std::string prop, std::string name, std::string value);
		void SetPropParameter(std::string prop, std::string name, std::vector<double>& value);
		void SetPropParameter(std::string prop, std::string name, char* value, int size);

		double GetPropParameterD(std::string prop, std::string name);
		std::string GetPropParameterStr(std::string prop, std::string name);
		bool GetPropParameterDv(std::string prop, std::string name, std::vector<double>& value);

	private:
		int CreateProp(const std::string& path);
		cv::Mat FromNamaOut(cv::Mat out);

		cv::Mat ToVertical(cv::Mat);
		cv::Mat ToHorizontal(cv::Mat);

	private:
		int _frameID;

		std::string _resDir;

	private:
		vecS _props_used;
		std::map<std::string, int> _props;

	private:
		bool _bgra_in = false;
		bool _bgra_out = false;
		bool _vertical_in = false;
		bool _vertical_out = false;
	};
}