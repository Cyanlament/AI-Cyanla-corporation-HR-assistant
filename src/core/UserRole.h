#ifndef USERROLE_H
#define USERROLE_H

enum class UserRole {
    Patient = 0,    // 患者
    Staff = 1,      // 客服
    Admin = 2       // 管理员
};

enum class MenuAction {
    // 患者菜单
    PatientChat = 100,
    PatientAppointment = 101,
    PatientMap = 102,
    
    // 客服菜单
    StaffChatManage = 200,
    StaffPatientList = 201,
    StaffKnowledge = 202,
    
    // 管理员菜单
    AdminUserManage = 300,
    AdminStats = 301,
    AdminSystem = 302
};

#endif // USERROLE_H 