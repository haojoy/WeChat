#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QIcon>
#include <QPixmap>
#include <QDebug>
#include <QMenu>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QResource>

MainWindow::MainWindow(QWidget *parent, int id)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_page(AllChat)
    , m_id(id)
    , m_avatarId("c6224575-0121-11ee-b77d-00163e2e631d")
{
    ui->setupUi(this);
    m_contactlist = new ContactListWidget;
    connect(m_contactlist, SIGNAL(SIG_GetFriendInfo(QString)), this, SLOT(onGetFriendInfoReceived(QString)));
    m_contactWidget = new ContactWidget;
    connect(m_contactWidget, SIGNAL(SIG_FriendRequestAccept(int)), this, SLOT(onAcceptFriendReceived(int)));
    m_allchatlist = new ChatListWidget;
    m_allchatWidget = new ChatWidget(nullptr, m_id);
    connect(m_allchatWidget, SIGNAL(SIG_SendChatMsg(int, QString)), this, SLOT(onChatMessageReceived(int, QString)));
    connect(m_allchatWidget, SIGNAL(SIG_SendFile(int, QString, uint64_t)), this, SLOT(onChatFileReceived(int, QString, uint64_t)));

    /* 设置界面 */
    m_menu = new QMenu(this);
    m_menu->addAction("更换头像");
    m_menu->addAction("设置");
    connect(m_menu, SIGNAL(triggered(QAction*)), this, SLOT(slot_dealMenu(QAction*)));

    m_userInfoDialog = new UserInfoShowDialog;
    connect(m_userInfoDialog, SIGNAL(SIG_AddFriendRequest(QString, int)), this, SLOT(onAddFriendRequest(QString, int)));
    init();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setIcon(QString &resPath)
{
    QIcon icon(resPath);
    QPixmap pixmap = icon.pixmap(QSize(40, 40));
    if (pixmap.isNull()) {
        qDebug() << "pixmap is NUll";
        return;
    }
    ui->headBtn->setIcon(QIcon(pixmap));
    ui->headBtn->setIconSize(QSize(40, 40));
}

void MainWindow::setAvatar(QByteArray &byteArray)
{
    QPixmap pixmap;
    pixmap.loadFromData(byteArray);
    if (pixmap.isNull()) {
        qDebug() << __func__ << "头像数据错误";
        return;
    }
    ui->headBtn->setIcon(QIcon(pixmap));
    ui->headBtn->setIconSize(QSize(40, 40));
}

void MainWindow::init()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

    this->setMouseTracking(true);
    this->installEventFilter(this); // 将事件过滤器对象(this)安装到目标对象(this)上

    /* 绑定信号与槽函数 */
    connect(ui->stickBtn, &QPushButton::clicked, this, &MainWindow::stickEvent);
    connect(ui->mininBtn, &QPushButton::clicked, this, &MainWindow::mininEvent);
    connect(ui->maxinBtn, &QPushButton::clicked, this, &MainWindow::maxinEvent);
    connect(ui->closeBtn, &QPushButton::clicked, this, &MainWindow::clozeEvent);

    connect(ui->allchatBtn, &QPushButton::clicked, this, &MainWindow::selectEvent);
    connect(ui->contactBtn, &QPushButton::clicked, this, &MainWindow::selectEvent);
    connect(ui->collectBtn, &QPushButton::clicked, this, &MainWindow::selectEvent);

    /* 设置初始化状态 */
    m_btn = ui->allchatBtn;
    ui->gridLayout_2->addWidget(m_allchatlist);
    m_allchatlist->show();

    connect(m_allchatlist, &ChatListWidget::onItemClicked, this, &MainWindow::onView);
}

Area MainWindow::getArea(int x, int y)
{
    auto rect = geometry();
    if (x <= 10)
    {
        if (y >= rect.height() - 10)
            return Area::BottomLeft;
        else if (y <= 10)
            return  Area::TopLeft;
        else
            return  Area::Left;
    }
    else if (x >= rect.width() - 10)
    {
        if (y >= rect.height() - 10)
            return Area::BottomRight;
        else if (y <= 10)
            return Area::TopRight;
        else
            return Area::Right;
    }
    else
    {
        if (y >= rect.height() - 10)
            return  Area::Bottom;
        else if (y <= 10)
            return Area::Top;
    }
    return Area::Center;
}

void MainWindow::stickEvent()
{
    ui->stickBtn->setIcon(QIcon(QPixmap(ui->stickBtn->isChecked() ? ":/images/sticky2.svg" : ":/images/sticky.svg")));
}

void MainWindow::mininEvent()
{
    setWindowState(Qt::WindowState::WindowMinimized);
}

void MainWindow::clozeEvent()
{
    close();
}

void MainWindow::maxinEvent()
{
    switch (windowState())
    {
    case Qt::WindowState::WindowMaximized:
        setWindowState(Qt::WindowState::WindowNoState);
        ui->maxinBtn->setIcon(QIcon(QPixmap(":/images/max.svg")));
        break;
    case Qt::WindowState::WindowNoState:
        setWindowState(Qt::WindowState::WindowMaximized);
        ui->maxinBtn->setIcon(QIcon(QPixmap(":/images/max3.svg")));
        break;
    }
}

void MainWindow::selectEvent()
{
    /*取消之前选中的按钮的选中状态，然后将当前的按钮设置为选中状态*/
    m_btn->setChecked(false);
    m_btn = (QPushButton*)sender();
    m_btn->setChecked(true);

    /*
     * 根据之前的页面类型 (m_page) 执行不同的操作(移除并隐藏)
     * gridLayout_2: 聊天列表窗口布局
     */
    switch (m_page) {
    case AllChat:
        ui->gridLayout_2->removeWidget(m_allchatlist);
        ui->gridLayout_3->removeWidget(m_allchatWidget);
        m_allchatlist->hide();
        m_allchatWidget->hide();
        break;
    case Contact:
        ui->gridLayout_2->removeWidget(m_contactlist);
        ui->gridLayout_3->removeWidget(m_contactWidget);
        m_contactlist->hide();
        m_contactWidget->hide();
        break;
    case Collect:
        break;
    }

    auto name = m_btn->objectName();
    if (name == "allchatBtn") {
        m_page = ChatPage::AllChat;
        ui->gridLayout_2->addWidget(m_allchatlist);
        // ui->gridLayout_4->addWidget(m_allchatWidget);
        m_allchatlist->show();
        // m_allchatWidget->show();
    } else if (name == "contactBtn") {
        m_page = ChatPage::Contact;
        ui->gridLayout_2->addWidget(m_contactlist);
        ui->gridLayout_4->addWidget(m_contactWidget);
        m_contactlist->show();
        m_contactWidget->show();
    } else if (name == "collectBtn") {
        m_page = ChatPage::Collect;
    }
}

void MainWindow::onView(QVariant var, PageType type)
{
    if (var.canConvert<Friend*>() && type == PageType::AllChatView) {
        auto data = var.value<Friend*>();
        ui->gridLayout_4->addWidget(m_allchatWidget); // gridLayout_4: 聊天框窗口布局
        m_allchatWidget->setData(data);
        m_allchatWidget->show();
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) // 左键按下
    {
        isMouseDown = true;
        m_pressRect = geometry();                   // m_pressRect设置为当前窗口的几何属性
        m_pressPos = event->globalPos();            // m_pressPos设置为鼠标按下时的全局位置
        m_posx = mapFromGlobal(m_pressPos).x();     // m_posx: 相对于窗口的局部x轴坐标
        m_posy = mapFromGlobal(m_pressPos).y();     // m_posy: 相对于窗口的局部y轴坐标
        m_area = getArea(event->x(), event->y());   // m_area: 鼠标所在的区域
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event); // 防止编译器发出未使用变量的警告
    if (isMouseDown)
    {
        auto currentPos = QCursor::pos();
        auto moveSize = currentPos - m_pressPos;
        auto currentRect = m_pressRect;
        switch (m_area)
        {
        case Top:
            currentRect.setTop(m_pressRect.top() + moveSize.y());
            break;
        case Bottom:
            currentRect.setBottom(m_pressRect.bottom() + moveSize.y());
            break;
        case Left:
            currentRect.setLeft(m_pressRect.left() + moveSize.x());
            break;
        case Right:
            currentRect.setRight(m_pressRect.right() + moveSize.x());
            break;
        case TopLeft:
            currentRect.setTopLeft(m_pressRect.topLeft() + moveSize);
            break;
        case BottomRight:
            currentRect.setBottomRight(m_pressRect.bottomRight() + moveSize);
            break;
        case TopRight:
            currentRect.setTopRight(m_pressRect.topRight() + moveSize);
            break;
        case BottomLeft:
            currentRect.setBottomLeft(m_pressRect.bottomLeft() + moveSize);
            break;
        case Center:
            move(currentPos.x() - m_posx, currentPos.y() - m_posy);
            break;
        }
        if (m_area != Center && currentRect.width() >= 700 && currentRect.height() >= 500)
        {
            setGeometry(currentRect);
        }
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event); // 防止编译器发出未使用变量的警告
    isMouseDown = false;
    QApplication::restoreOverrideCursor(); // 鼠标释放后，光标恢复到应用程序的默认状态
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event); // 防止编译器发出未使用变量的警告
    switch (windowState())
    {
    case Qt::WindowMaximized:
        setWindowState(Qt::WindowNoState);      //最大化状态->无状态(取消最大化)
        break;
    case Qt::WindowNoState:
        setWindowState(Qt::WindowMaximized);    //无状态->最大化状态(窗口最大化)
        break;
    }
}

bool MainWindow::eventFilter(QObject *sender, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) { // 鼠标点击事件
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        if (m_userInfoDialog && !m_userInfoDialog->geometry().contains(mouseEvent->pos())) {
            qDebug() << "鼠标点击事件: 点击其他区域";
            m_userInfoDialog->hide();
        }
    }
    // 鼠标悬停事件
    // QEvent::HoverEnter、QEvent::HoverMove、QEvent::HoverLeave
    if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
    {
        // QCursor::pos(): 返回当前鼠标的全局坐标，即相对于整个屏幕的坐标。
        auto pos = mapFromGlobal(QCursor::pos()); // 将全局鼠标坐标转换为当前窗口的局部坐标。
        auto area = getArea(pos.rx(), pos.ry());  // 获取鼠标所在的区域
        switch (area)
        {
        case Top:
        case Bottom:
            setCursor(Qt::SizeVerCursor);   // area的值为Top或Bottom, 将鼠标光标设置为垂直调整大小的光标
            break;
        case Left:
        case Right:
            setCursor(Qt::SizeHorCursor);   // area的值为Left或Right, 将鼠标光标设置为水平调整大小的光标
            break;
        case TopLeft:
        case BottomRight:
            setCursor(Qt::SizeFDiagCursor); // area的值为TopLeft或BottomRight, 将鼠标光标设置为斜向调整大小的光标
            break;
        case TopRight:
        case BottomLeft:
            setCursor(Qt::SizeBDiagCursor); // area的值为TopRight或BottomLeft, 将鼠标光标设置为反斜向调整大小的光标
            break;
        case Center:
        default:
            setCursor(Qt::ArrowCursor);     // area的值为Center或其他未指定的值, 将鼠标光标设置为默认的箭头光标
            break;
        }
    }
    return QMainWindow::eventFilter(sender, event); // 继续将事件传递给父类处理
}


void MainWindow::onChatMessageReceived(int id, QString message) {
    emit sendChatMessage(id, message);
}

void MainWindow::onChatFileReceived(int friendid, QString filename, uint64_t filesize) {
    emit sendChatFile(friendid, filename, filesize);
}

void MainWindow::onGetFriendInfoReceived(QString friendname) {
    emit sendGetFriendInfo(friendname);
}

void MainWindow::onAddFriendRequest(QString friendname, int friendId)
{
    emit sendAddFriendRequest(friendname, friendId);
}

void MainWindow::onAcceptFriendReceived(int id)
{
    emit sendFriendReqAccepted(id);
}

void MainWindow::on_moreBtn_clicked() // “更多”按钮点击事件
{
    qDebug() << __func__;
    QPoint p = QCursor::pos();
    QSize size = m_menu->sizeHint();
    m_menu->exec(QPoint(p.x(), p.y()-size.height()));
}

void MainWindow::slot_dealMenu(QAction *action)
{
    qDebug() << __func__;
    if( action->text() == "更换头像" ) {
        emit SIG_ChangeUserIcon();
    } else if(action->text() == "系统设置" ) {

    }
}

