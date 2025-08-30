cmake_minimum_required(VERSION 3.16)

project(Cyanla VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Charts Sql Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        
        # Core files
        src/core/DatabaseManager.cpp
        src/core/AIApiClient.cpp
        
        # Common view components  
        src/views/common/UIStyleManager.cpp
        src/views/common/LoginDialog.cpp
        src/views/common/RegisterDialog.cpp
        
        # visitor view components
        src/views/visitor/ChatWidget.cpp
        src/views/visitor/RealChatWidget.cpp
        src/views/visitor/FAQWidget.cpp
        src/views/visitor/AppointmentWidget.cpp
        src/views/visitor/VisitorMainWidget.cpp
        
        # Staff view components
        src/views/staff/StaffChatManager.cpp
        src/views/staff/StatsWidget.cpp
        src/views/staff/StaffMainWidget.cpp
        src/views/staff/ConsultationWidget.cpp
        src/views/staff/KnowledgeBaseWidget.cpp
        
        # Admin view components
        src/views/admin/UserManageWidget.cpp
        src/views/admin/SystemStatsWidget.cpp
        src/views/admin/SystemConfigWidget.cpp
        src/views/admin/AuditLogWidget.cpp
        src/views/admin/AdminMainWidget.cpp
)

qt_add_executable(Cyanla
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}

    src/views/staff/ConsultationWidget.h
    src/views/staff/ManualChatWidget.cpp
    src/views/staff/StaffWindow.cpp
    src/views/staff/RecordWidget.cpp
    src/views/staff/KnowledgeBaseWidget.h
    src/views/staff/RecordWidget.h
    src/views/staff/StaffMainWidget.h
    src/views/staff/StatsWidget.h
    src/views/staff/ManualChatWidget.h
    src/views/staff/StaffChatManager.h
    src/views/staff/StaffWindow.h
    src/views/common/BaseWindow.cpp
    src/views/common/BaseWindow.h
    src/views/common/SideMenuWidget.h
    src/views/common/TitleBarWidget.h
    src/views/common/SideMenuWidget.cpp
    src/views/common/TitleBarWidget.cpp
    src/core/UserRole.h
    src/core/AIApiClient.h
    src/core/DatabaseManager.h
    src/core/ChatStorage.h
    src/core/ChatStorage.cpp
    src/views/visitor/RealChatWidget.cpp
    src/views/visitor/AppointmentWidget.cpp
    src/views/visitor/VisitorWindow.cpp
    src/views/visitor/VisitorMainWidget.h
    src/views/visitor/RealChatWidget.h
    src/views/visitor/VisitorWindow.h
    src/views/visitor/FAQWidget.h
    src/views/visitor/AppointmentWidget.h
    src/views/visitor/ChatWidget.h
    mainwindow.h
    src/views/common/BaseWindow.cpp src/views/common/BaseWindow.h src/views/common/ExampleUsageWidget.cpp src/views/common/ExampleUsageWidget.h src/views/common/CompanyNavigationWidget.cpp src/views/common/CompanyNavigationWidget.h src/views/common/LoginDialog.cpp src/views/common/LoginDialog.h src/views/common/RegisterDialog.cpp src/views/common/RegisterDialog.h src/views/common/RoleSelector.cpp src/views/common/RoleSelector.h src/views/common/SideMenuWidget.cpp src/views/common/SideMenuWidget.h src/views/common/TitleBarWidget.cpp src/views/common/TitleBarWidget.h src/views/common/UIStyleManager.cpp src/views/common/UIStyleManager.h
    src/views/staff/ConsultationWidget.cpp src/views/staff/ConsultationWidget.h src/views/staff/KnowledgeBaseWidget.cpp src/views/staff/KnowledgeBaseWidget.h src/views/staff/ManualChatWidget.cpp src/views/staff/ManualChatWidget.h src/views/staff/RecordWidget.cpp src/views/staff/RecordWidget.h src/views/staff/StaffChatManager.cpp src/views/staff/StaffChatManager.h src/views/staff/StaffMainWidget.cpp src/views/staff/StaffMainWidget.h src/views/staff/StaffWindow.cpp src/views/staff/StaffWindow.h src/views/staff/StatsWidget.cpp src/views/staff/StatsWidget.h
    src/views/admin/AdminMainWidget.cpp src/views/admin/AdminMainWidget.h src/views/admin/AdminWindow.cpp src/views/admin/AdminWindow.h src/views/admin/AuditLogWidget.cpp src/views/admin/AuditLogWidget.h src/views/admin/SystemConfigWidget.cpp src/views/admin/SystemConfigWidget.h src/views/admin/SystemStatsWidget.cpp src/views/admin/SystemStatsWidget.h src/views/admin/UserManageWidget.cpp src/views/admin/UserManageWidget.h
    src/views/visitor/AppointmentWidget.cpp src/views/visitor/AppointmentWidget.h src/views/visitor/ChatWidget.cpp src/views/visitor/ChatWidget.h src/views/visitor/FAQWidget.cpp src/views/visitor/FAQWidget.h src/views/visitor/MapWidget.cpp src/views/visitor/MapWidget.h src/views/visitor/RealChatWidget.cpp src/views/visitor/RealChatWidget.h src/views/visitor/VisitorMainWidget.cpp src/views/visitor/VisitorMainWidget.h src/views/visitor/VisitorWindow.cpp src/views/visitor/VisitorWindow.h
    mainwindow.ui

)
set_source_files_properties(
    src/core/ChatStorage.cpp
    PROPERTIES
    SKIP_AUTOMOC TRUE
)
target_link_libraries(Cyanla PRIVATE Qt6::Core Qt6::Widgets Qt6::Charts Qt6::Sql Qt6::Network)

set_target_properties(Cyanla PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
)

# 包含目录
target_include_directories(Cyanla PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# 编译器特定设置
if(MSVC)
    target_compile_options(Cyanla PRIVATE /W4)
else()
    target_compile_options(Cyanla PRIVATE -Wall -Wextra -Wpedantic)
endif()

qt_finalize_executable(Cyanla)
