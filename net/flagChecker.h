#pragma once

#include <stdint.h>

// usages.
//enum class MLN_PERMISSION{
//	JOB_A,
//	JOB_B,
//	JOB_C,
//};
//
//mln::FlagChecker<MLN_PERMISSION> mln_permission;
//mln_permission.addFlags(
//	MLN_PERMISSION::JOB_B
//	, MLN_PERMISSION::JOB_C
//	);
//
//bool hasPermission = false;
//assert(false == mln_permission.isUsingFlag(MLN_PERMISSION::JOB_A));
//assert(true == mln_permission.isUsingFlag(MLN_PERMISSION::JOB_B));
//assert(true == mln_permission.isUsingFlag(MLN_PERMISSION::JOB_C));



namespace mln
{
	template <typename FLAG_DEFINE_CLASS>
	class FlagChecker
	{
	public:
		template <typename... ARGS>
		void addFlags(ARGS... flags)
		{
			assert(sizeof(m_flagContainer)* 8 > sizeof...(flags));

			addFlag(flags...);
		}

		bool isUsingFlag(const FLAG_DEFINE_CLASS flag) const{
			return 0 != (m_flagContainer & getFlagValue(flag));
		}

		uint64_t addFlag(const FLAG_DEFINE_CLASS flag){
			if (false == isUsingFlag(flag)) {
				m_flagContainer += getFlagValue(flag);
			}
			return m_flagContainer;
		}

		uint64_t subFlag(const FLAG_DEFINE_CLASS flag){
			if (true == isUsingFlag(flag)) {
				m_flagContainer -= getFlagValue(flag);
			}
			return m_flagContainer;
		}

	private:
		template <typename... ARGS>
		inline void addFlag(const FLAG_DEFINE_CLASS flag, ARGS&&... remainFlagList)
		{
			addFlag(flag);
			addFlag(remainFlagList...);
		}

		

		uint64_t getFlagValue(const FLAG_DEFINE_CLASS flag) const
		{
			return ((uint64_t)1) << (unsigned)flag;
		}

	private:
		uint64_t m_flagContainer = 0;
	};

};//namespace mln