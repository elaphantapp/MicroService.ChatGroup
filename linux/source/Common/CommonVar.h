//
// Created by luocf on 2019/7/17.
//

#ifndef CHATGROUP_COMMON_VAR_H
#define CHATGROUP_COMMON_VAR_H
namespace chatgroup{
    constexpr int CONST_SERVICE_LIMIT_NUM = 12;
    constexpr int Command_Ready = 0;
    constexpr int CreateGroup = 1;
    constexpr int Command_UpdateAddress = 2;
    constexpr int Command_UpdateMemberCount = 3;
    constexpr int Command_UpdateNickName = 4;
    constexpr int Command_UpdateStatus = 5;
    constexpr int Command_WatchDog = 6;
    constexpr int CONST_ERROR_CODE_MAX_NUM = -1;
    enum ConnectionStatus {
        ConnectionStatus_OnLine = 0,
        ConnectionStatus_OffLine = 1
    };

}

#endif //CHATGROUP_COMMON_VAR_H
