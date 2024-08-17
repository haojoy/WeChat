#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QVariant>
#include "Views/chatlistwidget.h"
#include "Views/chatwidget.h"
#include "Views/contactlistwidget.h"
#include "Views/contactwidget.h"
#include "Views/userinfoshowdialog.h"

/// 鼠标所在区域
enum Area {
    Top,        // 表示鼠标在顶部区域
    Left,       // 表示鼠标在左侧区域
    Right,      // 表示鼠标在右侧区域
    Bottom,     // 表示鼠标在底部区域
    Center,     // 表示鼠标在中心区域或其他未指定的区域
    TopLeft,    // 表示鼠标在左上角区域
    TopRight,   // 表示鼠标在右上角区域
    BottomLeft, // 表示鼠标在左下角区域
    BottomRight // 表示鼠标在右下角区域
};

/// midBar展现的界面
enum ChatPage {
    AllChat, // 所有的聊天列表
    Contact, // 联系人列表
    Collect, // 收藏列表
};

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
signals:
    void sendChatMessage(int id, const QString &message);  // 自定义信号声明
    void sendChatFile(int friendid, const QString& filename, uint64_t filesize); // 自定义信号声明
    void sendGetFriendInfo(QString friendname); // 自定义信号声明
    void sendAddFriendRequest(QString friendname, int friendId); // 自定义信号声明
    void sendFriendReqAccepted(int id);
    void SIG_ChangeUserIcon();
public:
    MainWindow(QWidget *parent = nullptr);
    MainWindow(QWidget *parent = nullptr, int id = 0);
    ~MainWindow();
    ChatListWidget* getChatListWidget() const { return m_allchatlist; }
    ContactListWidget* getContactList() const { return m_contactlist; }
    ContactWidget* getContactWidget() const { return m_contactWidget; }
    UserInfoShowDialog* getUserInfoDialog() const { return m_userInfoDialog; }
    const QString& getAvatarId() const { return m_avatarId; }
    void setAvatarId(const QString& id) { m_avatarId = id;}
public:
    void setIcon(const QString& resPath);
    void setAvatar(QByteArray& byteArray);
public slots:
    void onChatMessageReceived(int id, QString message);
    void onChatFileReceived(int friendid, QString filename, uint64_t filesize);
    void onGetFriendInfoReceived(QString friendname);
    void onAddFriendRequest(QString friendName, int friendId);
    void onAcceptFriendReceived(int id);
private:
    Ui::MainWindow *ui;

    /// 鼠标状态: true鼠标按下, false鼠标释放
    bool isMouseDown;
    /// 状态变量: 鼠标按下的全局位置
    QPoint m_pressPos;
    /// 状态变量: 鼠标按下的窗口几何属性
    QRect m_pressRect;
    /// 状态变量: 鼠标按下的局部坐标
    int m_posx, m_posy;
    /// 状态变量: 鼠标按下的所在区域
    Area m_area;
    /// 状态变量: 当前checked按钮
    QPushButton *m_btn;

    /// 添加好友显示的用户信息Dialog
    UserInfoShowDialog* m_userInfoDialog;
    /// 当前midBar展现的界面
    ChatPage m_page;    
    /// 最近的聊天列表
    ChatListWidget* m_allchatlist;
    /// 聊天item对应的聊天框
    ChatWidget* m_allchatWidget;
    // 联系人列表
    ContactListWidget* m_contactlist;
    ContactWidget* m_contactWidget;
    QMenu* m_menu;
    int m_id;
    QString m_avatarId;

    /**
     * @brief init 初始化MainWindow
     */
    void init();
    /**
     * @brief getArea 获取鼠标所在的区域
     * @param x 横坐标
     * @param y 纵坐标
     * @return 区域枚举值
     */
    Area getArea(int x, int y);

    /**
     * @brief stickEvent 置顶窗口事件
     */
    void stickEvent();
    /**
     * @brief mininEvent 窗口最小化事件
     */
    void mininEvent();
    /**
     * @brief clozeEvent 窗口关闭事件
     */
    void clozeEvent();
    /**
     * @brief maxinEvent 窗口最大化事件
     */
    void maxinEvent();
    /**
     * @brief selectEvent menu按钮选中事件
     */
    void selectEvent();

    /**
     * @brief onView 处理视图显示的操作
     * @param var
     * @param type
     */
    void onView(QVariant var, PageType type);

protected:
    /**
     * @brief mousePressEvent 重写鼠标点击事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;
    /**
     * @brief mouseMoveEvent 重写鼠标移动事件
     * @param event
     */
    void mouseMoveEvent(QMouseEvent *event) override;
    /**
     * @brief mouseReleaseEvent 重写鼠标释放事件
     * @param event
     */
    void mouseReleaseEvent(QMouseEvent *event) override;
    /**
     * @brief mouseDoubleClickEvent 重写鼠标双击事件
     * @param event
     */
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    /**
     * @brief eventFilter 重写事件过滤器函数(根据光标悬停位置，修改鼠标光标样式)
     * @param sender 目标对象
     * @param event
     * @return
     */
    bool eventFilter(QObject *sender, QEvent *event) override;
private slots:
    void on_moreBtn_clicked();
    /**
     * @brief slot_dealMenu 菜单按钮点击事件
     * @param action
     */
    void slot_dealMenu(QAction* action);
};
#endif // MAINWINDOW_H
