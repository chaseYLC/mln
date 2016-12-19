#pragma once

#include "connection.h"
#include <Base/Base/include/messageBuffer.h>

namespace MLN
{
	namespace Net
	{
		class NetService;
		class Connection;

		class EventReceiver
		{
		public:
			virtual void onAccept(Connection::ptr spConn) = 0;
			virtual void onConnect(Connection::ptr spConn) = 0;
			virtual void onClose(Connection::ptr spConn) = 0;
			virtual void onUpdate(unsigned long elapse) = 0;
			virtual void noHandler(Connection::ptr spConn, Base::MessageBuffer& msg) = 0;
			virtual void onExpiredSession(Connection::ptr spConn) = 0;

			virtual void onAcceptFailed(Connection::ptr spConn) = 0;
			virtual void onConnectFailed(Connection::ptr spConn) = 0;
			virtual void onCloseFailed(Connection::ptr spConn) = 0;

			NetService* getOwner() const{
				return _owner; 
			}

			void setSession(Connection::ptr session){
				m_session = session;
			}

			boost::weak_ptr<Net::Connection> getSession() const{
				return m_session;
			}


		protected:
			virtual void initHandler(){}

		protected:
			NetService* _owner;
			friend class NetService;

			boost::weak_ptr<Net::Connection> m_session;
		};
	};//namespace Net
};//namespace MLN
