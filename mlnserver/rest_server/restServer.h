#pragma once

#include <net/singleton.h>

namespace mlnserver {
	class TRestServer
		: public mln::SingletonLight<TRestServer>
	{
	public:
		void Start(const int port);

	private:
		void Accept(const int port);
	};
}
