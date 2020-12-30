#pragma once

#include <net/singleton.h>

namespace mlnserver {
	class RestServer
		: public mln::SingletonLight<RestServer>
	{
	public:
		void Start(const int port);

	private:
		void Accept(const int port);
	};
}
