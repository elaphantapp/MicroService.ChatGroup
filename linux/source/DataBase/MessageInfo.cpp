//
// Created by luocf on 2019/6/15.
//

#include "MessageInfo.h"
namespace chatgroup {
    MessageInfo::MessageInfo() {

    }
    MessageInfo::MessageInfo(const std::string& friendid, const std::string& message,
                           std::time_t send_time):
            mFriendid(friendid), mMsg(message),mSendTimeStamp(send_time){

    }
}