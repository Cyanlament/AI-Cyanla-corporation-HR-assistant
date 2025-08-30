#ifndef USERROLE_H
#define USERROLE_H
enum class UserRole {
    visitor = 0,    // 访客
    Staff = 1,      // UR客服
    Admin = 2       // 管理员
};

enum class MenuAction {
    // 访客菜单
    visitorChat = 100,
    visitorAppointment = 101,
    visitorMap = 102,

    // 客服菜单
    StaffChatManage = 200,
    StaffvisitorList = 201,
    StaffKnowledge = 202,

    // 管理员菜单
    AdminUserManage = 300,
    AdminStats = 301,
    AdminSystem = 302
};
#endif // USERROLE_H
