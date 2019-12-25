//
// Created by luocf on 2019/6/13.
//

#ifndef CHATGROUP_SERVICE_H
#define CHATGROUP_SERVICE_H

#include <stdlib.h>
#include <functional>
#include <memory> // std::unique_ptr
#include <ctime>
#include <thread>
#include <regex>
#include "Connector.h"
#include "PeerListener.h"
#include "PeerNode.h"
#include "Contact.hpp"
#include "DataBase/DatabaseProxy.h"
#include "Common/CommonVar.h"
namespace elastos {
    static const char *ChatGroupService_TAG = "ChatGroupService";
    class ChatGroupService:public std::enable_shared_from_this<ChatGroupService>{
    public:
        ChatGroupService(const std::string& path);
        ~ChatGroupService();
        int acceptFriend(const std::string& friendid);
        bool inRemovedList(const std::string& friendid);
        void updateMemberInfo(const std::string& friendid, const std::string& nickname,
                              chatgroup::ConnectionStatus status);
        void addMessage(const std::string& friend_id, const std::string& message, std::time_t send_time);
        std::shared_ptr<std::vector<std::shared_ptr<chatgroup::MemberInfo>>>getFriendList();
        void helpCmd(const std::vector<std::string> &args, const std::string& message);
        void listCmd(const std::vector<std::string> &args);
        void delCmd(const std::vector<std::string> &args);
        void blockFriendCmd(const std::vector<std::string> &args);
        void updateNickNameCmd(const std::vector<std::string> &args);
        void deleteGroupCmd(const std::vector<std::string> &args);
        std::string getGroupNickName();
        int getGroupStatus();
        std::time_t getTimeStamp();

        std::string mOwnerHumanCode;
    protected:
        std::string mPath;
    private:
        std::string convertDatetimeToString(std::time_t time);
        bool relayMessages();
        int mStatus;
        std::shared_ptr<std::regex> mMsgReg;
        std::mutex _mReplyMessage;
        std::string mLocalPath;
        std::string mCreaterFriendId;

        chatgroup::DatabaseProxy* mDatabaseProxy;
        Connector* mConnector;
        int Start();
        int Stop();

    };

    class ChatGroupMessageListener :public PeerListener::MessageListener{
    public:
        ChatGroupMessageListener( ChatGroupService* chatGroupService);
        ~ChatGroupMessageListener();
        void onEvent(ElaphantContact::Listener::EventArgs& event) override ;
        void onReceivedMessage(const std::string& humanCode, ElaphantContact::Channel channelType,
                               std::shared_ptr<ElaphantContact::Message> msgInfo) override;
    private:
        ChatGroupService*mChatGroupService;
    };

    extern "C" {
        ChatGroupService* CreateService(const char* path);
        void DestroyService(ChatGroupService* service);
    }
}

#endif //CHATGROUP_SERVICE_H
