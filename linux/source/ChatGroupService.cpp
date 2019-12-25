//
// Created by luocf on 2019/6/13.
//
#include <cstring>
#include <future>
#include <stack>
#include <iostream>
#include <ctime>

using namespace std;
#include <thread>
#include <chrono>
#include <Tools/Log.hpp>
#include "Json.hpp"
#include <ThirdParty/CompatibleFileSystem.hpp>
#include <Command/ChatGroupCmd.hpp>
#include "ChatGroupService.h"
#include "ErrCode.h"
namespace elastos {
    /***********************************************/
    /***** static function implement ***************/
    /***********************************************/
    ChatGroupService::ChatGroupService(const std::string& path)
            : mPath(path), mCreaterFriendId(""){
        mDatabaseProxy = new chatgroup::DatabaseProxy();
        //anypeer Id format
        std::string reg_str("(\\w{8})-(\\w{4})-(\\w{4})-(\\w{4})-(\\w{13})");
        mMsgReg = std::make_shared<std::regex>(reg_str, std::regex::icase);
        mStatus = 0;
        mLocalPath = path;
        mDatabaseProxy->startDb(path.c_str());
        mConnector = new Connector(ChatGroupService_TAG);
        this->Start();
    }
    
    ChatGroupService::~ChatGroupService() {
        this->Stop();
    }

    int ChatGroupService::Start() {
        if (mConnector == NULL) return -1;
        printf("Service start!\n");
        std::shared_ptr<PeerListener::MessageListener> message_listener = std::make_shared<ChatGroupMessageListener>(this);
        mConnector->SetMessageListener(message_listener);
        std::shared_ptr<chatgroup::MemberInfo> member_info = mDatabaseProxy->getMemberInfo(1);
        if (member_info.get() != nullptr) {
            mCreaterFriendId = member_info->mFriendid;
            printf("ChatGroupService Start mCreaterFriendId: %s\n",mCreaterFriendId.c_str());
        }
        int status = PeerNode::GetInstance()->GetStatus();
        printf("ChatGroupService Start status: %d\n",status);
        std::shared_ptr<ElaphantContact::UserInfo> user_info = mConnector->GetUserInfo();
        if (user_info.get() != NULL) {
            user_info->getHumanCode(mOwnerHumanCode);
            printf("Service start mOwnerHumanCode:%s\n", mOwnerHumanCode.c_str());
        }
        return 0;
    }

    int ChatGroupService::Stop() {
        mDatabaseProxy->closeDb();
        if (mConnector == NULL) return -1;
        printf("Service stop!\n");
        return 0;
    }
    std::time_t ChatGroupService::getTimeStamp() {
        return time(0);
    }

    bool ChatGroupService::inRemovedList(const std::string& friendid) {
        return mDatabaseProxy->inRemovedList(friendid);
    }

    void ChatGroupService::updateMemberInfo(const std::string& friendid,
                                            const std::string& nickname,
                                            chatgroup::ConnectionStatus status) {
        if (mCreaterFriendId.empty()) {
            mCreaterFriendId = friendid;
        }

        std::shared_ptr<chatgroup::MemberInfo> block_member_info = mDatabaseProxy->getBlockMemberInfo(friendid);
        if (block_member_info.get() == nullptr) {
            mDatabaseProxy->updateMemberInfo(friendid, nickname, status, 0);
            //当前状态为上线时，获取该成员offline以后的所以消息，并发送给该人,
            if (status == chatgroup::ConnectionStatus_OnLine) {
                relayMessages();
            }
        }
    }

    bool ChatGroupService::relayMessages() {
        chatgroup::MUTEX_LOCKER locker_sync_data(_mReplyMessage);
        std::shared_ptr<std::vector<std::shared_ptr<chatgroup::MemberInfo>>> memberlist = mDatabaseProxy->getFriendList();
        for (int i = 0; i < memberlist->size(); i++) {
            std::shared_ptr<chatgroup::MemberInfo> memberInfo = memberlist->at(i);
            if (memberInfo.get() == nullptr) {
                continue;
            }
            bool info_changed = false;
            memberInfo->Lock();
            if (memberInfo->mStatus != 0) {
                memberInfo->UnLock();
                continue;
            }
            std::shared_ptr<std::vector<std::shared_ptr<chatgroup::MessageInfo>>> message_list = mDatabaseProxy->getMessages(
                    memberInfo->mFriendid, memberInfo->mMsgTimeStamp, 100);
            if (message_list.get() != nullptr) {
                printf("relayMessages mFriendid:%s, message_list->size():%ld\n",
                       memberInfo->mFriendid.c_str(), message_list->size());
                for (int i = 0; i < message_list->size(); i++) {
                    std::shared_ptr<chatgroup::MessageInfo> message = message_list->at(i);
                    if (message.get() != nullptr) {
                        std::shared_ptr<chatgroup::MemberInfo> target_memberInfo = mDatabaseProxy->getMemberInfo(
                                message->mFriendid);
                        Json respJson;
                        respJson["serviceName"] = ChatGroupService_TAG;
                        respJson["type"] = "textMsg";
                        if (target_memberInfo.get() != nullptr) {
                            target_memberInfo->Lock();
                            respJson["content"] = message->mMsg;
                            respJson["nickName"] = target_memberInfo->mNickName;
                            respJson["timeStamp"] = this->convertDatetimeToString(message->mSendTimeStamp);
                            target_memberInfo->UnLock();
                        }
                        int msg_ret = mConnector->SendMessage(memberInfo->mFriendid, respJson.dump());
                        printf("relayMessages mFriendid:%s, response:%s, msg_ret:%d\n",
                                memberInfo->mFriendid.c_str(), respJson.dump().c_str(), msg_ret);
                        if (msg_ret != 0) {
                            break;
                        }
                        info_changed = true;
                        memberInfo->mMsgTimeStamp = message->mSendTimeStamp;
                    }
                }
            }
            memberInfo->UnLock();
            if (info_changed) {
                mDatabaseProxy->updateMemberInfo(memberInfo->mFriendid, memberInfo->mNickName,
                        memberInfo->mStatus==0?chatgroup::ConnectionStatus_OnLine:chatgroup::ConnectionStatus_OffLine,
                        memberInfo->mMsgTimeStamp);
            }

        }
        return false;
    }

    void ChatGroupService::addMessage(const std::string& friend_id, const std::string& message, std::time_t send_time) {
        std::string errMsg;
        std::string msg = message;
        std::string pre_cmd = msg + " " + friend_id;//Pretreatment cmd
        int ret = ChatGroupCmd::Do(this, pre_cmd, errMsg);
        if (ret < 0) {
            //save message
            mDatabaseProxy->addMessage(friend_id, msg, send_time);
            //将该消息转发给其他人
            relayMessages();
        }
    }

    std::shared_ptr<std::vector<std::shared_ptr<chatgroup::MemberInfo>>> ChatGroupService::getFriendList() {
        return mDatabaseProxy->getFriendList();
    }

    int ChatGroupService::acceptFriend(const std::string& friendid) {
        std::shared_ptr<chatgroup::MemberInfo> member_info = mDatabaseProxy->getBlockMemberInfo(friendid);
        if (member_info.get() == nullptr) {
            mDatabaseProxy->deleteRemovedListMember(friendid);
            mConnector->AcceptFriend(friendid);
        } else {
            Log::D(ChatGroupService_TAG, "acceptFriend can't accept friend, block member:%s", friendid.c_str());
        }

        return 0;
    }

    std::string ChatGroupService::convertDatetimeToString(std::time_t time) {
        tm *tm_ = localtime(&time);                // 将time_t格式转换为tm结构体
        int year, month, day, hour, minute, second;// 定义时间的各个int临时变量。
        year = tm_->tm_year +
               1900;                // 临时变量，年，由于tm结构体存储的是从1900年开始的时间，所以临时变量int为tm_year加上1900。
        month = tm_->tm_mon +
                1;                   // 临时变量，月，由于tm结构体的月份存储范围为0-11，所以临时变量int为tm_mon加上1。
        day = tm_->tm_mday;                        // 临时变量，日。
        hour = tm_->tm_hour;                       // 临时变量，时。
        minute = tm_->tm_min;                      // 临时变量，分。
        second = tm_->tm_sec;                      // 临时变量，秒。
        char yearStr[5], monthStr[3], dayStr[3], hourStr[3], minuteStr[3], secondStr[3];// 定义时间的各个char*变量。
        sprintf(yearStr, "%d", year);              // 年。
        sprintf(monthStr, "%d", month);            // 月。
        sprintf(dayStr, "%d", day);                // 日。
        sprintf(hourStr, "%d", hour);              // 时。
        sprintf(minuteStr, "%d", minute);          // 分。
        if (minuteStr[1] == '\0')                  // 如果分为一位，如5，则需要转换字符串为两位，如05。
        {
            minuteStr[2] = '\0';
            minuteStr[1] = minuteStr[0];
            minuteStr[0] = '0';
        }
        sprintf(secondStr, "%d", second);          // 秒。
        if (secondStr[1] == '\0')                  // 如果秒为一位，如5，则需要转换字符串为两位，如05。
        {
            secondStr[2] = '\0';
            secondStr[1] = secondStr[0];
            secondStr[0] = '0';
        }
        char s[20];                                // 定义总日期时间char*变量。
        sprintf(s, "%s-%s-%s %s:%s:%s", yearStr, monthStr, dayStr, hourStr, minuteStr,
                secondStr);// 将年月日时分秒合并。
        std::string str(s);                             // 定义string变量，并将总日期时间char*变量作为构造函数的参数传入。
        return str;                                // 返回转换日期时间后的string变量。
    }

    void ChatGroupService::helpCmd(const std::vector<std::string> &args, const std::string &message) {
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            Json respJson;
            respJson["serviceName"] = ChatGroupService_TAG;
            respJson["type"] = "textMsg";
            respJson["content"] = message;
            int ret = mConnector->SendMessage(friend_id, respJson.dump());
            if (ret != 0) {
                Log::I(ChatGroupService_TAG,
                       "helpCmd .c_str(): %s errno:(0x%x)",
                       message.c_str());
            }
        }
    }

    void ChatGroupService::listCmd(const std::vector<std::string> &args) {
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            std::shared_ptr<std::vector<std::shared_ptr<chatgroup::MemberInfo>>> memberlist = mDatabaseProxy->getFriendList();
            std::string ret_msg_str = "";
            for (int i = 0; i < memberlist->size(); i++) {
                std::shared_ptr<chatgroup::MemberInfo> memberInfo = memberlist->at(i);
                if (memberInfo.get() == nullptr) {
                    continue;
                }
                memberInfo->Lock();
                ret_msg_str += std::string(
                        std::to_string(memberInfo->mIndex) + ": " +
                        memberInfo->mNickName + " " +
                        std::string(memberInfo->mStatus == 0 ? "online" : "offline") + "\n");
                memberInfo->UnLock();//同名还没处理
            }

            Json respJson;
            respJson["serviceName"] = ChatGroupService_TAG;
            respJson["type"] = "textMsg";
            respJson["content"] = ret_msg_str;
            int ret = mConnector->SendMessage(friend_id, respJson.dump());
            if (ret != 0) {
                Log::I(ChatGroupService_TAG,
                       "listCmd .c_str(): %s errno:(0x%x)",
                       ret_msg_str.c_str());
            }
        }
    }

    int ChatGroupService::getGroupStatus() {
        return mStatus;
    }

    std::string ChatGroupService::getGroupNickName() {
        std::string nick_name = mDatabaseProxy->getGroupNickName();
        return nick_name;
    }

    void ChatGroupService::updateNickNameCmd(const std::vector<std::string> &args) {
        chatgroup::MUTEX_LOCKER locker_sync_data(_mReplyMessage);
        if (args.size() >= 3) {
            const std::string friend_id = args[2];
            if (mCreaterFriendId.compare(friend_id) == 0) {
                const std::string nick_name = args[1];
                mDatabaseProxy->updateGroupNickName(nick_name);
            }
        }
    }

    void ChatGroupService::deleteGroupCmd(const std::vector<std::string> &args) {
        chatgroup::MUTEX_LOCKER locker_sync_data(_mReplyMessage);
        if (args.size() >= 2) {
            const std::string friend_id = args[1];
            if (mCreaterFriendId.compare((friend_id)) == 0) {
                mStatus = -1;
                //好友解除关系
                std::shared_ptr<std::vector<std::shared_ptr<chatgroup::MemberInfo>>> memberlist = mDatabaseProxy->getFriendList();
                for (int i = 0; i < memberlist->size(); i++) {
                    std::shared_ptr<chatgroup::MemberInfo> memberInfo = memberlist->at(i);
                    if (memberInfo.get() == nullptr) {
                        continue;
                    }
                    memberInfo->Lock();
                    int ret = mConnector->RemoveFriend(memberInfo->mFriendid);
                    if (ret != 0) {
                        Log::I(ChatGroupService_TAG,
                               "deleteGroupCmd can't delete this user: %s",
                               memberInfo->mFriendid.c_str());
                    }
                    memberInfo->UnLock();
                }
            }
        }
    }

    void ChatGroupService::blockFriendCmd(const std::vector<std::string> &args) {
        if (args.size() >= 3) {
            const std::string friend_id = args[2];
            if (mCreaterFriendId.compare((friend_id)) == 0) {
                std::string del_userindex = args[1];
                int user_num = std::atoi(del_userindex.c_str());
                std::shared_ptr<chatgroup::MemberInfo> memberInfo = mDatabaseProxy->getMemberInfo(user_num);
                char msg_str[256];
                if (memberInfo.get() != nullptr) {
                    memberInfo->Lock();
                    std::string del_friend_id = memberInfo->mFriendid;
                    memberInfo->UnLock();
                    mDatabaseProxy->addBlockMember(memberInfo);
                    mDatabaseProxy->removeMember(del_friend_id);

                    int ret = mConnector->RemoveFriend(memberInfo->mFriendid);
                    if (ret != 0) {
                        Log::I(ChatGroupService_TAG,
                               "delCmd can't delete this user: %s",
                               memberInfo->mFriendid.c_str());
                    }
                } else {
                    sprintf(msg_str, "num %s member not exist!", del_userindex.c_str());
                }
                Json respJson;
                respJson["serviceName"] = ChatGroupService_TAG;
                respJson["type"] = "textMsg";
                respJson["content"] = msg_str;
                mConnector->SendMessage(friend_id, respJson.dump());
            }
        }
    }

    void ChatGroupService::delCmd(const std::vector<std::string> &args) {
        if (args.size() >= 3) {
            const std::string friend_id = args[2];
            if (mCreaterFriendId.compare((friend_id)) == 0) {
                std::string del_userindex = args[1];
                int user_num = std::atoi(del_userindex.c_str());
                std::shared_ptr<chatgroup::MemberInfo> memberInfo = mDatabaseProxy->getMemberInfo(user_num);
                char msg_str[256];
                if (memberInfo.get() != nullptr) {
                    memberInfo->Lock();
                    std::string del_friend_id = memberInfo->mFriendid;
                    memberInfo->UnLock();
                    mDatabaseProxy->removeMember(del_friend_id);
                    int ret = mConnector->RemoveFriend(memberInfo->mFriendid);
                    if (ret != 0) {
                        Log::I(ChatGroupService_TAG,
                               "deleteGroupCmd can't delete this user: %s",
                               memberInfo->mFriendid.c_str());
                    }
                } else {
                    sprintf(msg_str, "num %s member not exist!", del_userindex.c_str());
                }
                Json respJson;
                respJson["serviceName"] = ChatGroupService_TAG;
                respJson["type"] = "textMsg";
                respJson["content"] = msg_str;
                mConnector->SendMessage(friend_id, respJson.dump());
            }
        }
    }

    ChatGroupMessageListener::ChatGroupMessageListener(ChatGroupService* chatGroupService) {
        mChatGroupService = chatGroupService;
    }

    ChatGroupMessageListener::~ChatGroupMessageListener() {

    }

    void ChatGroupMessageListener::onEvent(ContactListener::EventArgs& event) {
        Log::W(ChatGroupService_TAG, "onEvent type: %d\n", event.type);
        switch (event.type) {
            case ElaphantContact::Listener::EventType::FriendRequest: {
                auto friendEvent = dynamic_cast<ElaphantContact::Listener::RequestEvent*>(&event);
                Log::W(ChatGroupService_TAG, "FriendRequest from: %s\n", friendEvent->humanCode.c_str());
                mChatGroupService->acceptFriend(friendEvent->humanCode);
                break;
            }
            case ElaphantContact::Listener::EventType::StatusChanged: {
                auto statusEvent = dynamic_cast<ElaphantContact::Listener::StatusEvent*>(&event);
                if (mChatGroupService->mOwnerHumanCode != statusEvent->humanCode) {
                    if (!mChatGroupService->inRemovedList(statusEvent->humanCode)) {
                        chatgroup::ConnectionStatus status = statusEvent->status == elastos::HumanInfo::Status::Online
                                                             ? chatgroup::ConnectionStatus_OnLine:chatgroup::ConnectionStatus_OffLine;
                        mChatGroupService->updateMemberInfo(statusEvent->humanCode, "", status);
                    }
                }
                Log::I(ChatGroupService_TAG, "StatusChanged from: %s, statusEvent->status:%d\n", statusEvent->humanCode.c_str(), static_cast<int>(statusEvent->status));
                break;
            }
            case ElaphantContact::Listener::EventType::HumanInfoChanged:{
                auto infoEvent = dynamic_cast<ElaphantContact::Listener::InfoEvent*>(&event);
                Log::I(ChatGroupService_TAG, "HumanInfoChanged from: %s\n", infoEvent->humanCode.c_str());
                std::string nickname;
                infoEvent->humanInfo->getHumanInfo(elastos::HumanInfo::Item::Nickname, nickname);
                mChatGroupService->updateMemberInfo(infoEvent->humanCode, nickname,chatgroup::ConnectionStatus_OnLine);
                break;
            }
            default: {
                break;
            }
        }
    };

    void ChatGroupMessageListener::onReceivedMessage(const std::string& humanCode, ContactChannel channelType,
                                                     std::shared_ptr<ElaphantContact::Message> msgInfo) {

        auto text_data = dynamic_cast<ElaphantContact::Message::TextData*>(msgInfo->data.get());
        std::string content = text_data->toString();
        try {
            Json json = Json::parse(content);
            std::string msg_content = json["content"];
            printf("ChatGroupMessageListener onReceivedMessage humanCode: %s,msg_content:%s \n", humanCode.c_str(), msg_content.c_str());
            mChatGroupService->addMessage(humanCode,
                                          msg_content, mChatGroupService->getTimeStamp());
        } catch (const std::exception& e) {
            printf("ChatGroupMessageListener parse json failed\n");
        }
    }

    extern "C" {
    ChatGroupService* CreateService(const char* path) {
        return new ChatGroupService(path);
    }

    void DestroyService(ChatGroupService* service) {
        if (service) {
            delete service;
        }
    }
    }
}