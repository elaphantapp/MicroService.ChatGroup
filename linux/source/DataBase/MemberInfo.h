//
// Created by luocf on 2019/6/15.
//

#ifndef CHATGROUP_MEMBERINFO_H
#define CHATGROUP_MEMBERINFO_H


#include <memory>
#include <mutex>
#include <ctime>

namespace chatgroup {

    class MemberInfo {
    public:
        static int sMemberCount;
        MemberInfo(const std::string&, const std::string&,  int status, std::time_t msg_timestamp);
        std::string mFriendid;
        std::string mNickName;
        std::time_t mMsgTimeStamp;
        int mStatus;
        int mIndex;
        bool mIsManager;
        void Lock();
        void UnLock();
    private:
        std::recursive_mutex mMutex;
    };
}


#endif //CHATGROUP_MEMBERINFO_H
