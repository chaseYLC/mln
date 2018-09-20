#ifndef _MLN_NET_STDAFX_H_
#define _MLN_NET_STDAFX_H_

#pragma warning ( disable: 4819 )
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/bind.hpp>
#include <boost/chrono.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/pool/pool.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>
#pragma warning ( default : 4819 )

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>


#define _XMLNSTR(x) L ## x



#endif