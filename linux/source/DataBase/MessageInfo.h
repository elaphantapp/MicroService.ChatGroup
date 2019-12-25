//
// Created by luocf on 2019/6/15.
//

#ifndef CHATGROUP_MESSAGEINFO_H
#define CHATGROUP_MESSAGEINFO_H


#include <string>
#include <ctime>
#include <memory>

namespace chatgroup {
    class MessageInfo {
    public:
        MessageInfo();
        MessageInfo(const std::string& friend_id, const std::string& message, std::time_t send_time);
        std::string mFriendid;
        std::string mMsg;
        std::time_t mSendTimeStamp;
    };
}

#endif //CHATGROUP_MESSAGEINFO_H
