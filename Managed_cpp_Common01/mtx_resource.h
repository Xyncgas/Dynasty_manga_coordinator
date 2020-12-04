#pragma once
namespace LF
{
	template<typename _T>
	struct mtx_resource {
		std::mutex mtx;
		_T react;
	};
}