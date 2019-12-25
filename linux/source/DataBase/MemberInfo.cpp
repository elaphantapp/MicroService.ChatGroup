//
// Created by luocf on 2019/6/15.
//
#include <string>
#include <sstream>
#include <iostream>
#include <Tools/Log.hpp>
#include "MemberInfo.h"

namespace chatgroup {
    int MemberInfo::sMemberCount = 0;

    MemberInfo::MemberInfo(const std::string& friendid,
                           const std::string& nickname,
                           int status,
                           std::time_t msg_timestamp) :
            mFriendid(friendid), mNickName(nickname),
            mStatus(status), mMsgTimeStamp(msg_timestamp) {
        mIndex = ++sMemberCount;
        if (nickname.empty() == true) {
            if (mIndex == 1) {
                mNickName = "群主";
            } else {
                mNickName = "匿名" + std::to_string(mIndex);
            }
        }
    }

    void MemberInfo::Lock() {
        mMutex.lock();
    }

    void MemberInfo::UnLock() {
        mMutex.unlock();
    }
}