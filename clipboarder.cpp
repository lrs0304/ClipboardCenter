#include "clipboarder.h"
#include "ui_clipboarder.h"

Clipboarder::Clipboarder(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Clipboarder)
{
    ui->setupUi(this);
    connect(board, SIGNAL(changed(QClipboard::Mode)), this, SLOT(clipboardChanged(QClipboard::Mode)));
    connect(ui->pushButton_quit, SIGNAL(clicked()), qApp, SLOT(quit()));
    setWindowFlags(Qt::WindowTitleHint|Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
    shadow_effect->setOffset(-5, 5);
    shadow_effect->setColor(Qt::gray);
    shadow_effect->setBlurRadius(8);
    ui->frame_main->setGraphicsEffect(shadow_effect);
    ui->stackedWidget->setCurrentIndex(0);
    setWindowOpacity(0.8);
    timer->start(500);
    connect(timer, SIGNAL(timeout()), this, SLOT(fRefresh()) );
    timer_opTop->start(10*1000);
    connect(timer_opTop, SIGNAL(timeout()), this, SLOT(fRefreshWindowOnTop()) );
}

Clipboarder::~Clipboarder()
{
    delete ui;
}

void Clipboarder::mousePressEvent(QMouseEvent *event)
{
    mMoveing = true;
    mMovePosition = event->globalPos() - pos();
    event->accept();
}

void Clipboarder::mouseMoveEvent(QMouseEvent *event)
{
    if (mMoveing && (event->buttons() && Qt::LeftButton)
            && (event->globalPos()-mMovePosition).manhattanLength() > QApplication::startDragDistance())
    {
        move(event->globalPos()-mMovePosition);
        mMovePosition = event->globalPos() - pos();
    }
    event->accept();
}

void Clipboarder::mouseReleaseEvent(QMouseEvent *event)
{
    mMoveing = false;
}

void Clipboarder::clipboardChanged(QClipboard::Mode mode_)
{
    QString text = getClipboardText();
    if(list_clipboarde.isEmpty())
    {
        list_clipboarde.insert(0,text);
        loadClipboardList(mode_);
        return;
    }
    if(mode_ != mode or text.isEmpty() or list_clipboarde[0]==text)
        return;

    list_clipboarde.insert(0,text);
    loadClipboardList(mode_);
}

void Clipboarder::fRefresh()
{
    clipboardChanged(mode);
}

void Clipboarder::fRefreshWindowOnTop()
{
    setWindowFlags(Qt::WindowTitleHint|Qt::WindowSystemMenuHint|Qt::WindowCloseButtonHint|Qt::WindowStaysOnTopHint|Qt::FramelessWindowHint);
    show();
}

void Clipboarder::setClipboardText(const QString &text)
{
    board->setText(text, mode);
    return;
}

QString Clipboarder::getClipboardText()
{
    return board->text(mode);
}

void Clipboarder::loadClipboardList(QClipboard::Mode mode_)
{
    ui->listWidget->clear();
    int key=0;
    foreach (QString text, list_clipboarde) {
        addItem(text, QDateTime::currentDateTime().toString("yyyy-MM-dd dddd hh:mm"), isStarText(text), key);
        key++;
    }
}

void Clipboarder::loadStarList()
{
    ui->listWidget_starList->clear();

    QSettings settings("AMS","ClipboardCenter");
    settings.beginGroup("starList");
    QStringList childKeys = settings.childKeys();
    int key=0;
    foreach (QString child, childKeys) {
        QStringList list =settings.value(child).toStringList();
        if(list.isEmpty())
            continue;
        QString text = list[0];
        QString time = list[1];
        addItem(text, time, key);
        key++;
    }
    settings.endGroup();
}

bool Clipboarder::isStarText(QString text)
{
    QString ID;
    ID.prepend(QCryptographicHash::hash(text.toUtf8(),QCryptographicHash::Md4));    QSettings settings("AMS","ClipboardCenter");
    settings.beginGroup("starList");
    if(settings.contains(ID))
        return true;
    else
        return false;
    settings.endGroup();
}

void Clipboarder::itemIsStarChanged_receiver(QString text, bool isStar_)
{
    QString ID;
    ID.prepend(QCryptographicHash::hash(text.toUtf8(),QCryptographicHash::Md4));
    QSettings settings("AMS","ClipboardCenter");
    settings.beginGroup("starList");
    if(!isStar_)
    {
        settings.remove(ID);
        return;
    }
    if(settings.contains(ID))
        return;
    QStringList list;
    list<<text<<QDateTime::currentDateTime().toString("yyyy-MM-dd dddd hh:mm");
    settings.setValue(ID, list);
    settings.endGroup();

    loadStarList();
}

void Clipboarder::itemClicked_receiver(QString text)
{
    setClipboardText(text);
    ui->stackedWidget->setCurrentIndex(0);
}

void Clipboarder::addItem(QString text ,QString time, bool isStar, int key)
{
    item *widget = new item(this);
    widget->setText(text);
    widget->setTime(time);
    widget->setStar(isStar);
    widget->setKey(key);
    connect(widget, SIGNAL(itemIsStarChanged(QString, bool)), this, SLOT(itemIsStarChanged_receiver(QString, bool)));
    connect(widget, SIGNAL(itemClicked(QString)), this, SLOT(itemClicked_receiver(QString)));
    /*const int textLong = 70;
    QHBoxLayout * hBoxLayout = new QHBoxLayout();
    QVBoxLayout * vBoxLayout = new QVBoxLayout();
    QLabel *label_text = new QLabel();
    QLabel *label_star = new QLabel();
    QLabel *label_time = new QLabel();
    QString text_ = text;*
    if(text.length()>textLong)
        text_ = text.left(textLong);*
    label_text->setText(text_);
    label_text->setToolTip("<b>Text:</b><br />    "+text);
    label_time->setText("<i><font color = #C0C0C0>"+time+"</font></i>");
    //QDateTime::currentDateTime().toString("yyyy-MM-dd dddd hh:mm")
    label_time->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    if(isStar)
        label_star->setPixmap(QPixmap(":/st"));
    hBoxLayout->addWidget(label_text);
    hBoxLayout->addWidget(label_star);
    vBoxLayout->addLayout(hBoxLayout);
    vBoxLayout->addWidget(label_time);
    widget->setLayout(vBoxLayout);*/
    QListWidgetItem *item_ui = new QListWidgetItem("",ui->listWidget);
    ui->listWidget->addItem(item_ui);
    ui->listWidget->setItemWidget(item_ui, widget);
    item_ui->setSizeHint(QSize(widget->width(),widget->height()));
}

void Clipboarder::addItem(QString text, QString time, int key)
{
    item *widget = new item(this);
    widget->setText(text);
    widget->setTime(time);
    widget->setStar(true);
    widget->setKey(key);
    connect(widget, SIGNAL(itemIsStarChanged(QString, bool)), this, SLOT(itemIsStarChanged_receiver(QString, bool)));
    connect(widget, SIGNAL(itemClicked(QString)), this, SLOT(itemClicked_receiver(QString)));
    QListWidgetItem *item_ui = new QListWidgetItem("",ui->listWidget_starList);
    ui->listWidget_starList->addItem(item_ui);
    ui->listWidget_starList->setItemWidget(item_ui, widget);
    item_ui->setSizeHint(QSize(widget->width(),widget->height()));
}

void Clipboarder::on_pushButton_toHistory_clicked()
{
    ui->pushButton_toStar->setIcon(QIcon(":/st2"));
    ui->pushButton_toHistory->setIcon(QIcon(":/hi"));
    ui->stackedWidget->setCurrentIndex(0);
    loadClipboardList(mode);
}

void Clipboarder::on_pushButton_toStar_clicked()
{
    ui->pushButton_toStar->setIcon(QIcon(":/st1"));
    ui->pushButton_toHistory->setIcon(QIcon(":/hi2"));
    ui->stackedWidget->setCurrentIndex(1);
    loadStarList();
}

void Clipboarder::on_pushButton_clicked()
{
    QSettings settings("AMS","ClipboardCenter");
    settings.clear();
    loadStarList();
}

void Clipboarder::on_pushButton_info_clicked()
{
    QMessageBox::about(this, "About ", "<html><head/><body><p align=\"center\"><img src=\":/logo\"/></p><p align=\"center\"><span style=\" font-size:11pt; color:#a2b2c2;\">AMS</span><span style=\" font-size:11pt; font-weight:600; color:#507596;\"> - ClipboardCenter</span></p><p align=\"center\"><span style=\" font-size:11pt; font-weight:600; color:#507596;\">V1.0</span></p><p align=\"center\"><br/>By <span style=\" font-weight:600; color:#ffd800;\">San Diego</span></p><p align=\"center\"><span style=\" color:#55aaff;\">2421653893@qq.com</span></p><p align=\"center\"><a href=\"https://github.com/MrAMS/ClipboardCenter\"><span style=\" text-decoration: underline; color:#0000ff;\">GitHub</span></a></p></body></html>");
}
