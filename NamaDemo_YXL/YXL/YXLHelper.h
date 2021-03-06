#ifndef _YXL_HELPER_H_
#define _YXL_HELPER_H_

//which modul to use
#define _YXL_OTHER_
#define _YXL_FILES_
#define _YXL_STRING_
#define _YXL_PARAM_PARSER_
#define _YXL_PRINT_
#define _YXL_OUT_STREAM_
#define _YXL_UNION_FIND_
#define _YXL_KD_TREE_
#define _YXL_TIME_
#define _YXL_CONSOLE_
#define _YXL_TRANSFORM_
#define _YXL_GRAPHIC_
#define _YXL_IMG_PROC_
//#define _YXL_GLFW_

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <chrono>
#include <mutex>
#include <queue>
#include <algorithm>
#include <stack>

#pragma warning(disable:4819)
#ifdef YXL_HELPER_DYNAMIC
#ifndef LIB_YXL_HELPER
#ifdef _LIB_YXL_HELPER_IMPL
#define LIB_YXL_HELPER __declspec(dllexport)
#else
#define LIB_YXL_HELPER __declspec(dllimport)
#endif
#endif
#else
#define LIB_YXL_HELPER
#endif

typedef const std::string CStr;
typedef std::vector<std::string> vecS;
typedef std::vector<int> vecI;
typedef std::vector<std::string> vecS;
typedef std::vector<float> vecF;
typedef std::vector<double> vecD;
#define _S(str) ((str).c_str())

//check marco
#ifndef _WITH_WINDOWS_
#undef _YXL_CONSOLE_
#endif
#ifndef _YXL_TRANSFORM_
#undef _YXL_GRAPHIC_
#endif
#ifndef _WITH_OPENCV_
#undef _YXL_IMG_PROC_
#endif

//
#ifdef _YXL_GLFW_
#include <gl/glew.h>
#include <GLFW/glfw3.h>
#endif

#ifdef _WITH_OPENCV_

#define POINTER_64 __ptr64
#include <opencv2/opencv.hpp>

#ifndef CV_LIB
#define CV_VERSION_ID CVAUX_STR(CV_MAJOR_VERSION) CVAUX_STR(CV_MINOR_VERSION) CVAUX_STR(CV_SUBMINOR_VERSION)
#ifdef _DEBUG
#define CV_LIB(name) "opencv_" name CV_VERSION_ID "d"
#else
#define CV_LIB(name) "opencv_" name CV_VERSION_ID
#endif
#endif

#define CV_Assert_(expr, args) \
{\
	if(!(expr)) {\
	std::string msg = cv::format args; \
	printf("%s in %s:%d\n", msg.c_str(), __FILE__, __LINE__); \
	cv::error(cv::Exception(CV_StsAssert, msg, __FUNCTION__, __FILE__, __LINE__) ); }\
}

#pragma comment( lib, CV_LIB("core"))
#pragma comment( lib, CV_LIB("imgproc"))
#pragma comment( lib, CV_LIB("highgui"))


#if (2 < CV_MAJOR_VERSION)
#pragma comment( lib, CV_LIB("imgcodecs"))
#endif

typedef const cv::Mat CMat;
typedef std::vector<cv::Mat> vecM;

#endif

#ifdef _WITH_WINDOWS_
#include <Windows.h>
#else
#ifdef _WITH_QT_
#include <qdir.h>
#include <qprocess.h>
#include <QtWidgets/qfiledialog.h>
#include <QtWidgets/QApplication.h>
#include <qdatetime.h>
#include <qmutex.h>

#ifdef _DEBUG
#pragma comment(lib, "Qt5Cored.lib")
#pragma comment(lib, "Qt5Guid.lib")
#pragma comment(lib, "Qt5Widgetsd.lib")
#else
#pragma comment(lib, "Qt5Core.lib")
#pragma comment(lib, "Qt5Gui.lib")
#pragma comment(lib, "Qt5Widgets.lib")
#endif
#endif
#endif

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#endif

#ifdef _YXL_OTHER_
namespace YXL
{
	inline long long MakeLongLong(const unsigned int a, const unsigned int b)
	{
		return ((unsigned long long)a) << 32 | (unsigned long long)b;
	}

	inline std::pair<unsigned int, unsigned int> SplitLongLong(const unsigned long long l)
	{
		return std::make_pair((unsigned int)(l >> 32), (unsigned int)(l & 0xffffffff));
	}

	std::string LIB_YXL_HELPER SHA1Digest(std::string str);

	inline int GetCurrentThreadID()
	{
#if defined(_WITH_WINDOWS_)
		return GetCurrentThreadId();
#elif defined(__linux__)
		return gettid();
#else
		return 0;
#endif
	}
}
#endif

#ifdef _YXL_STRING_
namespace YXL
{
	namespace Str
	{
		inline std::string Esacpe(const std::string& str, const std::string escape_ch="\t ")
		{
			auto beg = str.find_first_not_of(escape_ch);
			auto end = str.find_last_not_of(escape_ch);
			return str.substr(beg, end - beg+1);
		}

		inline std::string Replace(const std::string& str, std::map<std::string, std::string>& replace_strs)
		{
			std::string res = str;
			size_t pos = 0;
			for (auto& rep : replace_strs)
				while (std::string::npos != (pos = res.find(rep.first)))
					res.replace(pos, rep.first.length(), rep.second);
			return res;
		}

		inline void Spilt(std::vector<std::string>& res, const std::string& src, const std::string& splitChars)
		{
			std::size_t prePos = src.find_first_not_of(splitChars);
			std::size_t pos = src.find_first_of(splitChars, prePos);
			while (std::string::npos != pos)
			{
				res.push_back(src.substr(prePos, pos - prePos));
				prePos = src.find_first_not_of(splitChars, pos + 1);
				pos = src.find_first_of(splitChars, prePos);
			}
			if (std::string::npos != prePos)
				res.push_back(src.substr(prePos, src.length() - prePos));
		}
	}
}
#endif

#ifdef _YXL_FILES_
namespace YXL
{
	class LIB_YXL_HELPER File
	{
	public:
		static std::string ToUnixPath(const std::string& str)
		{
			std::string path = str;
			for (auto iter = path.begin(); iter != path.end(); ++iter)
				if ('\\' == *iter)
					*iter = '/';
			return path;
		}
		static std::string ToWindowsPath(const std::string& str)
		{
			std::string path = str;
			for (auto iter = path.begin(); iter != path.end(); ++iter)
				if ('/' == *iter)
					*iter = '\\';
			return path;
		}
		static std::string CheckDirPath(const std::string& str)
		{
			return ('/' == *str.rbegin() || '\\' == *str.rbegin()) ? str : str + "/";
		}
		static std::string GetFolder(CStr& path)
		{
			return path.substr(0, path.find_last_of("\\/") + 1);
		}
		static std::string GetName(CStr& path)
		{
			size_t start = path.find_last_of("\\/") + 1;
			size_t end = path.find_last_not_of(' ') + 1;
			return path.substr(start, end - start);
		}
		static std::string GetNameNE(CStr& path)
		{
			size_t start = path.find_last_of("\\/") + 1;
			size_t end = path.find_last_of('.');
			if (end >= 0)
				return path.substr(start, end - start);
			else
				return path.substr(start, path.find_last_not_of(' ') + 1 - start);
		}
		static std::string GetPathNE(CStr& path)
		{
			size_t end = path.find_last_of('.');
			if (end >= 0)
				return path.substr(0, end);
			else
				return path.substr(0, path.find_last_not_of(' ') + 1);
		}
		static std::string GetExtention(CStr name)
		{
			return name.substr(name.find_last_of('.'));
		}
		static void WriteNullFile(CStr& fileName) 
		{
			FILE *f = fopen(_S(fileName), "w"); 
			fclose(f); 
		}
		static vecS loadStrList(CStr &fName);
		static bool writeStrList(CStr &fName, const vecS &strs);

		static size_t FileSize(std::ifstream& file)
		{
			std::streampos oldPos = file.tellg();

			file.seekg(0, std::ios::beg);
			std::streampos beg = file.tellg();
			file.seekg(0, std::ios::end);
			std::streampos end = file.tellg();

			file.seekg(oldPos, std::ios::beg);

			return static_cast<size_t>(end - beg);
		}
		static size_t FileSize(const std::string& filename)
		{
			std::ifstream fin(filename);
			size_t ret = FileSize(fin);
			fin.close();
			return ret;
		}

		static bool LoadFileContentBinary(const std::string& filepath, std::string& data)
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
			data.resize(size);
			fin.read(reinterpret_cast<char*>(&data[0]), size);
			fin.close();
			return true;
		}

		static void LoadFileContent(const std::string& path, std::string& content, const std::string ignore="")
		{
			content = "";
			std::ifstream fin(path);
			std::string line;
			if ("" == ignore)
			{
				while (getline(fin, line))
					content += line + "\n";
			}
			else
			{
				while (getline(fin, line))
					if (Str::Esacpe(line).substr(0, ignore.length()) != ignore)
						content += line + "\n";
			}
			
			fin.close();
		}

		template<typename VertexType, typename IndexType> static void SavePlainObjFile(const std::string& save_path, const VertexType* vertices, const int vertex_cnt, const IndexType* face, const int face_cnt)
		{
			std::ofstream fout(save_path);
			for (int i(0); i != vertex_cnt; ++i)
			{
				fout << "v " << vertices[0] << " " << vertices[1] << " " << vertices[2] << "\n";
				vertices += 3;
			}
			for (int i(0); i != face_cnt; ++i)
			{
				fout << "f ";
				for (int j(0); j != 3; ++j)
				{
					fout << face[2 - j] + 1 << " ";
				}
				fout << "\n";
				face += 3;
			}
			fout.close();
		}

#if defined(_WITH_WINDOWS_) || defined(_WITH_QT_)
		static std::string ToAbsolutePath(const std::string& path)
		{
			if (path.length() > 2 && path[1] == ':')
				return path;
			return GetWkDir() + "/" + path;
		}
#endif

#ifdef _WITH_WINDOWS_
		enum FileInfo {
			FileInfo_CreateTime,
			FileInfo_LastAccessTime,
			FileInfo_LastWriteTime,
			FileInfo_FileSize
		};
		//return high-low
		static std::pair<DWORD, DWORD> GetFileInfo(const std::string& file_path, FileInfo fi);

		static std::string BrowseFile(const char* strFilter = "Images (*.jpg;*.png)\0*.jpg;*.png\0All (*.*)\0*.*\0\0", bool isOpen = true, const std::string& dir = "", CStr& title = "BrowseFile");
		static std::string BrowseFolder(CStr& title = "BrowseFolder");

		// Get file names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
		static int GetNames(CStr &nameW, vecS &names, std::string &dir = std::string());
		static int GetNames(CStr& rootFolder, CStr &fileW, vecS &names);
		static int GetNamesNE(CStr& nameWC, vecS &names, std::string &dir = std::string(), std::string &ext = std::string());
		static int GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names);

		static bool FileExist(CStr& filePath)
		{
			if (filePath.size() == 0)
				return false;
			return  GetFileAttributesA(_S(filePath)) != INVALID_FILE_ATTRIBUTES;
		}
		//FilesExist("./*.jpg")
		static bool FilesExist(CStr& fileW)
		{
			vecS names;
			int fNum = GetNames(fileW, names);
			return fNum > 0;
		}
		static bool FolderExist(CStr& strPath)
		{
			int i = (int)strPath.size() - 1;
			for (; i >= 0 && (strPath[i] == '\\' || strPath[i] == '/'); i--)
				;
			std::string str = strPath.substr(0, i + 1);

			WIN32_FIND_DATAA  wfd;
			HANDLE hFind = FindFirstFileA(_S(str), &wfd);
			bool rValue = (hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
			FindClose(hFind);
			return rValue;
		}

		static std::string GetWkDir()
		{
			std::string wd;
			wd.resize(1024);
			DWORD len = GetCurrentDirectoryA(1024, &wd[0]);
			wd.resize(len);
			return wd;
		}
		static void SetWkDir(CStr& dir)
		{
			SetCurrentDirectoryA(dir.c_str());
		}

		static bool MkDir(CStr&  path);

		// Eg: RenameImages("D:/DogImages/*.jpg", "F:/Images", "dog", ".jpg");
		static int Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt);

		//RmFile("./*.jpg")
		static void RmFile(CStr& fileW);
		static void CleanFolder(CStr& dir, bool subFolder = false);
		static void RmFolder(CStr& dir);
		

		static int GetSubFolders(CStr& folder, vecS& subFolders);

		static bool Copy(CStr &src, CStr &dst, bool failIfExist = false)
		{
			return ::CopyFileA(src.c_str(), dst.c_str(), failIfExist)==TRUE;
		}
		static bool Move(CStr &src, CStr &dst, DWORD dwFlags = MOVEFILE_REPLACE_EXISTING | MOVEFILE_COPY_ALLOWED | MOVEFILE_WRITE_THROUGH)
		{
			return MoveFileExA(src.c_str(), dst.c_str(), dwFlags)==TRUE;
		}
		//Move2Dir("./*.jpg", "../")
		static bool Move2Dir(CStr &srcW, CStr dstDir);
		//Copy2Dir("./*.jpg", "../")
		static bool Copy2Dir(CStr &srcW, CStr dstDir);

		static void RunProgram(CStr &fileName, CStr &parameters = "", bool waiteF = false, bool showW = true);
#else
#ifdef _WITH_QT_
		static void InitQApplication(int argc, char** argv)
		{
			if (nullptr == _app)
				_app = std::shared_ptr<QApplication>(new QApplication(argc, argv));
		}
		//may need call CmFile::InitQApplication first
		static std::string BrowseFile(const char* strFilter = "Images (*.jpg *.png);;All (*.*)", bool isOpen = true, const std::string& dir = "", CStr& title = "BrowseFile");
		//may need call CmFile::InitQApplication first
		static std::string BrowseFolder(CStr& title = "BrowseFolder");

		// Get file names from a wildcard. Eg: GetNames("D:\\*.jpg", imgNames);
		static int GetNames(CStr &nameW, vecS &names, std::string &dir);
		static int GetNames(CStr& rootFolder, CStr &fileW, vecS &names);
		static int GetNamesNE(CStr& nameWC, vecS &names, std::string &dir, std::string &ext);
		static int GetNamesNE(CStr& rootFolder, CStr &fileW, vecS &names);

		static inline bool FileExist(CStr& filePath)
		{
			if (filePath.size() == 0)
				return false;

			QFile file(QString::fromLocal8Bit(_S(filePath)));
			return file.exists();
		}
		static inline bool FilesExist(CStr& fileW)
		{
			vecS names;
			std::string dir;
			int fNum = GetNames(fileW, names, dir);
			return fNum > 0;
		}
		static inline bool FolderExist(CStr& strPath)
		{
			int i = (int)strPath.size() - 1;
			for (; i >= 0 && (strPath[i] == '\\' || strPath[i] == '/'); i--)
				;
			std::string str = strPath.substr(0, i + 1);

			QDir dir(QString::fromLocal8Bit(_S(str)));
			return dir.exists();
		}

		static inline std::string GetWkDir()
		{
			QDir dir;
			return dir.currentPath().toLocal8Bit();
		}
		static inline void SetWkDir(CStr& dir)
		{
			QDir tmp;
			tmp.setCurrent(QString::fromLocal8Bit(_S(dir)));
		}

		static bool MkDir(CStr&  path);

		// Eg: RenameImages("D:/DogImages/*.jpg", "F:/Images", "dog", ".jpg");
		static int Rename(CStr& srcNames, CStr& dstDir, const char* nameCommon, const char* nameExt);

		static void RmFile(CStr& fileW);
		static void RmFolder(CStr& dir)
		{
			QDir tmp;
			tmp.rmdir(QString::fromLocal8Bit(_S(dir)));
		}
		static void CleanFolder(CStr& dir, bool subFolder = false);

		static int GetSubFolders(CStr& folder, vecS& subFolders);

		static bool Copy(CStr &src, CStr &dst)
		{
			QFile f;
			return f.copy(QString::fromLocal8Bit(_S(src)), QString::fromLocal8Bit(_S(dst)));
		}
		static bool Move(CStr &src, CStr &dst)
		{
			QFile f;
			return f.copy(QString::fromLocal8Bit(_S(src)), QString::fromLocal8Bit(_S(dst)));
		}
		static bool Move2Dir(CStr &srcW, CStr dstDir);
		static bool Copy2Dir(CStr &srcW, CStr dstDir);

		static void RunProgram(CStr &fileName, CStr &parameters = "", bool waiteF = false, bool showW = true);
	private:
		static std::shared_ptr<QApplication> _app;
#endif
#endif
	};
}
#endif

#ifdef _YXL_PARAM_PARSER_
namespace YXL
{
	typedef bool(*CmdLineParserCallback)(const std::string& name, const std::string& val);
	//argv: -name=val
	/*
	std::multimap<std::string, std::string> params;
	bool ParserCallback(const std::string& name, const std::string& val)
	{
	params.insert(std::make_pair(name, val));
	return true;
	}
	CmdLineParser(argc, argv, ParserCallback);
	*/
	inline void CmdLineParser(int argc, char** argv, CmdLineParserCallback callback)
	{
		for (int i(1); i < argc; ++i)
		{
			std::string param = argv[i];
			auto pos = param.find('=');
			std::string name = "";
			std::string val = "";
			if (pos != std::string::npos)
			{
				name = param.substr(0, pos);
				val = param.substr(pos + 1, param.length() - pos - 1);
			}
			else
				name = param;
			callback(name, val);
		}
	}
}
#endif

#ifdef _YXL_PRINT_
namespace YXL
{
	template<typename type> void _print(type t, const int i)
	{
		std::cout << '\t' << t[i];
	}
	template<typename type> void _print2(type t, const int i)
	{
		if (i)
			std::cout << ',';
		std::cout <<t[i];
	}

	template<typename... type> void PrintVector(const int vec_len, const type*... vecs)
	{
		const int arg_cnt = sizeof...(vecs);
		std::cout << "[";
		for (int i(0); i != vec_len; ++i)
		{
			if (i)
				std::cout << ",";
			if(arg_cnt>1)
				std::cout << "(";
			int tmp[] = { (_print2(vecs, i), 0)... };
			if(arg_cnt>1)
				std::cout << ")";
		}
		std::cout << "]" << std::endl;
	}
	
	template<typename... type> void PrintVectorAsRow(int vec_len, type*... vecs)
	{
		for (int i(0); i != vec_len; ++i)
		{
			std::cout << i << ":";
			int tmp[] = { (_print(vecs, i), 0)... };
			std::cout << "\n";
		}
		
	}

	template<typename type> void PrintVector(std::vector<type>& v)
	{
		PrintVector(v.size(), v.data());
	}
	template<typename type> void PrintVectorAsRow(std::vector<type>& v)
	{
		PrintVectorAsRow(v.size(), v.data());
	}

	template<typename key, typename val> void PrintMapAsRows(std::map<key, val>& m, const std::string& padding = "")
	{
		for (auto iter = m.begin(); iter != m.end(); ++iter)
			YXL::yxlout << padding << iter->first << '\t' << iter->second << "\n";
	}
}
#endif

#ifdef _YXL_OUT_STREAM_
namespace YXL
{
	template<typename _Elem, typename _Traits>
	class YXLOutStream
	{
	public:
		typedef std::basic_ostream<_Elem, _Traits> _Myt;
		typedef std::basic_ios<_Elem, _Traits> _Myios;
		typedef std::basic_streambuf<_Elem, _Traits> _Mysb;
		typedef std::ostreambuf_iterator<_Elem, _Traits> _Iter;
		typedef std::num_put<_Elem, _Iter> _Nput;

		void Lock()
		{
			_mu.lock();
		}
		void Unlock()
		{
			_mu.unlock();
		}

		template<typename type>
		YXLOutStream& operator<<(type val)
		{
			std::cout << val;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << val;
				fout.close();
			}
			return *this;
		}
		YXLOutStream& operator<<(_Myt& (__cdecl *_Pfn)(_Myt&))
		{
			std::cout << _Pfn;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << _Pfn;
				fout.close();
			}
			return *this;
		}
		YXLOutStream& operator<<(_Myios& (__cdecl *_Pfn)(_Myios&))
		{
			std::cout << _Pfn;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << _Pfn;
				fout.close();
			}
			return *this;
		}
		YXLOutStream& operator<<(std::ios_base& (__cdecl *_Pfn)(std::ios_base&))
		{
			std::cout << _Pfn;
			if ("" != _log_file)
			{
				std::ofstream fout(_log_file, std::ios::app);
				fout << _Pfn;
				fout.close();
			}
			return *this;
		}

		void SetLogFile(const std::string& log_file)
		{
			_log_file = log_file;
		}

	private:
		std::mutex _mu;
		std::string _log_file = "";
	};

	typedef YXLOutStream<char, std::char_traits<char> > YXLOut;

#if defined(_WITH_WINDOWS_) || defined(__linux__)
#define YXL_LOG_PREFIX YXL::GetCurrentThreadID()<<"["<<__FUNCTION__<<"] "
#else
#define YXL_LOG_PREFIX "["<<__FUNCTION__<<"] "
#endif
	extern YXLOut yxlout;
}
#endif

#ifdef _YXL_UNION_FIND_
namespace YXL
{
	class LIB_YXL_HELPER UnionFind
	{
	public:
		UnionFind(int cnt):_group_cnt(cnt), _id(std::vector<int>(cnt))
		{
			for (int i(0); i != cnt; ++i)
			{
				_id[i] = i;
				_group_size[i] = 1;
			}
		}

		int GroupCount() const
		{
			return _group_cnt;
		}
		bool IsConnected(const int a, const int b) const
		{
			return Find(a) == Find(b);
		}
		int Find(int a) const
		{
			while (a != _id[a])
				a = _id[a];
			return a;
		}
		void Union(int a, int b);
		void Update();

		int GroupID(const int a) const
		{
			return (0 <= a&&a < _group_id.size()) ? _group_id[a] : -1;
		}

		const std::vector<int>* Group(const int group_id)
		{
			return (0 <= group_id && group_id < _group_cnt) ? &(_groups[group_id]) : nullptr;
		}
		
	private:
		int _group_cnt;
		std::vector<int> _id;
		std::map<int, int> _group_size;
		
		std::vector<int> _group_id;
		std::map<int, std::vector<int> > _groups;
	};
}
#endif

#ifdef _YXL_KD_TREE_
namespace YXL
{
	template<typename type, int dim=3>
	class KDTree
	{
		struct Element
		{
			type val[dim];
		};
		struct Node
		{
			~Node()
			{
				if (left)
					delete left;
				if (right)
					delete right;
			}
			Element val;
			int split_dim;
			Node *left = nullptr;
			Node *right = nullptr;
		};
	public:
		~KDTree()
		{
			if (_root)
				delete _root;
		}

		void Bulid(const type* data, const int ele_cnt)
		{
			std::vector<Element> ele(ele_cnt);
			memcpy(&ele[0], data, ele_cnt*sizeof(Element));

			_root = Build(&ele[0], ele_cnt, 0);
		}

		void FindNearest(type* ret, const type* ele)
		{
			Element _ele, _ret;
			memcpy(_ele.val, ele, sizeof(Element));
			double min_dist = (std::numeric_limits<double>::max)();
			FindNearest(_ret, _ele, _root, min_dist);
			memcpy(ret, _ret.val, sizeof(Element));
		}

	private:
		Node* Build(Element* _data, const int ele_cnt, const int split_dim)
		{
			std::sort(_data, _data + ele_cnt, 
				[split_dim](const Element& a, const Element& b) {return a.val[split_dim] < b.val[split_dim]; });

			int mid = ele_cnt / 2;
			while (mid + 2 < ele_cnt && _data[mid].val[split_dim] == _data[mid + 1].val[split_dim])
				++mid;

			Node* node = new Node;
			node->val = _data[mid];
			node->split_dim = split_dim;

			int next_split_dim = (split_dim + 1) % dim;
			if (mid > 0)
				node->left = Build(_data, mid, next_split_dim);
			if(mid<ele_cnt-1)
				node->right = Build(_data+mid+1, ele_cnt-mid-1, next_split_dim);
			return node;
		}
		void FindNearest(Element& ret, const Element& ele, Node* node, double& min_dist)
		{
			if (node == nullptr)
				return;
			Element diff;
			for (int i(dim); i--;)
				diff.val[i] = ele.val[i] - node->val.val[i];
			double cur_dist = YXL::Dot<dim>(diff.val, diff.val);
			if (cur_dist < min_dist)
			{
				min_dist = cur_dist;
				ret = node->val;
			}
			FindNearest(ret, ele,
				(ele.val[node->split_dim] <= node->val.val[node->split_dim]) ? node->left : node->right,
				min_dist);
			double range = ele.val[node->split_dim] - node->val.val[node->split_dim];
			if (abs(range) > min_dist)
				return;
			FindNearest(ret, ele,
				(range < 0 ? node->right : node->left),
				min_dist);
		}

	private:
		Node* _root=nullptr;
	};
}
#endif

#ifdef _YXL_TIME_
#ifdef __linux__
#include <system.h>
#endif
namespace YXL
{
	inline std::string GetCurTime(const char* format_ymd_hms)
	{
		auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		struct tm* ptm = localtime(&tt);
		char date[60] = { 0 };
		sprintf(date, format_ymd_hms,
			(int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
			(int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
		return std::string(date);
	}

	inline std::string GetCurTime()
	{
		return GetCurTime("%04d%02d%02d_%02d%02d%02d");
	}

	inline void Sleep(double ms)
	{
#if defined(_WITH_WINDOWS_)
		::Sleep(ms);
#elif defined(__linux__)
		usleep(ms*1000.);
#else
		std::cout << "no sleep function" << std::endl;
#endif
	}

	template<typename type> double TimeElapsedMS(type start, type end)
	{
		using namespace std::chrono;
		auto d = duration_cast<microseconds>(end - start);
		return double(d.count()*1000.)*microseconds::period::num / microseconds::period::den;
	}

	class LIB_YXL_HELPER Timer
	{
	public:
		void Reset()
		{
			_str_t_start.clear();
			_int_t_start.clear();
			_str_acc.clear();
			_int_acc.clear();
			_last = std::chrono::high_resolution_clock::now();
		}

		void Start(const std::string& stamp)
		{
			_str_t_start[stamp] = std::chrono::high_resolution_clock::now();
		}
		void Start(const int stamp)
		{
			_int_t_start[stamp] = std::chrono::high_resolution_clock::now();
		}
		double End(const std::string& stamp)
		{
			if (_str_t_start.find(stamp) == _str_t_start.end())
				return 0.;
			return TimeElapsedMS(_str_t_start[stamp], std::chrono::high_resolution_clock::now());
		}
		double EndAverage(const std::string& stamp)
		{
			auto ret = End(stamp);
			return TimeElapsedAverage(ret, _str_acc[stamp]);
			
		}
		double End(const int stamp)
		{
			if (_int_t_start.find(stamp) == _int_t_start.end())
				return 0.;
			return TimeElapsedMS(_int_t_start[stamp], std::chrono::high_resolution_clock::now());
		}
		double EndAverage(const int stamp)
		{
			auto ret = End(stamp);
			return TimeElapsedAverage(ret, _int_acc[stamp]);
		}

		//time escape from last call
		double TimeEscape()
		{
			auto tmp = std::chrono::high_resolution_clock::now();
			auto ret = TimeElapsedMS(_last, tmp);
			_last = tmp;
			return ret;
		}

	private:
		double TimeElapsedAverage(const double cur_escape, std::pair<int, double>& history)
		{
			double ret = (history.first*history.second + cur_escape) / (history.first + 1);
			++history.first;
			history.second = ret;
			return ret;
		}

	private:
		std::map<std::string, std::chrono::time_point<std::chrono::high_resolution_clock> > _str_t_start;
		std::map<int, std::chrono::time_point<std::chrono::high_resolution_clock> > _int_t_start;

		std::map<std::string, std::pair<int, double> > _str_acc;
		std::map<int, std::pair<int, double> > _int_acc;

		std::chrono::time_point<std::chrono::high_resolution_clock> _last;
	};

	extern Timer g_timer;

	class LIB_YXL_HELPER FPSCounter
	{
	public:
		FPSCounter(const int measure_frame=1)
		{
			_measure_frame = measure_frame;
			_last = std::chrono::high_resolution_clock::now();
		}

		//calculate current FPS
		double CalcFPS()
		{
			auto cur = std::chrono::high_resolution_clock::now();
			auto ms = TimeElapsedMS(_last, cur);
			_last = cur;
			_pre_frame_time_ms.push_back(ms);
			_total += ms;
			
			if (_pre_frame_time_ms.size() > _measure_frame)
			{
				_total -= _pre_frame_time_ms.front();
				_pre_frame_time_ms.pop_front();
			}
			
			_fps = 1000.*_pre_frame_time_ms.size() / _total;
			return _fps;
		}

		//not update, get last calculated FPS
		double GetFPS()
		{
			return _fps;
		}

	private:
		int _measure_frame;
		double _total = 0.;
		double _fps = 0.;
		std::deque<double> _pre_frame_time_ms;
		std::chrono::time_point<std::chrono::high_resolution_clock> _last;

	};

	class LIB_YXL_HELPER FPSLimiter
	{
	public:
		FPSLimiter() {}
		FPSLimiter(const double fps)
		{
			SetFPS(fps);
		}
		void SetFPS(const double fps)
		{
			_fps = fps;
			_ms_per_frame = 1000. / fps;
		}

		void Start()
		{
			_last = std::chrono::high_resolution_clock::now();
		}
		void End()
		{
			auto ms = TimeElapsedMS(_last, std::chrono::high_resolution_clock::now());
			if (ms < _ms_per_frame)
				Sleep(_ms_per_frame - ms);
		}

	private:
		double _fps = 30.;
		double _ms_per_frame=1000./30.;
		std::chrono::time_point<std::chrono::high_resolution_clock> _last;
	};
}
#endif

#ifdef _YXL_CONSOLE_
namespace YXL
{
	namespace Console
	{
		inline void ClearLine(const int idx)
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;

			COORD pos;
			pos.X = 0;
			pos.Y = idx;
			DWORD written(0);
			FillConsoleOutputCharacterA(hStdout, ' ', csbiInfo.dwSize.X, pos, &written);
		}
		inline void ClearCurrentLine()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;
			COORD pos = csbiInfo.dwCursorPosition;
			pos.X = 0;
			DWORD written(0);
			FillConsoleOutputCharacterA(hStdout, ' ', csbiInfo.dwSize.X, pos, &written);
		}
		inline void ClearAll()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;

			COORD line;
			line.X = 0;
			line.Y = 0;
			DWORD written(0);
			FillConsoleOutputCharacterA(hStdout, ' ', csbiInfo.dwSize.X*csbiInfo.dwSize.Y, line, &written);
		}
		inline void RedirectTo(const int x, const int y)
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return;

			COORD pos;
			pos.Y = x;
			pos.X = y;
			if (!SetConsoleCursorPosition(hStdout, pos))
				return;
		}

		inline void RedirectToFirstLine(bool is_clear=true)
		{
			if(is_clear)
				ClearAll();
			RedirectTo(0, 0);
		}
		
		inline int CurrentLine()
		{
			CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
			HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
			if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo))
				return -1;
			return csbiInfo.dwCursorPosition.Y;
		}
	}
}
#endif

#ifdef _YXL_TRANSFORM_
namespace YXL
{
	template<typename type> void Cross(type* v, const type* v0, const type* v1)
	{
		v[0] = v0[1] * v1[2] - v0[2] * v1[1];
		v[1] = v0[2] * v1[0] - v0[0] * v1[2];
		v[2] = v0[0] * v1[1] - v0[1] * v1[0];
	}
	template<int len, typename type> type Dot(const type* v0, const type* v1)
	{
		type ret = 0;
		for (int i(len); i--;)
			ret += v0[i] * v1[i];
		return ret;
	}
	template<int len, typename type> void Normalize(type* v, const type* v0)
	{
		type norm = static_cast<type>(1) / sqrt(Dot<len>(v0, v0));
		for (int i(len); i--; )
			v[i] = v0[i] * norm;
	}

	namespace Quat
	{
		template<typename type> void Rotate(type* out, const type* v, const type* quat)
		{
			float a[3], b[3];
			Cross(a, quat, v);
			Cross(b, quat, a);
			out[0] = v[0] + static_cast<type>(2)*(a[0] * quat[3] + b[0]);
			out[1] = v[1] + static_cast<type>(2)*(a[1] * quat[3] + b[1]);
			out[2] = v[2] + static_cast<type>(2)*(a[2] * quat[3] + b[2]);
		}
		template<typename type> void ToQuaternion(type* out, const type* axis, const type angle_degree)
		{
			type angle = angle_degree*(3.1415926*0.5 / 180);
			type c = cos(angle);
			type s = sin(angle);

			type norm = static_cast<type>(1) / Dot<3>(axis, axis);

			out[0] = axis[0] * norm*s;
			out[1] = axis[1] * norm*s;
			out[2] = axis[2] * norm*s;
			out[3] = c;
		}
	}

	namespace Mat
	{
		template<typename type> void RotationPoint(type* pt_ret, type* pt, type* rot)
		{
			if (pt_ret == pt)
			{
				type x = pt[0], y = pt[1], z = pt[2];
				pt_ret[0] = x*rot[0] + y*rot[4] + z*rot[8];
				pt_ret[1] = x*rot[1] + y*rot[5] + z*rot[9];
				pt_ret[2] = x*rot[2] + y*rot[6] + z*rot[10];
			}
			else
			{
				pt_ret[0] = pt[0]*rot[0] + pt[1]*rot[4] + pt[2]*rot[8];
				pt_ret[1] = pt[0]*rot[1] + pt[1]*rot[5] + pt[2]*rot[9];
				pt_ret[2] = pt[0]*rot[2] + pt[1]*rot[6] + pt[2]*rot[10];
			}
		}

		template<typename type> void Perspective(type* ret, type aspect, type z_near, type z_far, type fov)
		{
			type tan_half_fov = tan(fov*0.5f / 180.0f*3.1415926f);

			memset(ret, 0, sizeof(type) * 16);
			ret[0] = 1.0f / (tan_half_fov*aspect);
			ret[5] = 1.0f / tan_half_fov;
			ret[10] = -(z_near + z_far) / (z_far - z_near);
			ret[11] = -1.0f;
			ret[14] = -2.0f*z_far*z_near / (z_far - z_near);

			/*
			{
			1.0f / (tan_half_fov*aspect), 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f / tan_half_fov, 0.0f, 0.0f,
			0.0f, 0.0f, -(z_near + z_far) / (z_far - z_near), -1.0f,
			0.0f, 0.0f, -2.0f*z_far*z_near / (z_far - z_near), 0.0f
			};
			*/
		}

		template<typename type> void LookAt(type* ret, type* eye, type* at, type* up)
		{
			type f[] = { at[0] - eye[0], at[1] - eye[1], at[2] - eye[2] };
			Normalize<3>(f, f);

			type s[3];
			Cross(s, f, up);
			Normalize<3>(s, s);

			type u[3];
			Cross(u, s, f);

			ret[0] = s[0];
			ret[1] = s[1];
			ret[2] = s[2];
			ret[3] = 0;
			ret[4] = u[0];
			ret[5] = u[1];
			ret[6] = u[2];
			ret[7] = 0;
			ret[8] = f[0];
			ret[9] = f[1];
			ret[10] = f[2];
			ret[11] = 0;
			ret[12] = -Dot<3>(s, eye);
			ret[13] = -Dot<3>(u, eye);
			ret[14] = Dot<3>(f, eye);
			ret[15] = 1.0f;

			/*float data[] = {
			s[0], s[1], s[2], 0.0f,
			u[0], u[1], u[2], 0.0f,
			f[0], f[1], f[2], 0.0f,
			-s.dot(eye), -u.dot(eye), f.dot(eye), 1.0f
			};*/
		}

		template<typename type> void RotationX(type* ret, type angle)
		{
			memset(ret, 0, sizeof(type) * 16);
			float theta = angle*3.1415926f / 180.0f;
			ret[0] = 1.f;
			ret[5] = cos(theta);
			ret[6] = -sin(theta);
			ret[9] = sin(theta);
			ret[10] = cos(theta);
			ret[15] = 1.f;
			/*
			{
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, cos(theta), -sin(theta), 0.0f,
			0.0f, sin(theta), cos(theta), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			};
			*/
		}
		template<typename type> void RotationY(type* ret, type angle)
		{
			memset(ret, 0, sizeof(type) * 16);
			float theta = angle*3.1415926f / 180.0f;
			ret[0] = cos(theta);
			ret[2] = -sin(theta);
			ret[5] = 1.f;
			ret[8] = sin(theta);
			ret[10] = cos(theta);
			ret[15] = 1.f;
			/*
			{
			cos(theta), 0.0f, -sin(theta), 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			sin(theta), 0.0f, cos(theta), 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			};
			*/
		}
		template<typename type> void RotationZ(type* ret, type angle)
		{
			memset(ret, 0, sizeof(type) * 16);
			float theta = angle*3.1415926f / 180.0f;
			ret[0] = cos(theta);
			ret[1] = -sin(theta);
			ret[4] = sin(theta);
			ret[5] = cos(theta);
			ret[10] = 1.f;
			ret[15] = 1.f;
			/*
			{
			cos(theta), -sin(theta), 0.0f, 0.0f,
			sin(theta), cos(theta), 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
			};
			*/
		}
		template<typename type> void Scale(type* ret, type* scale_xyz)
		{
			memset(ret, 0, sizeof(type) * 16);
			ret[0] = scale_xyz[0];
			ret[5] = scale_xyz[1];
			ret[10] = scale_xyz[2];
			ret[15] = 1.0f;
		}
		template<typename type> void Translate(type* ret, type* trans_xyz)
		{
			memset(ret, 0, sizeof(type) * 16);
			ret[0] = 1.0f;
			ret[5] = 1.0f;
			ret[10] = 1.0f;
			ret[12] = trans_xyz[0];
			ret[13] = trans_xyz[1];
			ret[14] = trans_xyz[2];
			ret[15] = 1.0f;
		}


#ifdef _WITH_OPENCV_
		inline cv::Mat Perspective(float aspect, float z_near, float z_far, float fov)
		{
			float proj_mat_data[16];
			Perspective(proj_mat_data, aspect, z_near, z_far, fov);
			return cv::Mat(4, 4, CV_32FC1, proj_mat_data).clone();
		}

		inline cv::Mat LookAt(cv::Vec3f eye, cv::Vec3f at, cv::Vec3f up)
		{
			float data[16];
			LookAt(data, eye.val, at.val, up.val);
			return cv::Mat(4, 4, CV_32FC1, data).clone();
		}

		inline cv::Mat Rotation(cv::Vec3f rot_xyz)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			float tmp[16];
			if (rot_xyz[0] != 0.0)
			{
				RotationX(tmp, rot_xyz[0]);
				ret *= cv::Mat(4, 4, CV_32FC1, tmp);
			}
			if (rot_xyz[1] != 0.0)
			{
				RotationY(tmp, rot_xyz[1]);
				ret *= cv::Mat(4, 4, CV_32FC1, tmp);
			}
			if (rot_xyz[2] != 0.0)
			{
				RotationZ(tmp, rot_xyz[2]);
				ret *= cv::Mat(4, 4, CV_32FC1, tmp);
			}

			return ret;
		}

		inline cv::Mat Scale(cv::Vec3f scale_xyz)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			Scale((float*)ret.data, scale_xyz.val);
			return ret;
		}

		inline cv::Mat Translate(cv::Vec3f trans_xyz)
		{
			cv::Mat ret = cv::Mat::eye(4, 4, CV_32FC1);
			Translate((float*)ret.data, trans_xyz.val);
			return ret;
		}
#endif
	}
}
#endif

#ifdef _YXL_GRAPHIC_
namespace YXL
{
	template<typename v_type, typename i_type> void ComputeNormal(v_type* normals, 
		v_type* vertices, const int v_cnt, 
		i_type* tris, const int tri_cnt, const bool is_reverse_normal)
	{
		memset(normals, 0, sizeof(v_type)*v_cnt * 3);
		i_type idx[3];
		int tmp;
		v_type a[3], b[3], c[3], l2[3], face_normal[3];
		for (int i(0); i!= tri_cnt; ++i)
		{
			tmp = i * 3;
			idx[0] = tris[tmp + (is_reverse_normal ? 2 : 0)]*3;
			idx[1] = tris[tmp + 1]*3;
			idx[2] = tris[tmp + (is_reverse_normal ? 0 : 2)]*3;

			a[0] = vertices[idx[0]] - vertices[idx[1]];
			a[1] = vertices[idx[0] + 1] - vertices[idx[1] + 1];
			a[2] = vertices[idx[0] + 2] - vertices[idx[1] + 2];
			b[0] = vertices[idx[1]] - vertices[idx[2]];
			b[1] = vertices[idx[1] + 1] - vertices[idx[2] + 1];
			b[2] = vertices[idx[1] + 2] - vertices[idx[2] + 2];
			c[0] = vertices[idx[2]] - vertices[idx[0]];
			c[1] = vertices[idx[2] + 1] - vertices[idx[0] + 1];
			c[2] = vertices[idx[2] + 2] - vertices[idx[0] + 2];
			
			l2[0] = Dot<3>(a, a);
			l2[1] = Dot<3>(b, b);
			l2[2] = Dot<3>(c, c);
			Cross(face_normal, a, b);

			a[0] = 1.f / (l2[0] * l2[2]);
			a[1] = 1.f / (l2[0] * l2[1]);
			a[2] = 1.f / (l2[1] * l2[2]);

			tmp = idx[0];
			normals[tmp] += face_normal[0] * a[0];
			normals[tmp+1] += face_normal[1] * a[0];
			normals[tmp+2] += face_normal[2] * a[0];
			tmp = idx[1];
			normals[tmp] += face_normal[0] * a[1];
			normals[tmp + 1] += face_normal[1] * a[1];
			normals[tmp + 2] += face_normal[2] * a[1];
			tmp = idx[2];
			normals[tmp] += face_normal[0] * a[2];
			normals[tmp + 1] += face_normal[1] * a[2];
			normals[tmp + 2] += face_normal[2] * a[2];
		}
		for (int i(v_cnt); i--;)
			Normalize<3>(normals + i * 3, normals + i * 3);
	}

#ifdef _WITH_OPENCV_
	inline cv::Mat ComputeNormal(cv::Mat vertices, cv::Mat tris, bool is_reverse_normal)
	{
		cv::Mat normals = cv::Mat(vertices.size(), vertices.type());
		ComputeNormal((float*)normals.data, (float*)vertices.data, vertices.rows, (int*)tris.data, tris.rows, is_reverse_normal);
		return normals;
	}

#endif
}
#endif

#ifdef _YXL_IMG_PROC_
namespace YXL
{
	cv::Mat LIB_YXL_HELPER FilterImage(cv::Mat img, cv::Mat kernel, cv::Mat mask = cv::Mat());
}
#endif

#ifdef _YXL_GLFW_
namespace YXL
{
	class LIB_YXL_HELPER GLFWBase
	{
	public:
		virtual ~GLFWBase()
		{
			glfwDestroyWindow(_wnd);
		}
		virtual bool Init(const int wnd_w=1280, const int wnd_h=720, const bool hidden=false);
		virtual void Run();
		virtual void CleanUp() {}
		virtual void BeforeFrame(const int frame_id) {}
		virtual void AfterFrame(const int frame_id) {}
		virtual void Frame(const int frame_id) = 0;
		virtual void KeyCallback(int key, int scancode, int action, int mods) 
		{
			if (action == GLFW_PRESS)
			{
				switch (key)
				{
				case GLFW_KEY_ESCAPE:
					glfwSetWindowShouldClose(_wnd, GLFW_TRUE);
					break;
				default:
					break;
				}
			}
		}
		virtual void MouseButtonCallback(int button, int action, int mods)
		{
			//if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
			//	//do something
		}
		virtual void ScrollCallback(double xoffset, double yoffset)
		{

		}
		virtual void SetWindowTitle(const std::string title)
		{
			glfwSetWindowTitle(_wnd, title.c_str());
		}

	protected:
		GLFWwindow* _wnd = nullptr;

	private:
		int _frame_id = 0;
	};
}
#endif

#endif