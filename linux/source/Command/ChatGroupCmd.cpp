#include <Command/ChatGroupCmd.hpp>

#include <iostream>
#include <iterator>
#include <sstream>
namespace elastos {
/* =========================================== */
/* === variables initialize =========== */
/* =========================================== */
    const std::vector <ChatGroupCmd::CommandInfo> ChatGroupCmd::gCommandInfoList{
            {"h", "help",   ChatGroupCmd::Help,           "Print help usages."},
            //{'a', "add",   ChatGroupCmd::AddFriend,       "Add a new friend"},
            {"b", "block",  ChatGroupCmd::BlockFriend,    "Block a friend"},
            {"d", "del",    ChatGroupCmd::DelFriend,      "Delete a friend"},
            //{'i', "info",  ChatGroupCmd::PrintInfo, "Print friend's detail infommation"},
            {"l", "list",   ChatGroupCmd::ListFriends,    "List friends."},
            {"u", "update", ChatGroupCmd::UpdateNickName, "Update group name"},
            {"e", "exit",   ChatGroupCmd::DeleteGroup,    "Delete group and exit"},

    };

    inline std::string &ltrim(std::string &str) {
        std::string::iterator p = std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun<int, int>(isspace)));
        str.erase(str.begin(), p);
        return str;
    }

/* =========================================== */
/* === function implement ============= */
/* =========================================== */
    int ChatGroupCmd::Do(void *context,
                         const std::string &cmd_msg,
                         std::string &errMsg) {
        std::string trim_msg = cmd_msg;
        trim_msg = ltrim(trim_msg);
        if (trim_msg.find('/') != 0) {
            errMsg = "not command";
            return -10000;
        }

        const std::string &cmdLine = trim_msg.substr(1);
        auto wsfront = std::find_if_not(cmdLine.begin(), cmdLine.end(),
                                        [](int c) { return std::isspace(c); });
        auto wsback = std::find_if_not(cmdLine.rbegin(), cmdLine.rend(),
                                       [](int c) { return std::isspace(c); }).base();
        auto trimCmdLine = (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));

        std::istringstream iss(trimCmdLine);
        std::vector <std::string> args{std::istream_iterator < std::string > {iss},
                                       std::istream_iterator < std::string > {}};
        if (args.size() <= 0) {
            return 0;
        }
        const auto &cmd = args[0];

        for (const auto &cmdInfo : gCommandInfoList) {
            if (cmd.compare(0, 1, cmdInfo.mCmd) != 0
                && cmd != cmdInfo.mLongCmd) {
                continue;
            }

            int ret = cmdInfo.mFunc(context, args, errMsg);
            return ret;
        }

        errMsg = "Unknown command: " + cmd;
        return -10000;
    }

/* =========================================== */
/* === class public function implement  ====== */
/* =========================================== */

/* =========================================== */
/* === class protected function implement  === */
/* =========================================== */


/* =========================================== */
/* === class private function implement  ===== */
/* =========================================== */

    int ChatGroupCmd::Help(void *context,
                           const std::vector <std::string> &args,
                           std::string &errMsg) {
        std::cout << "Usage:" << std::endl;
        std::string msg = "";

        for (const auto &cmdInfo : gCommandInfoList) {
            msg += "" + cmdInfo.mCmd + " | " + cmdInfo.mLongCmd + " : " + cmdInfo.mUsage + "\n";
        }
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        carrier_robot->helpCmd(args, msg);
        return 0;
    }

    int ChatGroupCmd::AddFriend(void *context,
                                const std::vector <std::string> &args,
                                std::string &errMsg) {

        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        /*auto weakFriendMgr = contact->getFriendManager();
        auto friendMgr = weakFriendMgr.lock();
        if (friendMgr.get() == nullptr) {
            errMsg = "FriendManager has been released.";
            return -1;
        }

        auto friendId = args.size() > 1 ? args[1] : "";
        auto summary = args.size() > 2 ? args[2] : "";

        int ret = friendMgr->tryAddFriend(friendId, summary);
        if (ret < 0) {
            errMsg = "Failed to add friend ret=" + std::to_string(ret);
            return ret;
        }*/

        return 0;
    }


    int ChatGroupCmd::BlockFriend(void *context,
                                  const std::vector <std::string> &args,
                                  std::string &errMsg) {
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        carrier_robot->blockFriendCmd(args);
        return 0;
    }

    int ChatGroupCmd::DelFriend(void *context,
                                const std::vector <std::string> &args,
                                std::string &errMsg) {
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        carrier_robot->delCmd(args);
        return 0;
    }

    int ChatGroupCmd::ListFriends(void *context,
                                  const std::vector <std::string> &args,
                                  std::string &errMsg) {
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        carrier_robot->listCmd(args);
        return 0;
    }

    int ChatGroupCmd::UpdateNickName(void *context, const std::vector <std::string> &args,
                                     std::string &errMsg) {
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        carrier_robot->updateNickNameCmd(args);
        return 0;
    }

    int ChatGroupCmd::DeleteGroup(void *context, const std::vector <std::string> &args,
                                  std::string &errMsg) {
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);
        carrier_robot->deleteGroupCmd(args);
        return 0;
    }

    int ChatGroupCmd::PrintInfo(void *context,
                                const std::vector <std::string> &args,
                                std::string &errMsg) {
        auto carrier_robot = reinterpret_cast< elastos::ChatGroupService *>(context);

        return 0;
    }
}